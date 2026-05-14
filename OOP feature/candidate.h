#ifndef CANDIDATE_H
#define CANDIDATE_H

#include <string>
#include <fstream>
#include <vector>
#include "shared.h"   // brings in Question, trim, loadQuestionsFromFile

class CandidateAssessment : public User {
private:
    struct AnswerResult {
        std::string choice;
        int         elapsedSeconds;
    };

    struct ResponseEntry {
        Question     question;
        AnswerResult answer;
    };

    std::string candidateID;
    std::string candidatePhone;
    int         candidateAge = 0;
     // ── Static Member ─────────────────────────
    // 'static' means this variable belongs to
    // the CLASS ITSELF, not to any single object.
    // All CandidateAssessment objects share ONE
    // copy of sessionCount.
    // Declared here, defined in candidate.cpp.
    static int sessionCount;

    std::string  generateCandidateID();
    void         registerCandidate(const std::string& fileName);
    AnswerResult waitForOptionWithTimeout(int totalSeconds);
    void         saveResponse(std::ofstream& out,
                              const Question& q,
                              const AnswerResult& answer);
    void         saveCandidateHeader(std::ofstream& out);

public:
   

    // ── showMenu override ─────────────────────
    // Satisfies the pure virtual requirement
    // from User. Without this, this class also
    // becomes abstract and won't compile.
    void showMenu() override;

    void startAssessment();

    // ── Static method ─────────────────────────
    // Can be called WITHOUT any object:
    //   CandidateAssessment::getTotalSessions()
    // Useful to check session count anytime.
    static int getTotalSessions() {
        return sessionCount;
    }
    
};

#endif
