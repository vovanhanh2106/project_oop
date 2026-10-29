
#pragma once

#include "tool.h"

#include <sqlite3.h>
#include <string>

class MemoryTool : public Tool {
private:
    sqlite3* db = nullptr;

    bool initializeDatabase();

public:
    explicit MemoryTool(
        const std::string& databasePath = "agent_memory.db"
    );

    ~MemoryTool() override;

    MemoryTool(const MemoryTool&) = delete;
    MemoryTool& operator=(const MemoryTool&) = delete;

    std::string getName() const override;

    std::string getDescription() const override;

    std::string execute(const std::string& args) override;
};
