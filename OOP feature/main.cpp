#include "shared.h"  
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

#include "candidate.h"
 

// Bar graph (graphics.h based) – must come before Windows headers
#include "drawBarGraph.h"

// Windows-specific headers
#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
// Fallback for non-Windows (Linux/Mac) - keeps original POSIX code
#include <sys/select.h>
#include <unistd.h>
#include <termios.h>
#endif

using namespace std;
// Forward declaration — tells compiler this function exists
// before we reference it in the overloaded version below
void generateReport(ostream& out,
                    const string& candidateID,
                    const string& candidateName,
                    const string& candidateAge,
                    const string& candidatePhone,
                    const map<string, int>& traitTotals);

string getPassword() {
    string password;
    char ch;
#ifdef _WIN32
    while ((ch = _getch()) != '\r') {
        if (ch == '\b') {
            if (!password.empty()) { password.pop_back(); cout << "\b \b"; }
        } else { password += ch; cout << '*'; }
    }
    cout << endl;

#endif
    return password;
}

string encryptDecrypt(const string& data) {
    char key = 'K';
    string result = data;
    for (size_t i = 0; i < result.size(); i++)
        result[i] = result[i] ^ key;
    return result;
}

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
#ifdef _WIN32
    Sleep(1000);
#else
    sleep(1);
#endif
}
                cout << "\nLockout period over. You can try again.\n";
                attempts = 0;
            }
        }
    }
    return true;
}

string getBand(int score) {
    if (score <= 9)  return "LOW";
    if (score <= 15) return "MODERATE";
    return "HIGH";
}

string getInterpretation(const string& trait, const string& band) {
    if (trait == "Neuroticism") {
        if (band == "LOW")      return "Emotionally stable, calm under pressure, resilient in stressful situations.";
        if (band == "MODERATE") return "Occasionally affected by stress, generally manages emotions adequately.";
        return                         "Emotionally reactive, may struggle under high-pressure or combat situations.";
    }
    if (trait == "Openness") {
        if (band == "LOW")      return "Prefers routine and familiar procedures, consistent and predictable.";
        if (band == "MODERATE") return "Adaptable when needed, balances tradition with flexibility.";
        return                         "Highly adaptable, creative thinker, thrives in changing environments.";
    }
    if (trait == "Extraversion") {
        if (band == "LOW")      return "Reserved, works well independently, prefers quieter roles.";
        if (band == "MODERATE") return "Comfortable in both team and solo tasks, situationally assertive.";
        return                         "Highly sociable, natural communicator, strong in leadership/team roles.";
    }
    if (trait == "Agreeableness") {
        if (band == "LOW")      return "Independent-minded, assertive, may challenge authority.";
        if (band == "MODERATE") return "Cooperative yet capable of standing firm when needed.";
        return                         "Highly cooperative, team-oriented, empathetic and supportive.";
    }
    if (trait == "Conscientiousness") {
        if (band == "LOW")      return "Flexible but may lack structure, needs external direction.";
        if (band == "MODERATE") return "Reasonably organized and reliable under normal conditions.";
        return                         "Highly disciplined, detail-oriented, strong sense of duty and responsibility.";
    }
    return "No interpretation available.";
}

string padRight(const string& s, size_t width) {
    if (s.size() >= width) return s;
    return s + string(width - s.size(), ' ');
}

string padLeft(const string& s, size_t width) {
    if (s.size() >= width) return s;
    return string(width - s.size(), ' ') + s;
}
// CandidateResult groups all candidate data into
// one object instead of 5 loose parameters.
// 'friend' declaration means generateReport can
// access private members of this struct.
struct CandidateResult {
    string id, name, age, phone;
    map<string, int> traitTotals;

    // Constructor using initialiser list
    CandidateResult(const string& i, const string& n,
                    const string& a, const string& p,
                    const map<string, int>& t)
        : id(i), name(n), age(a),
          phone(p), traitTotals(t) {}

    // Friend declaration —
    // generateReport(ostream&, CandidateResult&)
    // can access private members of this struct
    friend void generateReport(ostream&,
                                const CandidateResult&);
};

