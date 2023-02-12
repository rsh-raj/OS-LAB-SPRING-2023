// change pwd MAX PATH size
// Integrate cd's cases like ~ in parser
// Make sure that processes running in background and foreground are killed when exiting from shell

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <vector>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
// 'sudo apt-get -y install libreadline-dev' to install the below library
#include <readline/readline.h>
#include <glob.h>
#include<map>

#define HIST_FILE_NAME ".yars_history"

using namespace std;

int isCommandGettingExecuted = 1;
int history_len = 0;
int flag = 1;
char **history;
int history_max_len = 1000;
int hist_idx=0;
vector<int> backgroundProcesses;
void sb(char *PID, bool isSuggest);

struct command
{
    char **cmdarr;
    bool input_red;
    char inputfile[100];
    bool output_red;
    char outputfile[100];
    bool amp;
};
void output_redirection(char *file_name)
{
    close(1);
    int newFd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC);
    if (newFd < 0)
    {
        perror("file open:");
        exit(EXIT_FAILURE);
    }
    chmod(file_name, S_IRUSR | S_IWUSR);
}
void input_redirection(char *file_name)
{
    close(0);
    int newFd = open(file_name, O_RDONLY);
    if (newFd < 0)
    {
        perror("file open:");
        exit(EXIT_FAILURE);
    }
}
void collect_file(int x, char *buf, char *name)
{
    // searching starts from index x in this function
    // need modification
    int index = 0;
    int flag = 0;
    for (int i = x; buf[i] != '\0'; i++)
    {
        if (i == x && buf[i] == ' ')
            continue; // ignore the space if there

        if(buf[i] == '"' || buf[i] == '\''){
            // quoted input, ignore backslash and "
            i++;
            while(!((buf[i] == '"' || buf[i] == '\'') && buf[i-1] != '\\')){
                if(buf[i] == '\\' && (buf[i+1] == '"' || buf[i+1] == '\\')){
                    i++;
                    name[index++] = buf[i++];
                    continue;
                }

                name[index++] = buf[i++];
            }

        }else{
            // unquoted input, only ignore back slash
            while (!(buf[i] == ' ' && buf[i-1] != '\\'))
            {   
                if(buf[i] == '\0') break;
                if(buf[i] == '\\'){
                    i++;
                    // skipping the back slash
                    name[index++] = buf[i++];
                    continue;
                }
                name[index++] = buf[i++];
            }
        }
    }

    name[index++] = '\0';
    // printf("File collected : %s\n", name);
}

void remove_spaces(char *buf)
{
    // if encounter " ", then do not compress the space between the double quotes
    // need modification
    char temp[200];
    int index = 0;
    int flag = 0;
    int num_arg = 0;

    for (int i = 0; buf[i] != '\0'; i++)
    {
        if (buf[i] == '\n') continue;

        if(buf[i] == '\\'){
            temp[index++] = '\\';
            i++;
            temp[index++] = buf[i];
            continue;
        }

        // this handles the " " in the arguments
        if((i == 0 && (buf[i] == '"' || buf[i] == '\'')) || ((buf[i] == '"' || buf[i] == '\'') && buf[i-1] != '\\')){
            // put the string in temp with the quotes
            temp[index++] = buf[i];
            i++;
            // Also handle the appreance of \within the string
            while(!((buf[i] == '"' || buf[i] == '\'') && buf[i-1] != '\\')){
                // take the entie literal as it is
                temp[index++] = buf[i++];
            }
            temp[index++] = buf[i];

            continue;
        }

        if (buf[i] != ' ')
        {
            temp[index++] = buf[i];
            flag = 0;
        }
        else if (buf[i] == ' ' && flag == 0)
        {
            temp[index++] = ' ';
            num_arg++;
            flag = 1;
        }
    }

    temp[index++] = '\0';

    // copying in buf after removing the space before the argument
    strcpy(buf, temp + (temp[0] == ' ' ? 1 : 0));
}

