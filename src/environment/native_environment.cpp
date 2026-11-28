
#include "native_environment.h"

#include <windows.h>

#include <chrono>
#include <map>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace {

// Convert UTF-8 string to UTF-16.
std::wstring toWide(const std::string& value) {
    if (value.empty()) {
        return {};
    }

    const int size = MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        value.data(),
        static_cast<int>(value.size()),
        nullptr,
        0
    );

    if (size == 0) {
        throw std::runtime_error("Invalid UTF-8 input.");
    }

    std::wstring result(size, L'\0');

    MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        value.data(),
        static_cast<int>(value.size()),
        result.data(),
        size
    );

    return result;
}

// Convert UTF-16 string to UTF-8.
std::string toUtf8(const std::wstring& value) {
    if (value.empty()) {
        return {};
    }

    const int size = WideCharToMultiByte(
        CP_UTF8,
        0,
        value.data(),
        static_cast<int>(value.size()),
        nullptr,
        0,
        nullptr,
        nullptr
    );

    if (size == 0) {
        throw std::runtime_error("Failed to convert process output.");
    }

    std::string result(size, '\0');

    WideCharToMultiByte(
        CP_UTF8,
        0,
        value.data(),
        static_cast<int>(value.size()),
        result.data(),
        size,
        nullptr,
        nullptr
    );

    return result;
}

// Quote one Windows command-line argument.
std::wstring quoteArgument(const std::wstring& arg) {
    std::wstring result = L"\"";
    std::size_t backslashes = 0;

    for (wchar_t c : arg) {
        if (c == L'\\') {
            ++backslashes;
        }
        else if (c == L'"') {
            result.append(backslashes * 2 + 1, L'\\');
            result += L'"';
            backslashes = 0;
        }
        else {
            result.append(backslashes, L'\\');
            backslashes = 0;
            result += c;
        }
    }

    // Backslashes before the closing quote must be doubled.
    result.append(backslashes * 2, L'\\');
    result += L'"';

    return result;
}

// Return the absolute path of an allowed Windows executable.
std::wstring getExecutablePath(const std::string& command) {
    static const std::map<std::string, std::wstring> allowed = {
        {"whoami", L"whoami.exe"},
        {"hostname", L"hostname.exe"},
        {"ipconfig", L"ipconfig.exe"}
    };

    const auto it = allowed.find(command);

    if (it == allowed.end()) {
        throw std::runtime_error("Command is not allowed.");
    }

    wchar_t systemDirectory[MAX_PATH];

    const UINT length = GetSystemDirectoryW(
        systemDirectory,
        MAX_PATH
    );

    if (length == 0 || length >= MAX_PATH) {
        throw std::runtime_error("Cannot locate Windows System32.");
    }

    std::wstring path(systemDirectory);
    path += L"\\";
    path += it->second;

    return path;
}

// Validate arguments separately for each command.
void validateArguments(
    const std::string& command,
    const std::vector<std::string>& args
) {
    if (command == "whoami" || command == "hostname") {
        if (!args.empty()) {
            throw std::runtime_error(
                "This command does not accept arguments."
            );
        }

        return;
    }

    if (command == "ipconfig") {
        if (args.size() > 1 ||
            (!args.empty() && args[0] != "/all")) {
            throw std::runtime_error(
                "Allowed ipconfig argument: /all"
            );
        }

        return;
    }

    throw std::runtime_error("Command is not allowed.");
}

} // namespace

NativeEnvironment::NativeEnvironment(
    const std::filesystem::path& workspace
)
    : workspace(std::filesystem::absolute(workspace).lexically_normal()) {

    std::filesystem::create_directories(this->workspace);
}

