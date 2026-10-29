
#include "memory_tool.h"

#include <nlohmann/json.hpp>

#include <sstream>
#include <stdexcept>
#include <string>

using json = nlohmann::json;

MemoryTool::MemoryTool(const std::string& databasePath) {
    if (sqlite3_open(databasePath.c_str(), &db) != SQLITE_OK) {
        std::string error =
            db ? sqlite3_errmsg(db) : "Could not allocate database";

        if (db) {
            sqlite3_close(db);
            db = nullptr;
        }

        throw std::runtime_error(error);
    }

    if (!initializeDatabase()) {
        std::string error = sqlite3_errmsg(db);
        sqlite3_close(db);
        db = nullptr;
        throw std::runtime_error(error);
    }
}

MemoryTool::~MemoryTool() {
    if (db) {
        sqlite3_close(db);
    }
}

bool MemoryTool::initializeDatabase() {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS memories (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            label TEXT NOT NULL,
            content TEXT NOT NULL,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )";

    char* errorMessage = nullptr;

    int result = sqlite3_exec(
        db,
        sql,
        nullptr,
        nullptr,
        &errorMessage
    );

    if (errorMessage) {
        sqlite3_free(errorMessage);
    }

    return result == SQLITE_OK;
}

std::string MemoryTool::getName() const {
    return "memory";
}

std::string MemoryTool::getDescription() const {
    return
        "Save or search persistent memories using SQLite. "
        "Save: {\"action\":\"save\",\"label\":\"topic\","
        "\"content\":\"information\"}. "
        "Search: {\"action\":\"search\",\"query\":\"keyword\"}.";
}

std::string MemoryTool::execute(const std::string& args) {
    try {
        const json input = json::parse(args);

        if (!input.is_object() ||
            !input.contains("action") ||
            !input["action"].is_string()) {
            return "Error: expected a string action.";
        }

        const std::string action =
            input["action"].get<std::string>();

        if (action == "save") {
            if (!input.contains("label") ||
                !input["label"].is_string() ||
                !input.contains("content") ||
                !input["content"].is_string()) {
                return "Error: save requires string label and content.";
            }

            const std::string label =
                input["label"].get<std::string>();

            const std::string content =
                input["content"].get<std::string>();

            if (label.empty() || content.empty()) {
                return "Error: label and content cannot be empty.";
            }

            const char* sql =
                "INSERT INTO memories (label, content) VALUES (?, ?);";

            sqlite3_stmt* statement = nullptr;

            if (sqlite3_prepare_v2(
                    db, sql, -1, &statement, nullptr
                ) != SQLITE_OK) {
                return std::string("SQLite error: ") +
                       sqlite3_errmsg(db);
            }

            sqlite3_bind_text(
                statement, 1, label.c_str(),
                -1, SQLITE_TRANSIENT
            );

            sqlite3_bind_text(
                statement, 2, content.c_str(),
                -1, SQLITE_TRANSIENT
            );

            const int result = sqlite3_step(statement);
            sqlite3_finalize(statement);

            if (result != SQLITE_DONE) {
                return std::string("SQLite error: ") +
                       sqlite3_errmsg(db);
            }

            return "Memory saved successfully.";
        }

        if (action == "search") {
            if (!input.contains("query") ||
                !input["query"].is_string()) {
                return "Error: search requires a string query.";
            }

            const std::string query =
                input["query"].get<std::string>();

            if (query.empty()) {
                return "Error: query cannot be empty.";
            }

            const char* sql = R"(
                SELECT id, label, content, created_at
                FROM memories
                WHERE instr(lower(label), lower(?)) > 0
                   OR instr(lower(content), lower(?)) > 0
                ORDER BY id DESC
                LIMIT 20;
            )";

            sqlite3_stmt* statement = nullptr;

            if (sqlite3_prepare_v2(
                    db, sql, -1, &statement, nullptr
                ) != SQLITE_OK) {
                return std::string("SQLite error: ") +
                       sqlite3_errmsg(db);
            }

            sqlite3_bind_text(
                statement, 1, query.c_str(),
                -1, SQLITE_TRANSIENT
            );

            sqlite3_bind_text(
                statement, 2, query.c_str(),
                -1, SQLITE_TRANSIENT
            );

            std::ostringstream output;
            int count = 0;
            int result;

            while ((result = sqlite3_step(statement)) == SQLITE_ROW) {
                ++count;

                const auto* label = sqlite3_column_text(statement, 1);
                const auto* content = sqlite3_column_text(statement, 2);
                const auto* created = sqlite3_column_text(statement, 3);

                output << count << ". "
                       << (label ? reinterpret_cast<const char*>(label) : "")
                       << "\n   "
                       << (content ? reinterpret_cast<const char*>(content) : "")
                       << "\n   Created: "
                       << (created ? reinterpret_cast<const char*>(created) : "")
                       << "\n\n";
            }

            sqlite3_finalize(statement);

            if (result != SQLITE_DONE) {
                return std::string("SQLite error: ") +
                       sqlite3_errmsg(db);
            }

            if (count == 0) {
                return "No memories found for: " + query;
            }

            return output.str();
        }

        return "Error: action must be 'save' or 'search'.";
    }
    catch (const json::exception& e) {
        return std::string("JSON error: ") + e.what();
    }
    catch (const std::exception& e) {
        return std::string("Error: ") + e.what();
    }
}
