#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <bits/stdc++.h>

using namespace std;
int isCommandGettingExecuted=1;
struct command
{
    char **cmdarr;
    bool input_red;
    char inputfile[100];
    bool output_red;
    char outputfile[100];
    bool amp;
};

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

void execute_command(struct command *cmd)
{
    // taking input output redirections for the command

    // executing the command
    execvp(cmd->cmdarr[0], cmd->cmdarr);
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
void shell()
{

    char cmd[200];

    printf("Enter command : ");
    // gets(cmd);
    isCommandGettingExecuted=0;

    fgets(cmd, 200, stdin);
    isCommandGettingExecuted=1;
    remove_spaces(cmd);
    printf("Parsed command : %s\n", cmd);
    pipe_execution(cmd, count_pipes(cmd));

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
    if(!isCommandGettingExecuted) printf("\nEnter command : ");
    else cout<<"\n";
    fflush(NULL);
}

int main()
{
    signal(SIGINT, sig_handler);
    while (1)
    {

        shell();
    }

    return 0;
}