std::string NativeEnvironment::executeCommand(
    const std::string& command,
    const std::vector<std::string>& args
) {
    HANDLE readPipe = nullptr;
    HANDLE writePipe = nullptr;

    PROCESS_INFORMATION processInfo{};

    try {
        // 1. Validate command and arguments.
        validateArguments(command, args);

        // 2. Get the absolute executable path.
        const std::wstring executable =
            getExecutablePath(command);

        // 3. Build the Windows command line.
        std::wstring commandLine =
            quoteArgument(executable);

        for (const auto& arg : args) {
            commandLine += L" ";
            commandLine += quoteArgument(toWide(arg));
        }

        // 4. Create a pipe to capture stdout and stderr.
        SECURITY_ATTRIBUTES security{};
        security.nLength = sizeof(SECURITY_ATTRIBUTES);
        security.bInheritHandle = TRUE;

        if (!CreatePipe(
                &readPipe,
                &writePipe,
                &security,
                0
            )) {
            throw std::runtime_error("Failed to create output pipe.");
        }

        // The parent must not pass the read end to the child.
        if (!SetHandleInformation(
                readPipe,
                HANDLE_FLAG_INHERIT,
                0
            )) {
            throw std::runtime_error(
                "Failed to configure output pipe."
            );
        }

        // 5. Configure child process startup.
        STARTUPINFOW startupInfo{};
        startupInfo.cb = sizeof(STARTUPINFOW);
        startupInfo.dwFlags = STARTF_USESTDHANDLES;
        startupInfo.hStdOutput = writePipe;
        startupInfo.hStdError = writePipe;
        startupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

        // CreateProcessW may modify its command-line buffer.
        std::vector<wchar_t> mutableCommandLine(
            commandLine.begin(),
            commandLine.end()
        );
        mutableCommandLine.push_back(L'\0');

        // 6. Create the process.
        if (!CreateProcessW(
                executable.c_str(),
                mutableCommandLine.data(),
                nullptr,
                nullptr,
                TRUE,
                CREATE_NO_WINDOW,
                nullptr,
                workspace.c_str(),
                &startupInfo,
                &processInfo
            )) {
            throw std::runtime_error(
                "CreateProcessW failed. Windows error: " +
                std::to_string(GetLastError())
            );
        }

        // Parent no longer needs the write end.
        CloseHandle(writePipe);
        writePipe = nullptr;

        // 7. Read output while monitoring the process.
        std::string output;
        constexpr std::size_t MAX_OUTPUT = 1024 * 1024;
        constexpr DWORD TIMEOUT_MS = 15000;

        const auto start = std::chrono::steady_clock::now();

        bool timedOut = false;
        bool processFinished = false;

        while (true) {
            DWORD available = 0;

            if (!PeekNamedPipe(
                    readPipe,
                    nullptr,
                    0,
                    nullptr,
                    &available,
                    nullptr
                )) {
                break;
            }

            if (available > 0) {
                char buffer[4096];
                DWORD bytesRead = 0;

                const DWORD toRead =
                    (available < sizeof(buffer))
                    ? available
                    : static_cast<DWORD>(sizeof(buffer));

                if (!ReadFile(
                        readPipe,
                        buffer,
                        toRead,
                        &bytesRead,
                        nullptr
                    ) || bytesRead == 0) {
                    break;
                }

                // Keep output bounded, but continue draining the pipe.
                const std::size_t remaining =
                    MAX_OUTPUT > output.size()
                    ? MAX_OUTPUT - output.size()
                    : 0;

                const std::size_t keep =
                    bytesRead < remaining ? bytesRead : remaining;

                output.append(buffer, keep);
            }

            if (!processFinished) {
                const DWORD waitResult =
                    WaitForSingleObject(processInfo.hProcess, 0);

                if (waitResult == WAIT_OBJECT_0) {
                    processFinished = true;
                }
                else if (waitResult == WAIT_FAILED) {
                    throw std::runtime_error(
                        "Failed to monitor process."
                    );
                }
            }

            if (processFinished && available == 0) {
                break;
            }

            const auto elapsed =
                std::chrono::steady_clock::now() - start;

            if (!processFinished &&
                elapsed > std::chrono::milliseconds(TIMEOUT_MS)) {
                timedOut = true;

                TerminateProcess(processInfo.hProcess, 1);
                WaitForSingleObject(processInfo.hProcess, INFINITE);
                break;
            }

            if (available == 0) {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(10)
                );
            }
        }

        // 8. Get exit code.
        DWORD exitCode = 1;

        if (!timedOut) {
            GetExitCodeProcess(
                processInfo.hProcess,
                &exitCode
            );
        }

        CloseHandle(readPipe);
        readPipe = nullptr;

        CloseHandle(processInfo.hThread);
        CloseHandle(processInfo.hProcess);

        if (timedOut) {
            return "Error: command timed out after 15 seconds.";
        }

        if (output.size() == MAX_OUTPUT) {
            output += "\n[Output truncated at 1 MB.]";
        }

        if (exitCode != 0) {
            return "Command failed. Exit code: " +
                   std::to_string(exitCode) +
                   "\nOutput:\n" + output;
        }

        return output.empty() ? "(no output)" : output;
    }
    catch (const std::exception& e) {
        if (readPipe != nullptr) {
            CloseHandle(readPipe);
        }

        if (writePipe != nullptr) {
            CloseHandle(writePipe);
        }

        if (processInfo.hThread != nullptr) {
            CloseHandle(processInfo.hThread);
        }

        if (processInfo.hProcess != nullptr) {
            CloseHandle(processInfo.hProcess);
        }

        return std::string("Error: ") + e.what();
    }
}

const std::filesystem::path&
NativeEnvironment::getWorkspace() const {
    return workspace;
}
