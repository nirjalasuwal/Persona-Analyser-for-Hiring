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
// ── ADDITION 1: Operator Overloading ─────────
// Add this RIGHT AFTER the Question struct's
// closing brace } semicolon above.
//
// Overload << operator so you can write:
//     cout << someQuestion;
// instead of manually printing each field.
//
// 'inline' is required in header files to avoid
// "defined multiple times" linker errors.

inline std::ostream& operator<<(std::ostream& out,
                                 const Question& q) {
    out << "ID: "       << q.questionID   << "\n";
    out << "Trait: "    << q.trait        << "\n";
    out << "Question: " << q.questionText << "\n";
    for (int i = 0; i < 4; i++) {
        out << "  " << (char)('A' + i)
            << ". " << q.options[i]
            << "  [" << q.marks[i] << " pts]\n";
    }
    return out;   // return allows: cout << q1 << q2
}

// Overload == so you can write:
//     if (q1 == q2)
// instead of: if (q1.questionID == q2.questionID)
inline bool operator==(const Question& a,
                        const Question& b) {
    return a.questionID == b.questionID;
}


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
// ─────────────────────────────────────────────
// ADDITION : Abstract Base Class
// Add this just BEFORE the #endif at the bottom
// of shared.h
//
// WHY HERE: shared.h is already included by
// every file in the project, so every file
// automatically sees User with zero extra work.
// ─────────────────────────────────────────────

class User {
protected:
    // 'protected' means:
    //   - accessible inside this class
    //   - accessible inside any derived class
    //   - NOT accessible from outside
    // So PersonaAnalyser and CandidateAssessment
    // can both read/write userName and userRole
    // directly, without needing getters.
    string userName;
    string userRole;

public:
  

    // ── Pure Virtual Function ────────────────
    // "= 0" makes this PURE VIRTUAL.
    // Effect 1: User cannot be instantiated.
    //           User u;  // ERROR — won't compile
    // Effect 2: Every derived class MUST provide
    //           its own showMenu() or it also
    //           becomes un-instantiatable.
    virtual void showMenu() = 0;

    // ── Regular Virtual Function ─────────────
    // HAS a body, so User is not forced abstract
    // by this one alone.
    // Derived classes CAN override it, but don't
    // have to — they get this default version.
    virtual void displayWelcome() const {
        cout << "\nWelcome, " << userName
             << " [" << userRole << "]\n";
    }

    // ── Getters ───────────────────────────────
    // 'const' at the end means this function
    // promises not to modify the object.
    string getUserName() const { return userName; }
    string getUserRole() const { return userRole; }

    // ── Setter ────────────────────────────────
    void setUserName(const string& name) {
        userName = name;
    }
};

#endif   // this was already here — do not add another