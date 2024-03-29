// minor bug in parser: remove "" quotes and replace space with \space for storing the string enclosed within double quotes
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
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
// 'sudo apt-get -y install libreadline-dev' to install the below library
#include <readline/readline.h>
#include <glob.h>       // for wildcard expansion 
#include <vector>       

#define HIST_FILE_NAME ".yars_history"

using namespace std;

int isCommandGettingExecuted = 1;
int history_len = 0;
char **history;
int history_max_len = 1000;
int hist_idx = 0;
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
    // searching starts from index i in this function
    int index = 0;
    for (int i = x; buf[i] != '\0'; i++)
    {
        if (i == x && buf[i] == ' ')
            continue; // ignore the space if there

        if (buf[i] == '"')
        {
            // take the file name till encounter "
            name[index++] = buf[i];
            while (buf[i] != '"')
            {
                name[index++] = buf[i++];
            }
            name[index++] = '"';
            name[index++] = '\0';

            break;
        }
        else
        {
            // take the file name till encounter space or end of line
            // what abput & in the last ???

            while (buf[i] != ' ' && buf[i] != '\0')
            {
                name[index++] = buf[i++];
            }
            name[index++] = '\0';

            break;
        }
    }

    // printf("%s\n", name);
}
/*
The function expandWildcard takes a single parameter, pattern, which is a string that may contain wildcard characters like * and ?.
The function uses the glob function to match the given pattern against the names of files in the file system.
The glob function returns the results of the pattern match in a glob_t structure, which is a dynamically allocated array of strings.
The glob function takes four parameters: the pattern string, a flag indicating how to handle tildes, a pointer to a function to be called on each match, and a pointer to the glob_t structure.
The return value of the glob function is checked against zero. If the value is non-zero, the function terminates with an error message indicating that the pattern could not be matched.
If the glob function succeeds, the function loops through the glob_result.gl_pathc elements of the glob_result structure and adds each matched file name to a vector<string> named matched_files. Finally, the function frees the memory used by the glob_result structure using globfree and returns the matched_files vector.
*/
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

