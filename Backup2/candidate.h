#ifndef CANDIDATE_H
#define CANDIDATE_H

#include <string>
#include <fstream>
#include <vector>
#include "shared.h"   // brings in Question, trim, loadQuestionsFromFile

class CandidateAssessment {
private:
    struct AnswerResult {
        std::string choice;
        int         elapsedSeconds;
    };

    struct ResponseEntry {
        Question     question;   // Question now known via shared.h
        AnswerResult answer;
    };

    std::string candidateID;
    std::string candidateName;
    std::string candidatePhone;
    int         candidateAge = 0;

    std::string  generateCandidateID();
    void         registerCandidate(const std::string& fileName);
    AnswerResult waitForOptionWithTimeout(int totalSeconds);
    void         saveResponse(std::ofstream& out,
                              const Question& q,
                              const AnswerResult& answer);
    void         saveCandidateHeader(std::ofstream& out);

public:
    void startAssessment();
};

#endif