char **make_arr(char *cmd)
{
    int index = 0;
    char temp[100];
    char **cmdarr;
    cmdarr = (char **)malloc(sizeof(char *));
    cmdarr[index] = (char *)malloc(100 * sizeof(char));

    int cnt = 0;
    int flag = 0;
    int mode = 0;  // for indicating ' or "
    for (int i = 0; cmd[i] != '\0'; i++)
    {
        // remove the starting spaces
        if (flag == 0 && cmd[i] == ' ')  continue;
        flag = 1;

        cnt = 0;
        // encountered a ' or "
        if((cmd[i] == '"' || cmd[i] == '\'') && cmd[i-1] != '\\'){
            mode = 1;
            i++;
            while(!((cmd[i] == '"' || cmd[i] == '\'') && cmd[i-1] != '\\')){
                if(cmd[i] == '\\'  && (cmd[i+1] == '"' || cmd[i+1] == '\\')){
                    i++;
                    temp[cnt++] = cmd[i++];
                    continue;
                }

                temp[cnt++] = cmd[i++];
            }
            i++;  // check this !!
        }


        // index for populating the array
        while (!(cmd[i] == ' ' && cmd[i-1] != '\\'))
        {   
            if(cmd[i] == '\0') break;
            if(cmd[i] == '\\'){
                i++;
                // skipping the back slash
                temp[cnt++] = cmd[i++];
                continue;
            }
            temp[cnt++] = cmd[i++];
        }

        temp[cnt++] = '\0';
        // printf("Temp is %s\n", temp);

        // copy temp into the cmdarr
        strcpy(cmdarr[index++], temp);

        // realloc cmdarr
        cmdarr = (char **)realloc(cmdarr, (index + 1) * sizeof(char *));
        cmdarr[index] = (char *)malloc(100 * sizeof(char));

        if (cmd[i] == '\0')  break;
    }

    cmdarr[index] = NULL;
    return cmdarr;
}

struct command *command_parser(char *buf)
{
    // makes the command structure and returns it
    struct command *ptr = (struct command *)malloc(sizeof(struct command));

    ptr->input_red = false;
    ptr->output_red = false;

    int infile = 0;
    int outfile = 0;
    int index = 0;
    int flag = 1;
    bool amp = false;
    char temp[200]; // stores the command before I/O redirections
    char filename[200];

    for (int i = 0; buf[i] != '\0'; i++)
    {
        if (buf[i] == '>')
        {
            // contains output file redirection
            flag = 0;
            ptr->output_red = true;
            collect_file(i + 1, buf, ptr->outputfile);
        }

        else if (buf[i] == '<')
        {
            flag = 0;
            ptr->input_red = true;
            collect_file(i + 1, buf, ptr->inputfile);
        }

        else if (buf[i] == '&')
        {
            flag = 0;
            amp = true;
        }

        else
        {
            // space at the last of temp, any problem ??
            if (flag)
                temp[index++] = buf[i];
        }
    }

    temp[index++] = '\0';
    char **cmdarr = make_arr(temp);
    ptr->cmdarr = cmdarr;
    ptr->amp = amp;

    return ptr;
}

void execute_command(struct command *cmd)
{
    // taking input output redirections for the command
    // executing the command
    if (cmd->input_red)
        input_redirection(cmd->inputfile);
    if (cmd->output_red)
        output_redirection(cmd->outputfile);
    cout << cmd->cmdarr[0];
    if (execvp(cmd->cmdarr[0], cmd->cmdarr) < 0)
    {
        printf(" command '%s' not found\n", cmd->cmdarr[0]);
    }
}

