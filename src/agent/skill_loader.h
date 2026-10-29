
#pragma once

#include <filesystem>
#include <string>
#include <vector>

struct Skill {
    std::string name;
    std::vector<std::string> keywords;
    std::string content;
};

class SkillLoader {
private:
    std::filesystem::path skillsDirectory;
    std::vector<Skill> skills;

public:
    explicit SkillLoader(
        const std::filesystem::path& directory = "skills"
    );

    void loadSkills();

    std::vector<Skill> getRelevantSkills(
        const std::string& query
    ) const;

    std::string buildSkillPrompt(
        const std::string& query
    ) const;

    std::size_t getSkillCount() const;
};