// This is FUNCTION OVERLOADING —
// same name generateReport, different parameters.
// This version takes a CandidateResult object.
// It delegates to the original 5-parameter version.
void generateReport(ostream& out,
                    const CandidateResult& cr) {
    const string& id    = cr.id;
    const string& name  = cr.name;
    const string& age   = cr.age;
    const string& phone = cr.phone;
    const map<string,int>& traits = cr.traitTotals;
    generateReport(out, id, name, age, phone, traits);
}
void generateReport(ostream& out,
                    const string& candidateID,
                    const string& candidateName,
                    const string& candidateAge,
                    const string& candidatePhone,
                    const map<string, int>& traitTotals) {

    const vector<string> order = {
        "Neuroticism", "Openness", "Extraversion", "Agreeableness", "Conscientiousness"
    };

    string strongest, weakest;
    int maxScore = -1, minScore = 999;
    for (const string& t : order) {
        auto it = traitTotals.find(t);
        if (it == traitTotals.end()) continue;
        int s = it->second;
        if (s > maxScore) { maxScore = s; strongest = t; }
        if (s < minScore) { minScore = s; weakest   = t; }
    }

    int grandTotal = 0;
    for (const auto& kv : traitTotals) grandTotal += kv.second;

    out << "================================================\n";
    out << "       NEPAL ARMY - CHARACTER ASSESSMENT REPORT\n";
    out << "================================================\n";
    out << "CandidateID : " << candidateID    << "\n";
    out << "Name        : " << candidateName  << "\n";
    out << "Age         : " << candidateAge   << "\n";
    out << "Phone       : " << candidatePhone << "\n";
    out << "------------------------------------------------\n";
    out << "TRAIT SCORES & PROFILE\n";
    out << "------------------------------------------------\n";

    int idx = 1;
    for (const string& trait : order) {
        auto it = traitTotals.find(trait);
        int score = (it != traitTotals.end()) ? it->second : 0;
        string band   = getBand(score);
        string interp = getInterpretation(trait, band);

        string label = to_string(idx) + ". " + padRight(trait, 18);
        out << label << ": " << score << "/20  [" << band << "]\n";
        out << "   -> " << interp << "\n\n";
        idx++;
    }

    out << "------------------------------------------------\n";
    out << "OVERALL SCORE            : " << grandTotal << "/100\n";
    out << "------------------------------------------------\n";
    out << "TRAIT SUMMARY\n";
    out << "  Strongest Trait  : " << strongest << " (" << maxScore << "/20)\n";
    out << "  Weakest Trait    : " << weakest   << " (" << minScore << "/20)\n";
    if (weakest == "Neuroticism" || strongest == "Neuroticism")
        out << "  * Note: Low Neuroticism = High Emotional Stability\n";
    out << "\n";
    out << "================================================\n\n";
}

bool parseCandidateFile(const string& filePath,
                        string& candidateID,
                        string& candidateName,
                        string& candidateAge,
                        string& candidatePhone,
                        map<string, int>& traitTotals) {
    ifstream candFile(filePath);
    if (!candFile) return false;

    string currentTrait;
    string line;

    while (getline(candFile, line)) {
        line = trim(line);
        if      (line.substr(0, 12) == "CandidateID:") candidateID    = trim(line.substr(12));
        else if (line.substr(0, 5)  == "Name:")        candidateName  = trim(line.substr(5));
        else if (line.substr(0, 4)  == "Age:")         candidateAge   = trim(line.substr(4));
        else if (line.substr(0, 6)  == "Phone:")       candidatePhone = trim(line.substr(6));
        else if (line.substr(0, 7)  == "Trait: ")      currentTrait   = trim(line.substr(7));
        else if (line.substr(0, 6)  == "Marks:") {
            try {
                int m = stoi(trim(line.substr(6)));
                if (!currentTrait.empty()) traitTotals[currentTrait] += m;
            } catch (...) {}
        }
    }
    candFile.close();
    return !candidateID.empty();
}

struct RegistryEntry { string id, name, file; };