void file_lock_detection(char *file_path){
    char out[200];
    int fd[2];
    pipe(fd);

    if(fork() == 0){
        // redirecting the stdout of the process 
        close(1);
        dup(fd[1]);

        int x = execlp("fuser", "fuser", file_path, (char *)NULL);
        // cout<<x<<endl;

        exit(0);
    }
    wait(0);

    // making read non blocking
    int flags = fcntl(fd[0], F_GETFL, 0);
    fcntl(fd[0], F_SETFL, flags | O_NONBLOCK);

    // cout<<"wow"<<endl;
    // parent processing

    // checking if the file has any locks or not
    int x = read(fd[0], out, 500);
    if(x == -1){
        cout<<"File has no locks"<<endl;
        if(!remove(file_path)){

            cout<<"File successfully deleted !"<<endl;
        }else cout<<"Failed to delete file"<<endl;

        return ;
    }

    // parsing the pids from the 
    vector<int> locking_pids;
    char temp[20];
    int index = 0;

    for (int i = 0; out[i] != '\0'; i++)
    {
        if(out[i] == ' ') continue;
        else{
            while(out[i] != ' '){
                if(out[i] == '\0') break;
                temp[index++] = out[i++];
            }
        }
        temp[index++] = '\0';
        locking_pids.push_back(atoi(temp));
        index = 0;
    }

    // for(auto it : locking_pids){
    //     cout<<it<<" ";
    // }
    // cout<<endl;
    cout<<"The following processes are locking/read/write the file :"<<endl;
    for(auto it : locking_pids){
        char str[10];
        sprintf(str, "%d", it);
        char buf[50];
        int fd[2];
        pipe(fd);

        if(fork() == 0){
            close(1);
            dup(fd[1]);
            execlp("ps", "ps", "-p", str, "-o", "comm=", NULL);
            exit(0);
        }

        wait(0);
        read(fd[0], buf, 50);
        buf[strlen(buf)-1] = '\0';
        
        cout<<"PID : "<<it<<"   ";
        cout<<"Name : "<<buf<<endl;
    }

    while(1){
        cout<<"Do you want to delete them (Y/n) ? ";
        char ans;
        cin>>ans;
        if(ans == 'y' || ans == 'Y'){
            // kill process with the given pids
            for(auto it : locking_pids){
                kill(it, SIGKILL);
            }
            if(!remove(file_path)){

                cout<<"File successfully deleted !"<<endl;
            }else cout<<"Failed to delete file"<<endl;
            break;
        }
        else if(ans == 'n' || ans == 'N'){
            cout<<"Not deleted"<<endl;
            break;
        }
        else cout<<"Incorrect answer"<<endl;
    }
}

bool haveWildCard(char *command)
{

    if ((strchr(command, '*') != NULL) || (strchr(command, '?') != NULL))
    {
        // cout << "wildcard found" << endl;
        return true;
    }
    else
    {
        // cout << "wildcard not found" << endl;
        return false;
    }
}

vector<string> expandWildcard(const string &pattern)
{
    
    glob_t glob_result;
    memset(&glob_result, 0, sizeof(glob_result));

    int return_value = glob(pattern.c_str(), GLOB_TILDE, NULL, &glob_result);
    if (return_value != 0)
    {
        cerr << "Error: failed to match the pattern: " << pattern << endl;
        globfree(&glob_result);
        exit(1);
    }

    vector<string> matched_files;
    for (size_t i = 0; i < glob_result.gl_pathc; ++i)
    {
        matched_files.push_back(string(glob_result.gl_pathv[i]));
    }

    globfree(&glob_result);

    return matched_files;
}

void pwd(char *cwd)  
{
    if (getcwd(cwd, sizeof(cwd)) == nullptr)
    {
        cerr << "pwd: " << strerror(errno) << endl;
    }
}

// change directory function to change the directory to the path given
void cd(const char *path)
{
    char PCuser[30];
    getlogin_r(PCuser, sizeof(PCuser)); // getlogin_r is used to get the username of the current user
    string temp;
    if (strcmp(path, "~") == 0)
    {
        temp = "/home/";
        temp += PCuser;
        if(chdir(temp.c_str())==-1)
        {
            cerr << "cd: " << strerror(errno) << endl;
        }
    }
    else
    {
        if (chdir(path) == -1)
        {
            cerr << "cd: " << strerror(errno) << endl;
        }
    }
}

