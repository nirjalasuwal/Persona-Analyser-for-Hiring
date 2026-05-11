#ifndef SHARED_H
#define SHARED_H

#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <limits>
#include <map>

using namespace std;

struct Question {
    std::string questionID;
    std::string trait;
    std::string questionText;
    std::string options[4];
    int         marks[4];
};

// Trim whitespace — defined once, used everywhere
inline std::string trim(const std::string& input) {
    size_t start = 0;
    while (start < input.size() && isspace((unsigned char)input[start])) start++;
    size_t end = input.size();
    while (end > start && isspace((unsigned char)input[end - 1])) end--;
    return input.substr(start, end - start);
}



// Load questions from file — defined once, used everywhere
inline std::vector<Question> loadQuestionsFromFile(const std::string& fileName) {
    std::ifstream file(fileName);
    std::vector<Question> loaded;
    if (!file) return loaded;

    std::string line;
    Question current;
    bool inQuestion = false;
    int  optionsRead = 0;

    while (std::getline(file, line)) {
        if (line.substr(0, 11) == "QuestionID:") {
            if (inQuestion && optionsRead == 4) loaded.push_back(current);
            current      = Question();
            current.questionID = trim(line.substr(11));
            inQuestion   = true;
            optionsRead  = 0;
        } else if (inQuestion && line.substr(0, 7) == "Trait: ") {
            current.trait = trim(line.substr(7));
        } else if (inQuestion && line.substr(0, 9) == "OptionID:") {
            if (optionsRead < 4) {
                size_t p1 = line.find('|');
                size_t p2 = line.find('|', p1 == std::string::npos ? 0 : p1 + 1);
                size_t mp = line.find("Marks:");
                if (p1 != std::string::npos && p2 != std::string::npos && mp != std::string::npos) {
                    current.options[optionsRead] = trim(line.substr(p1 + 1, p2 - p1 - 1));
                    try {
                        current.marks[optionsRead] = std::stoi(trim(line.substr(mp + 6)));
                        optionsRead++;
                    } catch (...) { inQuestion = false; optionsRead = 0; }
                }
            }
        } else if (inQuestion && line == "----------------------------------") {
            if (optionsRead == 4) loaded.push_back(current);
            inQuestion = false; optionsRead = 0;
        } else if (inQuestion && current.questionText.empty() && !line.empty()) {
            current.questionText = line;
        }
    }
    if (inQuestion && optionsRead == 4) loaded.push_back(current);
    return loaded;
}

#endif