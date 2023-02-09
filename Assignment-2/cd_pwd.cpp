#include <iostream>
#include <unistd.h>
#include <cstring>
#include <errno.h>
using namespace std;

void pwd()
{
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == nullptr)
    {
        cerr << "pwd: " << strerror(errno) << endl;
    }
    else
    {
        cout << cwd << endl;
    }
}

void cd(const char *path)
{
    char PCuser[30];
    getlogin_r(PCuser, sizeof(PCuser)); // getlogin_r is used to get the username of the current user
    string temp;
    if (path == nullptr)
    {
        temp = "/home/";
        temp += PCuser;
        chdir(temp.c_str());
    }
    else if (strcmp(path, "~") == 0)
    {
        temp = "/home/";
        temp += PCuser;
        // cd(temp.c_str());
        chdir(temp.c_str());
    }
    else
    {

        if (chdir(path) == -1)
        {
            cerr << "cd: " << strerror(errno) << endl;
        }
        else
        {
            // cout << "Directory changed successfully\n";
            pwd();
        }
    }
}

int main(int argc, char *argv[])
{
    while (1)
    {
        string input;
        string temp;
        // cout << "Enter command: ";
        cout << "shell=> ";
        getline(cin, input);

        char *cmd = new char[input.length() + 1];
        strcpy(cmd, input.c_str());

        char *A[64];
        int cnt = 0;

        char *B = strtok(cmd, " ");
        while (B != nullptr)
        {
            A[cnt++] = B;
            B = strtok(nullptr, " ");
        }

        char PCuser[30];
        getlogin_r(PCuser, sizeof(PCuser)); // getlogin_r is used to get the username of the current user

        if (strcmp(A[0], "pwd") == 0)
        {
            pwd();
        }
        else if (strcmp(A[0], "cd") == 0)
        {
            if (cnt == 1)
            {
                temp = "/home/";
                temp += PCuser;
                cd(temp.c_str());
            }
            else if (cnt == 2)
            {
                if (strcmp(A[1], "~") == 0)
                {
                    temp = "/home/";
                    temp += PCuser;
                    cd(temp.c_str());
                }
                else
                    cd(A[1]);
            }
            else
            {
                cout << "Invalid command\n";
            }
        }
        else if (strcmp(A[0], "exit") == 0)
        {
            break;
        }
        else
        {
            cout << "Invalid command\n";
        }
    }
    cout << "Exiting...\n";
    return 0;
}
