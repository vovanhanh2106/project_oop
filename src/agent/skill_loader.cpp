
#include "skill_loader.h"
#include <iostream>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <string>

namespace {

std::string trim(const std::string& text) {
    const auto start = text.find_first_not_of(" \t\r\n");

    if (start == std::string::npos) {
        return "";
    }

    const auto end = text.find_last_not_of(" \t\r\n");

    return text.substr(start, end - start + 1);
}

std::string toLower(std::string text) {
    std::transform(
        text.begin(),
        text.end(),
        text.begin(),
        [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        }
    );

    return text;
}

std::vector<std::string> parseKeywords(
    const std::string& line
) {
    std::vector<std::string> keywords;

    // Định dạng: keywords: plan, task, organize
    const auto colon = line.find(':');

    if (colon == std::string::npos) {
        return keywords;
    }

    std::stringstream stream(line.substr(colon + 1));
    std::string keyword;

    while (std::getline(stream, keyword, ',')) {
        keyword = toLower(trim(keyword));

        if (!keyword.empty()) {
            keywords.push_back(keyword);
        }
    }

    return keywords;
}

}

SkillLoader::SkillLoader(
    const std::filesystem::path& directory
)
    : skillsDirectory(directory) {}

void SkillLoader::loadSkills() {
    skills.clear();
    std::cout << "Skills directory: "
          << std::filesystem::absolute(skillsDirectory)
          << "\n";
    if (!std::filesystem::exists(skillsDirectory)) {
    std::cout << "ERROR: skills directory does not exist!\n";
    return;
}

if (!std::filesystem::is_directory(skillsDirectory)) {
    std::cout << "ERROR: skills path is not a directory!\n";
    return;
}

    for (const auto& entry :
         std::filesystem::directory_iterator(skillsDirectory)) {

        if (!entry.is_regular_file() ||
            entry.path().extension() != ".md") {
            continue;
        }

        std::ifstream file(entry.path());

        if (!file) {
            continue;
        }

        std::string firstLine;

        if (!std::getline(file, firstLine)) {
            continue;
        }

        // Bắt buộc dòng đầu phải có dạng "keywords: ..."
        if (toLower(firstLine).find("keywords:") != 0) {
            continue;
        }

        Skill skill;
        skill.name = entry.path().stem().string();
        skill.keywords = parseKeywords(firstLine);

        std::ostringstream content;
        content << file.rdbuf();

        skill.content = content.str();

        if (!skill.keywords.empty()) {
            skills.push_back(std::move(skill));
        }
    }
}

std::vector<Skill> SkillLoader::getRelevantSkills(
    const std::string& query
) const {
    std::vector<Skill> relevantSkills;

    const std::string normalizedQuery = toLower(query);

    for (const Skill& skill : skills) {
        for (const std::string& keyword : skill.keywords) {
            if (normalizedQuery.find(keyword) != std::string::npos) {
                relevantSkills.push_back(skill);
                break;
            }
        }
    }

    return relevantSkills;
}

std::string SkillLoader::buildSkillPrompt(
    const std::string& query
) const {
    const auto relevantSkills = getRelevantSkills(query);

    if (relevantSkills.empty()) {
        return "";
    }

    std::ostringstream prompt;

    prompt << "Relevant skills and instructions:\n";

    for (const Skill& skill : relevantSkills) {
        prompt << "\n--- Skill: " << skill.name << " ---\n";
        prompt << skill.content << "\n";
    }

    return prompt.str();
}

std::size_t SkillLoader::getSkillCount() const {
    return skills.size();
}
