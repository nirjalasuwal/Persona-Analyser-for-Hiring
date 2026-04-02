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
#include <sys/select.h>
#include <unistd.h>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#endif

using namespace std;


//  Shared Data Structure

struct Question {
    string questionID;
    string trait;
    string questionText;
    string options[4];
    int    marks[4];
};


//  Utility helpers

string trim(const string& input) {
    size_t start = 0;
    while (start < input.size() && isspace((unsigned char)input[start])) start++;
    size_t end = input.size();
    while (end > start && isspace((unsigned char)input[end - 1])) end--;
    return input.substr(start, end - start);
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

vector<Question> loadQuestionsFromFile(const string& fileName) {
    ifstream file(fileName);
    vector<Question> loaded;
    if (!file) return loaded;

    string line;
    Question current;
    bool inQuestion = false;
    int optionsRead = 0;

    while (getline(file, line)) {
        if (line.substr(0, 11) == "QuestionID:") {
            if (inQuestion && optionsRead == 4) loaded.push_back(current);
            current = Question();
            current.questionID = trim(line.substr(11));
            inQuestion   = true;
            optionsRead  = 0;
        } else if (inQuestion && line.substr(0, 7) == "Trait: ") {
            current.trait = trim(line.substr(7));
        } else if (inQuestion && line.substr(0, 9) == "OptionID:") {
            if (optionsRead < 4) {
                size_t firstPipe  = line.find('|');
                size_t secondPipe = line.find('|', firstPipe == string::npos ? 0 : firstPipe + 1);
                size_t marksPos   = line.find("Marks:");
                if (firstPipe != string::npos && secondPipe != string::npos && marksPos != string::npos) {
                    current.options[optionsRead] = trim(line.substr(firstPipe + 1, secondPipe - firstPipe - 1));
                    try {
                        current.marks[optionsRead] = stoi(trim(line.substr(marksPos + 6)));
                        optionsRead++;
                    } catch (...) { inQuestion = false; optionsRead = 0; }
                }
            }
        } else if (inQuestion && line == "----------------------------------") {
            if (optionsRead == 4) loaded.push_back(current);
            inQuestion  = false;
            optionsRead = 0;
        } else if (inQuestion && current.questionText.empty() && !line.empty()) {
            current.questionText = line;
        }
    }

    if (inQuestion && optionsRead == 4) loaded.push_back(current);
    return loaded;
}


//  Password Hidden Input (Cross Platform)

string getPassword() {
    string password;
    char ch;
#ifdef _WIN32
    while ((ch = _getch()) != '\r') {
        if (ch == '\b') {
            if (!password.empty()) {
                password.pop_back();
                cout << "\b \b";
            }
        } else {
            password += ch;
            cout << '*';
        }
    }
#else
    termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    while (true) {
        ch = getchar();
        if (ch == '\n') break;
        if (ch == 127) {
            if (!password.empty()) {
                password.pop_back();
                cout << "\b \b";
            }
        } else {
            password += ch;
            cout << '*';
        }
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
    cout << endl;
    return password;
}


//  Simple XOR Encryption / Decryption

string encryptDecrypt(const string& data) {
    char key = 'K';
    string result = data;
    for (size_t i = 0; i < result.size(); i++)
        result[i] = result[i] ^ key;
    return result;
}


//  Admin Login Gate

bool adminLogin() {
    const string adminPassword   = "admin123";
    const string encryptedStored = encryptDecrypt(adminPassword);
    const int    MAX_ATTEMPTS    = 3;
    const int    LOCKOUT_TIME    = 30;

    int  attempts      = 0;
    bool accessGranted = false;

    while (!accessGranted) {
        cout << "\nEnter Admin Password (Attempt " << (attempts + 1) << "/" << MAX_ATTEMPTS << "): ";
        string input = getPassword();

        if (encryptDecrypt(input) == encryptedStored) {
            cout << "Welcome, Admin! You have full access to the system." << endl;
            accessGranted = true;
        } else {
            attempts++;
            if (attempts < MAX_ATTEMPTS) {
                cout << "Access Denied! Wrong Password. "
                     << (MAX_ATTEMPTS - attempts) << " attempts remaining.\n";
            } else {
                cout << "\nAccess Denied! Maximum attempts exceeded.\n";
                cout << "Your account is locked. Please wait " << LOCKOUT_TIME << " seconds...\n";

                for (int i = LOCKOUT_TIME; i > 0; i--) {
                    cout << "\rTime remaining: " << i << " seconds  " << flush;
                    this_thread::sleep_for(chrono::seconds(1));
                }
                cout << "\nLockout period over. You can try again.\n";
                attempts = 0;
            }
        }
    }
    return true;
}


//  ADMIN PANEL

class PersonaAnalyser {
private:
    vector<Question> questions;
    const map<string, int> traitMap = {
        {"Openness", 1}, {"Conscientiousness", 2}, {"Extraversion", 3},
        {"Agreeableness", 4}, {"Neuroticism", 5}
    };

    pair<int, int> parseQuestionID(const string& id) {
        size_t dotPos = id.find('.');
        if (dotPos == string::npos) return {0, 0};
        return {stoi(id.substr(0, dotPos)), stoi(id.substr(dotPos + 1))};
    }

    bool compareIDs(const string& id1, const string& id2) {
        pair<int,int> p1 = parseQuestionID(id1);
        pair<int,int> p2 = parseQuestionID(id2);
        if (p1.first != p2.first) return p1.first < p2.first;
        return p1.second < p2.second;
    }

    int partition(vector<Question>& arr, int low, int high) {
        string pivot = arr[high].questionID;
        int i = low - 1;
        for (int j = low; j < high; j++) {
            if (compareIDs(arr[j].questionID, pivot)) {
                i++;
                swap(arr[i], arr[j]);
            }
        }
        swap(arr[i + 1], arr[high]);
        return i + 1;
    }

    void quickSort(vector<Question>& arr, int low, int high) {
        if (low < high) {
            int pi = partition(arr, low, high);
            quickSort(arr, low, pi - 1);
            quickSort(arr, pi + 1, high);
        }
    }

    bool assignMarksByPriority(const string& priorityInput, int marks[4]) {
        istringstream ss(priorityInput);
        string token;
        vector<string> order;

        while (ss >> token) {
            if (token != "A" && token != "B" && token != "C" && token != "D") {
                cout << "Invalid option '" << token << "'. Only A, B, C, D allowed.\n";
                return false;
            }
            for (const string& e : order) {
                if (e == token) {
                    cout << "Duplicate option '" << token << "' found.\n";
                    return false;
                }
            }
            order.push_back(token);
        }

        if (order.size() != 4) {
            cout << "You must rank exactly 4 options (A, B, C, D).\n";
            return false;
        }

        int pts = 4;
        for (const string& opt : order) marks[opt[0] - 'A'] = pts--;
        return true;
    }

    void writeQuestionToFile(ofstream& file, const Question& q) {
        file << "QuestionID:" << q.questionID << "\n";
        file << "Trait: "     << q.trait      << "\n";
        file << q.questionText                << "\n";
        for (int i = 0; i < 4; i++)
            file << "OptionID: " << q.questionID << (char)('A' + i)
                 << " | " << q.options[i] << " | Marks: " << q.marks[i] << "\n";
        file << "----------------------------------\n";
    }

public:
    PersonaAnalyser() { loadFromFile(); }

    string selectTrait() {
        int choice;
        while (true) {
            cout << "\nSelect Trait for the Question:\n";
            cout << "1. Openness\n";
            cout << "2. Conscientiousness\n";
            cout << "3. Extraversion\n";
            cout << "4. Agreeableness\n";
            cout << "5. Neuroticism\n";
            cout << "Enter choice: ";
            cin >> choice;
            switch (choice) {
                case 1: return "Openness";
                case 2: return "Conscientiousness";
                case 3: return "Extraversion";
                case 4: return "Agreeableness";
                case 5: return "Neuroticism";
                default: cout << "Wrong option. Try again.\n";
            }
        }
    }

    string generateQuestionID(const string& trait) {
        ifstream file("questions.txt");
        string line;
        int count = 0;
        while (getline(file, line))
            if (line.find("Trait: " + trait) != string::npos) count++;
        file.close();
        int traitNumber = traitMap.count(trait) ? traitMap.at(trait) : 0;
        return to_string(traitNumber) + "." + to_string(count + 1);
    }

    void addQuestion() {
        Question q;
        q.trait      = selectTrait();
        q.questionID = generateQuestionID(q.trait);

        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        cout << "\nEnter Question: ";
        getline(cin, q.questionText);

        cout << "Enter Option A: "; getline(cin, q.options[0]);
        cout << "Enter Option B: "; getline(cin, q.options[1]);
        cout << "Enter Option C: "; getline(cin, q.options[2]);
        cout << "Enter Option D: "; getline(cin, q.options[3]);

        string priorityInput;
        bool firstAttempt = true;
        do {
            cout << "\nEnter options in priority order (highest to lowest), separated by spaces.\n";
            cout << "Example: A C D B  ->  A=4pts, C=3pts, D=2pts, B=1pt\n";
            cout << "Your ranking: ";
            if (!firstAttempt) cin.clear();
            getline(cin, priorityInput);
            firstAttempt = false;
            for (char& c : priorityInput) c = toupper(c);
        } while (!assignMarksByPriority(priorityInput, q.marks));

        cout << "\nMarks assigned:\n";
        for (int i = 0; i < 4; i++)
            cout << "  Option " << (char)('A' + i)
                 << " (" << q.options[i] << "): " << q.marks[i] << " pts\n";

        questions.push_back(q);
        saveToFile(q);
        cout << "\nQuestion Added Successfully!\n";
    }

    void saveToFile(const Question& q) {
        ofstream file("questions.txt", ios::app);
        if (!file) { cout << "Error opening file!\n"; return; }
        writeQuestionToFile(file, q);
        file.close();
    }

    void viewQuestions() {
        sortQuestions();
        ifstream file("questions.txt");
        if (!file) { cout << "\nNo questions found!\n"; return; }
        string line;
        cout << "\n===== Stored Questions =====\n\n";
        while (getline(file, line)) cout << line << endl;
        file.close();
        cout << "\n===== End of Questions =====\n";
    }

    void loadFromFile() {
        questions = loadQuestionsFromFile("questions.txt");
    }

    void saveAllToFile() {
        ofstream file("questions.txt", ios::trunc);
        if (!file) { cout << "Error opening file!\n"; return; }
        for (const Question& q : questions)
            writeQuestionToFile(file, q);
        file.close();
    }

    void sortQuestions() {
        loadFromFile();
        if (questions.empty()) { cout << "\nNo questions to sort!\n"; return; }
        quickSort(questions, 0, questions.size() - 1);
        saveAllToFile();
        cout << "\nQuestions sorted successfully by ID (e.g., 1.1, 1.2, 2.1, ...)!\n";
    }
};


//  CANDIDATE ASSESSMENT

class CandidateAssessment {
private:
    struct AnswerResult {
        string choice;
        int    elapsedSeconds;
    };

    string candidateID;
    string candidateName;
    int    candidateAge  = 0;
    string candidatePhone;
    int    responseSerial = 0;
    const string responseFile = "candidate_responses.txt";

    string generateCandidateID() {
        auto now     = chrono::system_clock::now();
        auto epochMs = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()).count();
        random_device rd;
        uniform_int_distribution<int> dist(100, 999);
        return "C" + to_string(static_cast<long long>(epochMs)) + to_string(dist(rd));
    }

    int optionIndex(char option) { return option - 'A'; }

    vector<Question> buildRandomQuestionSet(const vector<Question>& allQuestions) {
        unordered_map<string, vector<Question>> byTrait;
        for (const Question& q : allQuestions) byTrait[q.trait].push_back(q);

        random_device rd;
        mt19937 generator(rd());

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

    AnswerResult waitForOptionWithTimeout(int totalSeconds) {
        auto start    = chrono::steady_clock::now();
        auto deadline = start + chrono::seconds(totalSeconds);

        while (true) {
            auto now = chrono::steady_clock::now();
            if (now >= deadline) return {"TIMEOUT", totalSeconds};

            long long remainingMs = chrono::duration_cast<chrono::milliseconds>(deadline - now).count();
            int remaining = (int)(remainingMs / 1000);
            if (remaining < 0) remaining = 0;

            
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
    }

    void saveCandidateHeader(ofstream& out) {
        out << "CandidateID: " << candidateID   << '\n';
        out << "Name: "        << candidateName  << '\n';
        out << "Age: "         << candidateAge   << '\n';
        out << "Phone: "       << candidatePhone << '\n';
        out << "==================================" << '\n';
    }

    void saveResponse(ofstream& out, const Question& q, const AnswerResult& answer) {
        responseSerial++;
        string responseID = candidateID + "-" + q.questionID + "-" + to_string(responseSerial);

        out << "ResponseID: "    << responseID     << '\n';
        out << "QuestionID: "    << q.questionID   << '\n';
        out << "Trait: "         << q.trait        << '\n';
        out << "Question: "      << q.questionText << '\n';

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

public:
    void startAssessment() {
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

        cout << "\n===== Candidate Details =====\n";
        cout << "Enter your name: ";
        getline(cin, candidateName);

        readIntWithRetry("Enter your age: ", candidateAge, 1);
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        cout << "Enter your phone number: ";
        getline(cin, candidatePhone);

        candidateID    = generateCandidateID();
        responseSerial = 0;

        ofstream out(responseFile, ios::app);
        if (!out) {
            cout << "Unable to open " << responseFile << " for writing.\n";
            return;
        }

        saveCandidateHeader(out);

        cout << "\nAssessment starts now. You get 30 seconds per question.\n";
        cout << "Enter options (type A, B, C or D)" << endl;

        for (size_t i = 0; i < questions.size(); ++i) {
            const Question& q = questions[i];
            cout << "\nQuestion " << (i + 1) << " / " << questions.size() << '\n';
            cout << q.questionText << '\n';
            cout << "A. " << q.options[0] << '\n';
            cout << "B. " << q.options[1] << '\n';
            cout << "C. " << q.options[2] << '\n';
            cout << "D. " << q.options[3] << '\n';

            AnswerResult answer = waitForOptionWithTimeout(30);

            if (answer.choice == "TIMEOUT")
                cout << "Time is up. Moving to next question.\n";

            cout << "Time taken: " << answer.elapsedSeconds << " seconds\n";
            saveResponse(out, q, answer);
        }

        out << "EndOfCandidate: " << candidateID << '\n';
        out << "==================================" << "\n\n";

        cout << "\nAssessment completed. Responses saved to " << responseFile << "\n";
    }
};


//  MAIN

int main() {
    PersonaAnalyser     admin;
    CandidateAssessment candidate;

    int mainChoice;
    do {
        cout << "\n========== PERSONA ANALYSER ==========\n";
        cout << "1. Admin Panel\n";
        cout << "2. Candidate Panel\n";
        cout << "3. Exit\n";
        cout << "Enter choice: ";

        if (!(cin >> mainChoice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "\nInvalid input!\n";
            continue;
        }

        switch (mainChoice) {

            // ADMIN PANEL
            case 1: {
                cin.ignore(numeric_limits<streamsize>::max(), '\n'); // flush leftover \n before password input
                if (!adminLogin()) break;

                int adminChoice;
                do {
                    cout << "\n===== ADMIN PANEL =====\n";
                    cout << "1. Add Question\n";
                    cout << "2. View Questions\n";
                    cout << "3. Logout\n";
                    cout << "Enter choice: ";

                    if (!(cin >> adminChoice)) {
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        cout << "\nInvalid input!\n";
                        continue;
                    }

                    switch (adminChoice) {
                        case 1: {
                            int addMore = 1;
                            while (addMore == 1) {
                                admin.addQuestion();
                                cout << "\nDo you want to add another question? (1=Yes, 0=No): ";
                                cin >> addMore;
                            }
                            break;
                        }
                        case 2:
                            admin.viewQuestions();
                            break;
                        case 3:
                            cout << "\nLogout Sucessfull.......\n";
                            break;
                        default:
                            cout << "\nInvalid Choice!\n";
                    }
                } while (adminChoice != 3);
                break;
            }

            // CANDIDATE PANEL 
            case 2:
                candidate.startAssessment();
                break;

            //  EXIT 
            case 3:
                cout << "\nExiting Persona Analyser. Goodbye!\n";
                break;

            default:
                cout << "\nInvalid Choice!\n";
        }

    } while (mainChoice != 3);

    return 0;
}