vector<RegistryEntry> loadRegistry(const string& registryFile) {
    vector<RegistryEntry> entries;
    ifstream reg(registryFile);
    if (!reg) return entries;
    string line;
    while (getline(reg, line)) {
        line = trim(line);
        if (line.empty()) continue;
        size_t p1 = line.find('|');
        size_t p2 = line.find('|', p1 + 1);
        if (p1 == string::npos || p2 == string::npos) continue;
        RegistryEntry e;
        e.id   = trim(line.substr(0, p1));
        e.name = trim(line.substr(p1 + 1, p2 - p1 - 1));
        e.file = trim(line.substr(p2 + 1));
        entries.push_back(e);
    }
    return entries;
}


// ─────────────────────────────────────────────
//  Timed input: Windows implementation
//  Returns true if input was received within
//  timeoutSeconds, false on timeout.
// ─────────────────────────────────────────────


// ─────────────────────────────────────────────
//  ADMIN PANEL
// ─────────────────────────────────────────────

class PersonaAnalyser : public User {
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
    }       // Parses "2.5" into {2, 5} for sorting questions by trait and number

    bool compareIDs(const string& id1, const string& id2) {
        pair<int,int> p1 = parseQuestionID(id1);
        pair<int,int> p2 = parseQuestionID(id2);
        if (p1.first != p2.first) return p1.first < p2.first;
        return p1.second < p2.second;
    }       // Compares two question IDs for sorting: first by trait number, then by question number

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
 

    // ── showMenu override ─────────────────────
    // Defined inline here since PersonaAnalyser
    // has no separate .cpp file.
    void showMenu() override {
        cout << "\n";
cout << "╔══════════════════════════════════════════════╗\n";
cout << "║               ADMIN PANEL                    ║\n";
cout << "╠══════════════════════════════════════════════╣\n";
cout << "║  [1] Add Question                            ║\n";
cout << "║  [2] View Questions                          ║\n";
cout << "║  [3] Delete Question                         ║\n";
cout << "║  [4] Calculate All Candidate Scores          ║\n";
cout << "║  [5] View Result by Candidate ID             ║\n";
cout << "║  [6] Logout                                  ║\n";
cout << "╠══════════════════════════════════════════════╣\n";
cout << "║           *SELECT AN OPTION TO CONTINUE*     ║\n";
cout << "╚══════════════════════════════════════════════╝\n";
cout << "Enter Choice : ";
    }


    string selectTrait() {
        int choice;
        while (true) {
            cout << "\nSelect Trait for the Question:\n";
            cout << "1. Openness\n2. Conscientiousness\n3. Extraversion\n4. Agreeableness\n5. Neuroticism\n";
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

    void deleteQuestion() {
        loadFromFile();
        if (questions.empty()) { cout << "\nNo questions available to delete.\n"; return; }

        cout << "\n===== Available Questions =====\n";
        for (const Question& q : questions)
            cout << "  ID: " << q.questionID << " | [" << q.trait << "] " << q.questionText << "\n";

        cout << "\nEnter the Question ID to delete (e.g. 1.3): ";
        string targetID;
        cin >> targetID;
        targetID = trim(targetID);

        auto it = remove_if(questions.begin(), questions.end(),
            [&](const Question& q) { return q.questionID == targetID; });

        if (it == questions.end()) { cout << "\nQuestion ID '" << targetID << "' not found.\n"; return; }

        questions.erase(it, questions.end());

        map<string, int> traitCounter;
        for (Question& q : questions) {
            int traitNum = traitMap.count(q.trait) ? traitMap.at(q.trait) : 0;
            traitCounter[q.trait]++;
            q.questionID = to_string(traitNum) + "." + to_string(traitCounter[q.trait]);
        }

        saveAllToFile();
        cout << "\nQuestion " << targetID << " deleted and IDs renumbered successfully.\n";
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

    void loadFromFile()    { questions = loadQuestionsFromFile("questions.txt"); }

    void saveAllToFile() {
        ofstream file("questions.txt", ios::trunc);
        if (!file) { cout << "Error opening file!\n"; return; }
        for (const Question& q : questions) writeQuestionToFile(file, q);
        file.close();
    }

    void sortQuestions() {
        loadFromFile();
        if (questions.empty()) { cout << "\nNo questions to sort!\n"; return; }
        quickSort(questions, 0, questions.size() - 1);
        saveAllToFile();
        cout << "\nQuestions sorted successfully by ID (e.g., 1.1, 1.2, 2.1, ...)!\n";
    }

    void calculateScores() {
        const string registryFile = "candidate_registry.txt";
        const string scoreFile    = "candidate_scores.txt";

        vector<RegistryEntry> entries = loadRegistry(registryFile);
        if (entries.empty()) {
            cout << "\nNo candidate registry found. No candidates have taken the test yet.\n";
            return;
        }

        ofstream scoreOut(scoreFile, ios::trunc);
        if (!scoreOut) { cout << "\nCould not open " << scoreFile << " for writing.\n"; return; }

        scoreOut << "================================================\n";
        scoreOut << "        NEPAL ARMY - CANDIDATE SCORE SUMMARY\n";
        scoreOut << "================================================\n\n";

        cout << "\n================================================\n";
        cout << "        NEPAL ARMY - CANDIDATE SCORE SUMMARY\n";
        cout << "================================================\n\n";

        int candidatesProcessed = 0;

        for (const RegistryEntry& entry : entries) {
            string candidateID, candidateName, candidateAge, candidatePhone;
            map<string, int> traitTotals;

            if (!parseCandidateFile(entry.file, candidateID, candidateName,
                                    candidateAge, candidatePhone, traitTotals)) {
                cout << "Warning: Could not read file for candidate " << entry.id << "\n";
                continue;
            }

            generateReport(cout,      candidateID, candidateName, candidateAge, candidatePhone, traitTotals);
            generateReport(scoreOut,  candidateID, candidateName, candidateAge, candidatePhone, traitTotals);
            candidatesProcessed++;
        }

        scoreOut.close();

        if (candidatesProcessed == 0)
            cout << "\nNo valid candidate data found.\n";
        else
            cout << "\nTotal " << candidatesProcessed
                 << " candidate report(s) displayed above and saved to '" << scoreFile << "'.\n";
    }

    void viewCandidateResult() {
        const string registryFile = "candidate_registry.txt";

        vector<RegistryEntry> entries = loadRegistry(registryFile);
        if (entries.empty()) {
            cout << "\nNo candidate registry found. No candidates have taken the test yet.\n";
            return;
        }

        size_t maxNameLen = 4;
        for (const RegistryEntry& e : entries)
            if (e.name.size() > maxNameLen) maxNameLen = e.name.size();

        const string divider = "+---------+-" + string(maxNameLen, '-') + "-+";

        cout << "\n" << divider << "\n";
        cout << "| " << padRight("ID",   7) << " | " << padRight("Name", maxNameLen) << " |\n";
        cout << divider << "\n";
        for (const RegistryEntry& e : entries)
            cout << "| " << padRight(e.id,   7) << " | " << padRight(e.name, maxNameLen) << " |\n";
        cout << divider << "\n";

        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "\nEnter Candidate ID: ";
        string targetID;
        getline(cin, targetID);
        targetID = trim(targetID);

        string targetUpper = targetID;
        for (char& c : targetUpper) c = toupper((unsigned char)c);

        string matchedFile;
        for (const RegistryEntry& e : entries) {
            string regUpper = e.id;
            for (char& c : regUpper) c = toupper((unsigned char)c);
            if (regUpper == targetUpper) { matchedFile = e.file; break; }
        }

        if (matchedFile.empty()) {
            cout << "\nCandidate ID '" << targetID << "' not found.\n";
            return;
        }

        string candidateID, candidateName, candidateAge, candidatePhone;
        map<string, int> traitTotals;

        if (!parseCandidateFile(matchedFile, candidateID, candidateName,
                                candidateAge, candidatePhone, traitTotals)) {
            cout << "\nCould not read response file for candidate '" << targetID << "'.\n";
            return;
        }

        cout << "\n";
        generateReport(cout, candidateID, candidateName, candidateAge, candidatePhone, traitTotals);

        // ── Open bar graph in a side-by-side graphics window ─────
        drawCandidateBarGraph(candidateID, candidateName, traitTotals);
    }
};


// ─────────────────────────────────────────────
//  CANDIDATE ASSESSMENT
// ─────────────────────────────────────────────




// ─────────────────────────────────────────────
//  MAIN PANEL
// ─────────────────────────────────────────────

int main() {
#ifdef _WIN32
    // Enable UTF-8 output on Windows console (for box-drawing characters)
    SetConsoleOutputCP(CP_UTF8);
#endif
    PersonaAnalyser admin;
    CandidateAssessment candidate;
// ── POLYMORPHISM ──────────────────────────────
// User* is a BASE CLASS POINTER.
// It can point to ANY derived class object.
// When showMenu() is called through this pointer,
// C++ decides AT RUNTIME which version to call —
// that is Runtime Polymorphism.
User* panels[2] = { &admin, &candidate };

// Example: show welcome for whichever panel
// is active — same call, different behaviour:
// panels[0]->displayWelcome();  // Admin welcome
// panels[1]->displayWelcome();  // Candidate welcome
//
    int mainChoice;
    do {
         printf("╔═════════════════════════════════════════════════════════════════════╗\n");
        printf("║                    ██████╗   █████╗  ██╗  ██╗                        ║\n");
        printf("║                    ██╔══██╗ ██╔══██╗ ██║  ██║                        ║\n");
        printf("║                    ██████╔╝ ███████║ ███████║                        ║\n");
        printf("║                    ██╔═══╝  ██╔══██║ ██╔══██║                        ║\n");
        printf("║                    ██║      ██║  ██║ ██║  ██║                        ║\n");
        printf("║                    ╚═╝      ╚═╝  ╚═╝ ╚═╝  ╚═╝                        ║\n");
        printf("║                                                                      ║\n");
        printf("╠══════════════════════════════════════════════════════════════════════╣\n");
        printf(" ║                   *PERSONA ANALYSER FOR HIRING*                    ║\n");
        printf("╠══════════════════════════════════════════════════════════════════════╣\n");
        printf("║                                                                      ║\n");
        printf("║      ▸ [1] ADMIN PANEL                                               ║\n");
        printf("║             ➤ Manage candidates, results & system settings           ║\n");
        printf("║                                                                      ║\n");
        printf("║      ▸ [2] CANDIDATE PANEL                                           ║\n");
        printf("║             ➤ Conduct Assessment                                     ║\n");
        printf("║                                                                      ║\n");
        printf("║      ▸ [3] EXIT                                                      ║\n");
        printf("║             ➤ Close the application safely                           ║\n");
        printf("║                                                                      ║\n");
        printf("╠══════════════════════════════════════════════════════════════════════╣\n");
        printf(" ║                  *SELECT AN OPTION TO CONTINUE*                    ║\n");
        printf("╚══════════════════════════════════════════════════════════════════════╝\n");
        printf("\n");
        printf("                         ► Enter Choice : ");

        if (!(cin >> mainChoice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "\nInvalid input!\n";
            continue;
        }

        switch (mainChoice) {

            case 1: {
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                if (!adminLogin()) break;

                int adminChoice;
                do {
                    admin.showMenu();
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
                        case 2: admin.viewQuestions();       break;
                        case 3: admin.deleteQuestion();      break;
                        case 4: admin.calculateScores();     break;
                        case 5: admin.viewCandidateResult(); break;
                        case 6: cout << "\nLogout Successful.\n"; break;
                        default: cout << "\nInvalid Choice!\n";
                    }
                } while (adminChoice != 6);
                break;
            }

            case 2:
                //candidate.startAssessment();
                
                candidate.showMenu();
                break;

            case 3:
                cout << "\nExiting Persona Analyser. Goodbye!\n";
                break;

            default:
                cout << "\nInvalid Choice!\n";
        }

    } while (mainChoice != 3);

    return 0;
}