void pipe_execution(char *cmd, int numcommand)
{
    int stdin_fd = dup(0);

    char temp[100];
    int cnt = 0;
    int command = 0;
    int mode = 0;
    for (int i = 0; cmd[i] != '\0'; i++)
    {
        cnt = 0;
        while (cmd[i] != '\0')
        {   
            if((cmd[i] == '"' || cmd[i] == '\'') && cmd[i-1] != '\\'){
                mode = 1 - mode;
            }

            if(cmd[i] == '|' && mode == 1){
                temp[cnt++] = cmd[i++];
                continue;
            }else if(cmd[i] == '|' && mode == 0) break;

            temp[cnt++] = cmd[i++];
        }

        // segregating the first command
        temp[cnt++] = '\0';
        command++;

        struct command *ptr = command_parser(temp);
        // temp contains commands that start with whitespace
        // char **cmdarr = make_arr(temp);
        // command_arr.push_back(cmdarr);

        int fd[2];
        pipe(fd);

        // replace by a function call
        if (!strcmp(ptr->cmdarr[0], "cd"))
        {
            if(ptr->cmdarr[1]){
                cd(ptr->cmdarr[1]);
            }else{
                cd("~");
            }
           continue;
        }

        // command for file lock detection
        if(!strcmp(ptr->cmdarr[0], "delep")){
            file_lock_detection(ptr->cmdarr[1]);
            continue;
        }

        else if(!strcmp(ptr->cmdarr[0],"rm") || !strcmp(ptr->cmdarr[0],"gedit") || !strcmp(ptr->cmdarr[0],"cp") || !strcmp(ptr->cmdarr[0],"mv")){
            if (ptr->cmdarr[1] != NULL)
            {
                
                if (haveWildCard(ptr->cmdarr[1]))
                {
                    
                    vector<string> files = expandWildcard(ptr->cmdarr[1]);
                    cout << files.size() << endl;
                    int j = 1;
                    
                    for (int i = 0; i < files.size(); i++)
                    {
                        // cout << files[i] << endl;
                        ptr->cmdarr[j] = (char *)files[i].c_str();
                        j++;
                    }
                    
                    ptr->cmdarr[j] = NULL;
                    if(fork() == 0){

                        execute_command(ptr);
                        exit(0);
                    }
                    free(ptr);
                    continue;
                }
            }
        }
        else if(!strcmp(ptr->cmdarr[0],"ls")){

            if (ptr->cmdarr[1] != NULL)
            {
                
                if (haveWildCard(ptr->cmdarr[1]))
                {
                    
                    vector<string> files = expandWildcard(ptr->cmdarr[1]);
                
                    for (int i = 0; i < files.size(); i++)
                    {
                        cout << files[i] << endl;
                        
                    }
                    continue;
                }
            }
            
        }
        if (!strcmp(ptr->cmdarr[0], "sb"))
        {
            if (!strcmp(ptr->cmdarr[1], "-suggest"))
                sb(ptr->cmdarr[2], 1);
            else
                sb(ptr->cmdarr[1], 0);

            continue;
        }

        int x,status;
        x = fork();
        if (x == 0)
        {
            // executing the command, all file descriptors copied to the child process
            // redirecting the output of the commmand
            if (command != numcommand) dup2(fd[1], 1);

            execute_command(ptr);
            exit(0);
        }

        if (command == numcommand && !ptr->amp){
            while (waitpid(x,&status,WNOHANG) != x){
                if(flag == 0){
                    flag = 1;
                    break;
                }
            }
        }

        // redirecting the stdin of the parent process to the other end of the pipe
        dup2(fd[0], 0);
        close(fd[1]);

        if (command == numcommand)
        {
            // restore the stdin of the parent
            dup2(stdin_fd, 0);
            break;
        }
    }
}

// handle corner cases in count_pipes
int count_pipes(char *cmd)
{
    int index = 0;
    int mode = 0;
    for (int i = 0; cmd[i] != '\0'; i++)
    {
        if((cmd[i] == '"' || cmd[i] == '\'') && cmd[i-1] != '\\'){
                mode = 1 - mode;
            }

        if(cmd[i] == '|' && mode == 0) index++;
    }

    return index + 1;
}

int HistorySave(const char *filename) {
    fstream filestream(filename,fstream::in|fstream::out|fstream::trunc);
    if(!filestream){
        cerr<<"Cannot open the output file";
        exit(EXIT_FAILURE);
    }  
    // ofstream fp(filename);
    string s;
    int j;
    // umask(old_umask);
    // if (fp == NULL) return -1;
    // chmod(filename,S_IRUSR|S_IWUSR);
    for (j = 0; j < history_len; j++)
    {
        string s = history[j];
        filestream<<s<<endl;
    }
    filestream.close();
    return 0;
}

int HistoryAdd(const char *line) {
    char *linecopy;

    if (history_max_len == 0) return 0;

    /* Initialization on first call. */
    if (history == NULL) {
        history = (char**)malloc(sizeof(char*)*history_max_len);
        if (history == NULL) return 0;
        memset(history,0,(sizeof(char*)*history_max_len));
    }

    /* Add an heap allocated copy of the line in the history.
     * If we reached the max length, remove the older line. */
    linecopy = strdup(line);
    if (!linecopy) return 0;
    if (history_len == history_max_len) {
        free(history[0]);
        memmove(history,history+1,sizeof(char*)*(history_max_len-1));
        history_len--;
    }
    history[history_len] = linecopy;
    history_len++;
    return 1;
}
void shell()
{
    // printf("Enter command : ");
    // gets(cmd);
    // fgets(cmd, 200, stdin);
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == nullptr)
    {
        cerr << "pwd: " << strerror(errno) << endl;
    }
    strcat(cwd,"> ");

    isCommandGettingExecuted = 0;
    char *cmd = readline(cwd);
    hist_idx=0;
    if (!strcmp(cmd, "exit") || !strcmp(cmd, "exit;"))
    {
        printf("BYE!\n");
        HistorySave(HIST_FILE_NAME);
        exit(EXIT_SUCCESS);
    }
    HistoryAdd(cmd);
    isCommandGettingExecuted = 1;
    remove_spaces(cmd);
    // printf("Parsed command : %s\n", cmd);
    pipe_execution(cmd, count_pipes(cmd));
    free(cmd);
}

