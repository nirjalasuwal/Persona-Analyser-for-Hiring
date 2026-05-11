#include <iostream>
#include <fstream>
#include <vector>
#include <limits>
#include <string>
#include <sstream>
#include <map>
#include <chrono>
#include <thread>
#include <ctime>
#include <cctype>
#include <algorithm>
#include <random>
#include <unordered_map>
//#include "shared.h"

// Windows ko lagiii
#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#endif
#include "candidate.h"

using namespace std;

    string CandidateAssessment::generateCandidateID() {
        int counter = 1;
        ifstream in("candidate_counter.txt");
        if (in) in >> counter;
        in.close();

        ofstream out("candidate_counter.txt");
        out << (counter + 1);
        out.close();

        string id = "C";
        if      (counter < 10)  id += "00";
        else if (counter < 100) id += "0";
        id += to_string(counter);
        return id;
    }

    bool readIntWithRetry(const string& prompt, int& value, int minValue = numeric_limits<int>::min()) {
    while (true) {
        cout << prompt;
        if (cin >> value && value >= minValue) return true;
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input. Try again.\n";
    }
}

    void CandidateAssessment::registerCandidate(const string& fileName) {
        ofstream reg("candidate_registry.txt", ios::app);
        if (reg) reg << candidateID << "|" << candidateName << "|" << fileName << "\n";
        reg.close();
    }

    int optionIndex(char option) { return option - 'A'; }

    vector<Question> buildRandomQuestionSet(const vector<Question>& allQuestions) {
        unordered_map<string, vector<Question>> byTrait;
        for (const Question& q : allQuestions) byTrait[q.trait].push_back(q);

        // Use high-resolution clock for better seeding on Windows
        unsigned long seed = chrono::high_resolution_clock::now().time_since_epoch().count();
        mt19937 generator(seed);
        vector<Question> selected;
        selected.reserve(25);

        const vector<string> traits = {
            "Openness", "Conscientiousness", "Extraversion", "Agreeableness", "Neuroticism"
        };

        for (const string& trait : traits) {
            auto found = byTrait.find(trait);
            if (found == byTrait.end()) continue;
            vector<Question>& tq = found->second;
            shuffle(tq.begin(), tq.end(), generator);
            int take = min(5, (int)tq.size());
            for (int i = 0; i < take && selected.size() < 25; ++i)
                selected.push_back(tq[i]);
        }

        shuffle(selected.begin(), selected.end(), generator);
        return selected;
    }

    // ─────────────────────────────────────────────
    //  Timed answer: platform-specific
    // ─────────────────────────────────────────────
    CandidateAssessment::AnswerResult CandidateAssessment::waitForOptionWithTimeout(int totalSeconds) {
        auto start    = chrono::steady_clock::now();
        auto deadline = start + chrono::seconds(totalSeconds);

#ifdef _WIN32
        // Windows: poll with _kbhit() to avoid blocking on std::cin
        string inputBuffer;

        while (true) {
            auto now = chrono::steady_clock::now();
            if (now >= deadline) return {"TIMEOUT", totalSeconds};

            long long remainingMs =
                chrono::duration_cast<chrono::milliseconds>(deadline - now).count();
            int remaining = (int)(remainingMs / 1000);

            // Reprint the prompt with updated countdown
            cout << "\rYour answer (type A, B, C or D) — Time left: "
                 << remaining << "s: " << inputBuffer << "   " << flush;

            if (_kbhit()) {
                char ch = _getch();

                if (ch == '\r' || ch == '\n') {
                    cout << endl;
                    if (inputBuffer.empty()) {
                        cout << "Invalid input. Please type A, B, C, or D.\n";
                        inputBuffer.clear();
                        continue;
                    }
                    char sel = (char)toupper((unsigned char)inputBuffer[0]);
                    if (sel == 'A' || sel == 'B' || sel == 'C' || sel == 'D') {
                        auto finish = chrono::steady_clock::now();
                        int elapsed = (int)chrono::duration_cast<chrono::seconds>(finish - start).count();
                        if (elapsed < 0) elapsed = 0;
                        if (elapsed > totalSeconds) elapsed = totalSeconds;
                        return {string(1, sel), elapsed};
                    }
                    cout << "Invalid option. Please choose A, B, C, or D.\n";
                    inputBuffer.clear();

                } else if (ch == '\b') {
                    if (!inputBuffer.empty()) inputBuffer.pop_back();
                } else {
                    inputBuffer += ch;
                }
            } else {
                Sleep(200);
            }
        }

#else
        // Linux / macOS: original select()-based approach
        while (true) {
            auto now = chrono::steady_clock::now();
            if (now >= deadline) return {"TIMEOUT", totalSeconds};

            long long remainingMs = chrono::duration_cast<chrono::milliseconds>(deadline - now).count();
            int remaining = (int)(remainingMs / 1000);
            if (remaining < 0) remaining = 0;

            cout << "Your answer (type A, B, C or D) — Time left: " << remaining << "s: ";

            fd_set inputSet;
            FD_ZERO(&inputSet);
            FD_SET(STDIN_FILENO, &inputSet);

            timeval timeout;
            timeout.tv_sec  = (time_t)(remainingMs / 1000);
            timeout.tv_usec = (suseconds_t)((remainingMs % 1000) * 1000);

            int ready = select(STDIN_FILENO + 1, &inputSet, nullptr, nullptr, &timeout);
            if (ready <= 0) return {"TIMEOUT", totalSeconds};

            string input;
            getline(cin, input);
            input = trim(input);
            if (input.empty()) { cout << "Invalid input. Please type A, B, C, or D.\n"; continue; }

            char sel = (char)toupper((unsigned char)input[0]);
            if (sel == 'A' || sel == 'B' || sel == 'C' || sel == 'D') {
                auto finish = chrono::steady_clock::now();
                int elapsed = (int)chrono::duration_cast<chrono::seconds>(finish - start).count();
                if (elapsed < 0) elapsed = 0;
                if (elapsed > totalSeconds) elapsed = totalSeconds;
                return {string(1, sel), elapsed};
            }
            cout << "Invalid option. Please choose A, B, C, or D.\n";
        }
#endif
    }

    void CandidateAssessment::saveCandidateHeader(ofstream& out) {
        out << "CandidateID: " << candidateID   << '\n';
        out << "Name: "        << candidateName  << '\n';
        out << "Age: "         << candidateAge   << '\n';
        out << "Phone: "       << candidatePhone << '\n';
        out << "==================================" << '\n';
    }

    void CandidateAssessment::saveResponse(ofstream& out, const Question& q, const AnswerResult& answer) {
        out << "QuestionID: "  << q.questionID   << '\n';
        out << "Trait: "       << q.trait        << '\n';
        out << "Question: "    << q.questionText << '\n';

        if (answer.choice == "TIMEOUT") {
            out << "SelectedOption: TIMEOUT\n";
            out << "SelectedText: No answer\n";
            out << "Marks: 0\n";
        } else {
            int idx = optionIndex(answer.choice[0]);
            out << "SelectedOption: " << answer.choice[0] << '\n';
            out << "SelectedText: "   << q.options[idx]   << '\n';
            out << "Marks: "          << q.marks[idx]     << '\n';
        }
        out << "TimeTakenSeconds: " << answer.elapsedSeconds << '\n';
        out << "----------------------------------" << '\n';
    }


    void CandidateAssessment::startAssessment() {
        vector<Question> allQuestions = loadQuestionsFromFile("questions.txt");
        if (allQuestions.empty()) {
            cout << "\nNo questions available. Ask admin to add questions first.\n";
            return;
        }

        vector<Question> questions = buildRandomQuestionSet(allQuestions);
        if (questions.empty()) {
            cout << "\nNo valid random question set could be created.\n";
            return;
        }

        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        cout << "\n";
        cout << "  ╔══════════════════════════════════════════════════════════════╗\n";
        cout << "  ║           NEPAL ARMY - CHARACTER ASSESSMENT TEST            ║\n";
        cout << "  ╚══════════════════════════════════════════════════════════════╝\n";
        cout << "\n";
        cout << "  ABOUT THIS TEST\n";
        cout << "  ---------------------------------------------------------------\n";
        cout << "  This is a personality assessment which will evaluate your personality\n";
        cout << "\n";
        cout << "  TEST STRUCTURE\n";
        cout << "  ---------------------------------------------------------------\n";
        cout << "    - Total Questions    : 25\n";
        cout << "    - Time per Question  : 60 seconds\n";
        cout << "    - Total Time (max)   : ~25 minutes\n";
        cout << "    - Answer Format      : Multiple Choice (A, B, C or D)\n";
        cout << "\n";
        cout << "  INSTRUCTIONS\n";
        cout << "  ---------------------------------------------------------------\n";
        cout << "    * Read each question carefully before answering.\n";
        cout << "    * Type A, B, C, or D and press ENTER to submit your answer.\n";
        cout << "    * If you do not answer within 60 seconds, the question will\n";
        cout << "      be skipped and marked as unanswered (0 marks).\n";
        cout << "    * There are no right or wrong answers - answer honestly.\n";
        cout << "    * You cannot go back to a previous question.\n";
        cout << "    * Do not close the program during the test.\n";
        cout << "\n";
        cout << "  ---------------------------------------------------------------\n";
        cout << "  When you are ready, please enter your details below to begin.\n";
        cout << "  ---------------------------------------------------------------\n";
        cout << "\n";

        cout << "\n===== Candidate Details =====\n";
        cout << "Enter your name: ";
        getline(cin, candidateName);

        readIntWithRetry("Enter your age: ", candidateAge, 1);
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        cout << "Enter your phone number: ";
        getline(cin, candidatePhone);

        candidateID = generateCandidateID();

        string safeName = candidateName;
        replace(safeName.begin(), safeName.end(), ' ', '_');
        string fileName = safeName + "_" + candidateID + ".txt";

        ofstream out(fileName);
        if (!out) { cout << "Unable to open " << fileName << " for writing.\n"; return; }

        registerCandidate(fileName);
        saveCandidateHeader(out);

        cout << "\nAssessment starts now. You get 60 seconds per question.\n";
        cout << "Enter options (type A, B, C or D)\n" << endl;

        vector<ResponseEntry> responses;

        for (size_t i = 0; i < questions.size(); ++i) {
            const Question& q = questions[i];
            cout << "\nQuestion " << (i + 1) << " / " << questions.size() << '\n';
            cout << q.questionText << '\n';
            cout << "A. " << q.options[0] << '\n';
            cout << "B. " << q.options[1] << '\n';
            cout << "C. " << q.options[2] << '\n';
            cout << "D. " << q.options[3] << '\n';

            AnswerResult answer = waitForOptionWithTimeout(60);

            if (answer.choice == "TIMEOUT") cout << "\nTime is up. Moving to next question.\n";
            cout << "Time taken: " << answer.elapsedSeconds << " seconds\n";
            responses.push_back({q, answer});
        }

        sort(responses.begin(), responses.end(), [](const ResponseEntry& a, const ResponseEntry& b) {
            size_t dotA = a.question.questionID.find('.');
            size_t dotB = b.question.questionID.find('.');
            int majorA = stoi(a.question.questionID.substr(0, dotA));
            int majorB = stoi(b.question.questionID.substr(0, dotB));
            if (majorA != majorB) return majorA < majorB;
            int minorA = stoi(a.question.questionID.substr(dotA + 1));
            int minorB = stoi(b.question.questionID.substr(dotB + 1));
            return minorA < minorB;
        });

        for (const ResponseEntry& entry : responses)
            saveResponse(out, entry.question, entry.answer);

        out << "EndOfCandidate: " << candidateID << '\n';
        out << "==================================" << "\n\n";
        out.close();

        cout << "\nAssessment completed. Responses saved to: " << fileName << "\n";
        cout << "Your Candidate ID is: " << candidateID << "\n";
    }