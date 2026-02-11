#include<iostream>
#include<stdlib.h>
using namespace std;
int main()
{ int n;
start:
    cout<<"Select your role to proceed with the appropriate access permissions:" <<endl;
    cout<<"1. Admin"<<endl;
    cout<<"2. Candidate"<<endl;
    cin>>n;
    switch(n)
    {
        case 1:
            cout<<"Welcome, Admin! You have full access to the system."<<endl;
            break;
        case 2:
            cout<<"Welcome, Candidate! You have limited access to the system."<<endl;
            break;
        default:
            cout<<"Invalid selection. Please choose either 1 for Admin or 2 for Candidate."<<endl;
            goto start;
}
    return 0;
}