void sig_handler(int signum)
{
    // this function will fork a new process that will kill all the children
    // pkill -P (parent process PID)
    pid_t pid = getpid();
    char temp[50], num[20];
    strcpy(temp, "pkill -P ");
    sprintf(num, "%d", pid);
    strcat(temp, num);

    // printf("%s\n", temp);
    if (fork() == 0)
    {
        char **cmdarr = make_arr(temp);
        execvp(cmdarr[0], cmdarr);
        exit(0);
    }
    if (!isCommandGettingExecuted)
        printf("\nEnter command : ");
    else
        cout << "\n";
    fflush(NULL);
}

int beginning_of_line(int count, int key)
{
    rl_point = 0;
    return 0;
}

int end_of_line(int count, int key)
{
    rl_point = rl_end;
    return 0;
}

int history_next(int count, int key)
{
    if(history_len>0)
    {
        hist_idx--;

        if(hist_idx < 0)
        {
            hist_idx = 0;
            return 1;
        }
        else if(hist_idx >= history_len)
        {
            hist_idx = history_len-1;
            return 1;
        }

        char *comm = history[history_len-1-hist_idx];
        rl_replace_line(comm, 0);
        rl_point = rl_end;
        if(hist_idx == 0)
        {
            free(history[history_len-1]);
            history_len--;
        }
    }
    return 1;
}
int history_prev(int count, int key)
{
    if(history_len>0)
    {
        if(hist_idx == 0)
        {
            if(history_len == history_max_len)
            {
                free(history[0]);
                memmove(history,history+1,sizeof(char*)*(history_max_len-1));
                history_len--;
            }
            history[history_len]=strdup(rl_line_buffer);
            // rl_copy_text(0, rl_end)
            history_len++;
        }
        hist_idx++;

        if(hist_idx < 0)
        {
            hist_idx = 0;
            return 1;
        }
        else if(hist_idx >= history_len)
        {
            hist_idx = history_len-1;
            return 1;
        }

        char *comm = history[history_len-1-hist_idx];
        rl_replace_line(comm, 0);
        rl_point = rl_end;
    }
    return 1;
}

void history_disktomem(char *filename)
{
    ifstream fp;
    fp.open(filename);

    string s;
    while (getline(fp, s)) {
        char *buf = (char *)s.c_str();
        char *p;
        p = strchr(buf,'\r');
        if (!p) p = strchr(buf,'\n');
        if (p) *p = '\0';
        HistoryAdd(buf);
    }
    fp.close();
}

void sig_background(int signum){
    flag = 0;
}
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
int populateProcessTimeMap(char *PID)
{
    char *ppid = strdup(PID), *pend;
    while (strtol(ppid, &pend, 10) != 0)
    {
        ppid = setProcessTime(ppid);
        if (ppid == "-1")
            return 0;
    }
    return 1;
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
    processName.clear();
    processStartTime.clear();
    totalDescendants.clear();
    if (!PID)
    {
        printf("usage <PID> flags..\n");
        return;
    }
    if (populateProcessTimeMap(PID) == 0)
    {
        return;
    }
    populateTotalDescendants();
    cout << "Process->Parent->grandparent->....\n";
    for (auto it : processStartTime)
        cout << it.first << "->";
    cout << endl;
    char *culpritProcessId = "-1", *endptr;
    double culpritValue = -1;
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

        cout << culpritProcessId << endl;
    }
}

int main()
{
    rl_add_defun("beginning-of-line", beginning_of_line, 1);
    rl_add_defun("end-of-line", end_of_line, 5);

    rl_bind_keyseq("\033\[A", history_prev);
    rl_bind_keyseq("\033\[B", history_next);
    history_disktomem(HIST_FILE_NAME);
    signal(SIGINT, sig_handler);
    signal(SIGTSTP, sig_background);
    while (1)
    {
        shell();
    }
    free(history);

    return 0;
}