void remove_spaces(char *buf)
{

    // if encounter " ", then do not compress the space between the double quotes
    char temp[200];
    int index = 0;
    int flag = 0;

    for (int i = 0; buf[i] != '\0'; i++)
    {
        if (buf[i] == '\n')
            continue;
        if (buf[i] != ' ')
        {
            temp[index++] = buf[i];
            flag = 0;
        }
        else if (buf[i] == ' ' && flag == 0)
        {
            temp[index++] = ' ';
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
    for (int i = 0; cmd[i] != '\0'; i++)
    {
        // remove the starting spaces
        if (flag == 0 && cmd[i] == ' ')
            continue;
        flag = 1;
        cnt = 0;
        while (cmd[i] != ' ' && cmd[i] != '\0')
        {
            temp[cnt++] = cmd[i];
            i++;
        }
        temp[cnt++] = '\0';
        // printf("Temp is %s\n", temp);

        // copy temp into the cmdarr
        strcpy(cmdarr[index++], temp);

        // realloc cmdarr
        cmdarr = (char **)realloc(cmdarr, (index + 1) * sizeof(char *));
        cmdarr[index] = (char *)malloc(100 * sizeof(char));

        if (cmd[i] == '\0')
            break;
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

    return ptr;
}
// print working directory function
void pwd()  
{
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == nullptr)
    {
        cerr << "pwd: " << strerror(errno) << endl;
    }
    // else
    // {
    //     cout << cwd << endl;
    // }
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

void pipe_execution(char *cmd, int numcommand)
{
    int stdin_fd = dup(0);

    char temp[100];
    int cnt = 0;
    int command = 0;

    for (int i = 0; cmd[i] != '\0'; i++)
    {
        cnt = 0;
        while (cmd[i] != '|' && cmd[i] != '\0')
        {
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
        if (!strcmp(ptr->cmdarr[0], "cd"))
        {
            
            /***************** Issue 1 *****************/
            // not able to check if ptr->cmdarr[1] is null or not
            // if it is null then it should change the directory to home
            // but it is not working
            if (ptr->cmdarr[1] == NULL)
            {
                cd("~");
            }
            else
            {
                cd(ptr->cmdarr[1]);
            }
            continue;
        }

        /****************** Issue 2 *******************/
        // not able to implement the wildcards with commands like ls, cp, mv, rm, etc..
        // I have written this below code for a dummy lw command for checking if the wildcard expansion is working or not
        // it is working fine
        // The function expandWildcard is written above now you have to itegrate it with the other commands
        if (!strcmp(ptr->cmdarr[0], "lw"))
        {
            vector<string> files = expandWildcard(ptr->cmdarr[1]);
            for (int i = 0; i < files.size(); i++)
            {
                cout << files[i] << endl;
            }
            continue;
        }

        if (fork() == 0)
        {
            // executing the command, all file descriptors copied to the child process
            // redirecting the output of the commmand
            if (command != numcommand)
            {
                dup2(fd[1], 1);
            }

            execute_command(ptr);
            // execvp(cmdarr[0], cmdarr);
            exit(0);
        }

        if (command == numcommand && !ptr->amp)
            while (wait(NULL) > 0)
                ;

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

int count_pipes(char *cmd)
{
    int index = 0;
    for (int i = 0; cmd[i] != '\0'; i++)
    {
        if (cmd[i] == '|')
            index++;
    }

    return index + 1;
}

int HistorySave(const char *filename)
{
    fstream filestream(filename, fstream::out | fstream::trunc);
    if (!filestream)
    {
        cerr << "Cannot open the output file";
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
        filestream << s << endl;
    }
    filestream.close();
    return 0;
}
int HistoryAdd(const char *line)
{
    char *linecopy;

    if (history_max_len == 0)
        return 0;

    /* Initialization on first call. */
    if (history == NULL)
    {
        history = (char **)malloc(sizeof(char *) * history_max_len);
        if (history == NULL)
            return 0;
        memset(history, 0, (sizeof(char *) * history_max_len));
    }

    /* Add an heap allocated copy of the line in the history.
     * If we reached the max length, remove the older line. */
    linecopy = strdup(line);
    if (!linecopy)
        return 0;
    if (history_len == history_max_len)
    {
        free(history[0]);
        memmove(history, history + 1, sizeof(char *) * (history_max_len - 1));
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

    isCommandGettingExecuted = 0;
    char *cmd = readline("Enter Command : ");
    hist_idx = 0;
    if (!strcmp(cmd, "exit") || !strcmp(cmd, "exit;"))
    {
        printf("BYE!\n");
        HistorySave(HIST_FILE_NAME);
        exit(EXIT_SUCCESS);
    }
    HistoryAdd(cmd);
    isCommandGettingExecuted = 1;
    remove_spaces(cmd);
    printf("Parsed command : %s\n", cmd);
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
    if (history_len > 0)
    {
        hist_idx--;

        if (hist_idx < 0)
        {
            hist_idx = 0;
            return 1;
        }
        else if (hist_idx >= history_len)
        {
            hist_idx = history_len - 1;
            return 1;
        }

        char *comm = history[history_len - 1 - hist_idx];
        rl_replace_line(comm, 0);
        rl_point = rl_end;
        if (hist_idx == 0)
        {
            free(history[history_len - 1]);
            history_len--;
        }
    }
    return 1;
}
int history_prev(int count, int key)
{
    if (history_len > 0)
    {
        if (hist_idx == 0)
        {
            if (history_len == history_max_len)
            {
                free(history[0]);
                memmove(history, history + 1, sizeof(char *) * (history_max_len - 1));
                history_len--;
            }
            history[history_len] = strdup(rl_line_buffer);
            // rl_copy_text(0, rl_end)
            history_len++;
        }
        hist_idx++;

        if (hist_idx < 0)
        {
            hist_idx = 0;
            return 1;
        }
        else if (hist_idx >= history_len)
        {
            hist_idx = history_len - 1;
            return 1;
        }

        char *comm = history[history_len - 1 - hist_idx];
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
    while (getline(fp, s))
    {
        char *buf = (char *)s.c_str();
        char *p;
        p = strchr(buf, '\r');
        if (!p)
            p = strchr(buf, '\n');
        if (p)
            *p = '\0';
        HistoryAdd(buf);
    }
    fp.close();
}

int main()
{
    // rl_add_defun("beginning-of-line", beginning_of_line, 49);
    // rl_add_defun("end-of-line", end_of_line, 57);

    rl_bind_keyseq("\033\[A", history_prev);
    rl_bind_keyseq("\033\[B", history_next);
    history_disktomem(HIST_FILE_NAME);
    signal(SIGINT, sig_handler);
    while (1)
    {

        shell();
    }
    free(history);

    return 0;
}