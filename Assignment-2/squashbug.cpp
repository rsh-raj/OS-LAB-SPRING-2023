
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <vector>
#include <map>
using namespace std;
// map<char *, long long> processStartTime;

vector<pair<char *, long long>> processStartTime;
map<char *, char *> processName;
map<char *, long long> totalDescendants;
char *setProcessTime(char *PID)
{
    char path[100] = "/proc/";
    strcat(path, PID);
    strcat(path, "/stat");

    FILE *in_file = fopen(path, "r");
    if (!in_file)
    {
        printf("Unable get the processs information, check if PID is correct\n");
        return "-1";
    }
    long long i = 1, ticksSinceStart;
    char *ppid;
    char info[2000];
    fgets(info, 2000, in_file);
    char *token = strtok(info, " ");
    char *pend, *name;

    while (token != NULL)
    {
        switch (i)
        {

        case 2:
            name = strdup(token);
            break;
        case 4:
            ppid = strdup(token);
            break;
        case 22:
            ticksSinceStart = strtol(token, &pend, 10);
        }
        i++;
        token = strtok(NULL, " ");
    }
    processStartTime.push_back({PID, ticksSinceStart});
    processName[PID] = strdup(name);
    return ppid;
    // printf("%d %s %d", ppid, name,ticksSinceStart);
}
void populateProcessTimeMap(char *PID)
{
    char *ppid = strdup(PID), *pend;
    while (strtol(ppid, &pend, 10) != 0)
    {
        ppid = setProcessTime(ppid);
    }
}
long long getNumberOfChild(char *PID)
{
    char path[100] = "/proc/";
    strcat(path, PID);
    strcat(path, "/task/");
    strcat(path, PID);
    strcat(path, "/children");
    FILE *in_file = fopen(path, "r");
    char info[2000];
    fgets(info, 2000, in_file);
    char *token = strtok(info, " ");
    long long nChildren = 0;

    while (token != NULL)
    {

        nChildren++;
        token = strtok(NULL, " ");
    }
    return nChildren;
}
void populateTotalDescendants()
{
    long long totalProcessSoFar = 0;
    for (auto it : processStartTime)
    {
        totalProcessSoFar += getNumberOfChild(it.first);
        totalDescendants[it.first] = totalProcessSoFar;
    }
}
void sb(char *PID, bool isSuggest)
{
    populateProcessTimeMap(PID);
    populateTotalDescendants();
    cout << "Process->Parent->grandparent->....\n";
    for (auto it : processStartTime)
        cout << it.first << "->";
    cout << endl;
    char *culpritProcessId = "-1", *endptr;
    double culpritValue = 0;
    if (isSuggest)
    {
        // calculate using heuristic 1(no of children*starttime, The  time the process started after system boot. )
        // This heuristic assumes that malware will have PID greater than 4500(The approximate number of system process, should be reconfigured for each system for better result)
        cout << endl
             << "Higher Culpritness value indicates that process is more likely to be a malware" << endl;

        cout << endl
             << "PID" << ' ' << "Name" << ' ' << "     Culpritness value" << endl;

        for (auto it : processStartTime)
        {
            if (strtol(it.first, &endptr, 10) < 4500)
                break;
            if (it.second < 5000)
                break;
            // cout << (0.6) * it.second + getNumberOfChild(it.first) << endl;

            cout << it.first << ' ' << processName[it.first] << ' ';
            cout << totalDescendants[it.first] + 0.00004 * it.second << endl;
            if (totalDescendants[it.first] + 0.00004 * it.second > culpritValue)
            {
                culpritValue = totalDescendants[it.first] + 0.00004 * it.second;
                culpritProcessId = strdup(it.first);
            }
        }
        cout << "Probable culprit's PID according to heuristic 1 is: ";
        cout << culpritProcessId << endl;

        // Calculate using heuristic 2(The culprit process will have it's name different from the parent process)
        for (int i = 0; i < processStartTime.size() - 1; i++)
        {
            if (strcmp(processName[processStartTime[i].first], processName[processStartTime[i + 1].first]))
            {
                culpritProcessId = strdup(processStartTime[i].first);
                break;
            }
        }
        cout << "Probable culprit's PID according to heuristic 2 is: ";

        cout << culpritProcessId;
    }
}
int main()
{

    printf("%d\n", getNumberOfChild("104121"));
    // populateProcessTimeMap("57529");
    sb("100579", 1);
    

    return 0;
}