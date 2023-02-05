//Main shell file
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

using namespace std;

void parser(char *buf)
{
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
    for (int i = 0; cmd[i] != '\0'; i++)
    {
        cnt = 0;
        while (cmd[i] != ' ' && cmd[i] != '\0')
        {
            temp[cnt++] = cmd[i];
            i++;
        }
        temp[cnt++] = '\0';
        printf("Temp is %s\n", temp);

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
void execute_command(char **cmdarr){
    execvp(cmdarr[0], cmdarr);
}

int main()
{
    char cmd[200];

    while (1)
    {
        printf("Enter command : ");
        // gets(cmd);
        fgets(cmd, 200, stdin);

        parser(cmd);
        printf("Parsed command : %s\n", cmd);

        // making an array of string pointers terminated by NULL pointer
        if (fork() == 0)
        {
            char **cmdarr = make_arr(cmd);
            printf("%s\n", cmdarr[0]);
            execute_command(cmdarr);
            
        }

        else
            wait(0);
    }

    return 0;
}