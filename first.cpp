#include<iostream>
#include<stdlib.h>
#include <limits>
#include <chrono>
#include <thread>
using namespace std;
#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

using namespace std;

// ===== Password Hidden Input (Cross Platform) =====
string getPassword() {
    string password;
    char ch;

#ifdef _WIN32
    while ((ch = _getch()) != '\r') {   // Enter key
        if (ch == '\b') {               // Backspace
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

        if (ch == 127) {   // Backspace (Mac)
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

// ===== Simple Encryption Function (XOR) =====
string encryptDecrypt(string data) {
    char key = 'K';  // Secret key
    for (int i = 0; i < data.size(); i++) {
        data[i] = data[i] ^ key;
    }
    return data;
}

int main()
{ int choice;
start:
    cout << "===== Welcome to Persona System =====\n";
    cout<<"Select your role to proceed with the appropriate access permissions:" <<endl;
    cout<<"1. Admin"<<endl;
    cout<<"2. Candidate"<<endl;
    cin>>choice;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    switch(choice)
    {
        case 1:
        {
            string adminPassword = "admin123";   // Original password
            string encryptedStored = encryptDecrypt(adminPassword);
            int attempts = 0;
            const int MAX_ATTEMPTS = 3;
            const int LOCKOUT_TIME = 30;  // seconds
            bool accessGranted = false;

            while (attempts < MAX_ATTEMPTS && !accessGranted) {
                cout << "\nEnter Admin Password (Attempt " << (attempts + 1) << "/" << MAX_ATTEMPTS << "): ";
                string input = getPassword();

                if (encryptDecrypt(input) == encryptedStored) {
                    cout << "Welcome, Admin! You have full access to the system." << endl;
                    accessGranted = true;
                } else {
                    attempts++;
                    if (attempts < MAX_ATTEMPTS) {
                        cout << "Access Denied! Wrong Password. " << (MAX_ATTEMPTS - attempts) << " attempts remaining.\n";
                    } else {
                        cout << "\nAccess Denied! Maximum attempts exceeded.\n";
                        cout << "Your account is locked. Please wait " << LOCKOUT_TIME << " seconds...\n";
                        
                        // Lockout countdown
                        for (int i = LOCKOUT_TIME; i > 0; i--) {
                            cout << "\rTime remaining: " << i << " seconds  " << flush;
                            this_thread::sleep_for(chrono::seconds(1));//Wait for 1 second before continuing the next line of code.
                        }
                        cout << "\nLockout period over. You can try again.\n";
                        attempts = 0;  // Reset attempts after lockout
                    }
                }
            }
            break;
        }
            
        case 2:
            cout<<"Welcome, Candidate! You have limited access to the system."<<endl;
            break;
        default:
            cout<<"Invalid selection. Please choose either 1 for Admin or 2 for Candidate."<<endl;
            goto start;
}
    return 0;
}