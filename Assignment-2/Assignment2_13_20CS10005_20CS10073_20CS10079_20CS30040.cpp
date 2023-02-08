// minor bug in purser: remove "" quotes and replace space with \space for storing the string enclosed within double quotes

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <readline/readline.h>

// 'sudo apt-get -y install libreadline-dev' to install the below library

using namespace std;
int isCommandGettingExecuted = 1;
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

        // keep collecting until encounter a space which is not preceded by a back slash
        // i-1 will always exits, hence no worry
        while(!(buf[i] == ' ' && buf[i-1] != '\\')){
            if(buf[i] == '\0'){
                flag = 1;
                break;
            }
            name[index++] = buf[i++];
        }

        if(flag) break;
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

    for (int i = 0; buf[i] != '\0'; i++)
    {
        if (buf[i] == '\n') continue;

        if(buf[i] == '\\'){
            temp[index++] = '\\';
            i++;
            temp[index++] = buf[i];
            continue;
        }

        if((i == 0 && (buf[i] == '"' || buf[i] == '\'')) || ((buf[i] == '"' || buf[i] == '\'') && buf[i-1] != '\\')){
            // put the string in temp without the quotes
            i++;
            // Also handle the appreance of \within the string
            while(!((buf[i] == '"' || buf[i] == '\'') && buf[i-1] != '\\')){
                if(buf[i] == ' '){
                    temp[index++] = '\\';
                }
                temp[index++] = buf[i++];
            }

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
            if (flag) temp[index++] = buf[i];
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

    if (!strcmp(cmd->cmdarr[0], "pwd"))
    {
        char cwd[1024];

        if (getcwd(cwd, sizeof(cwd)) == nullptr)
        {
            cerr << "pwd: " << strerror(errno) << endl;
        }
    }

    if (execvp(cmd->cmdarr[0], cmd->cmdarr) < 0)
    {
        printf("command '%s' not found\n", cmd->cmdarr[0]);
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
            while (wait(NULL) > 0);

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

void shell()
{
    // printf("Enter command : ");
    // gets(cmd);
    // fgets(cmd, 200, stdin);

    isCommandGettingExecuted = 0;
    char *cmd = readline("Enter Command : ");
    if (!strcmp(cmd, "exit"))
    {
        printf("BYE!\n");
        exit(EXIT_SUCCESS);
    }
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

int main()
{
    // rl_add_defun("beginning-of-line", beginning_of_line, 49);
    // rl_add_defun("end-of-line", end_of_line, 57);

    signal(SIGINT, sig_handler);
    while (1)
    {
        shell();
    }

    return 0;
}
