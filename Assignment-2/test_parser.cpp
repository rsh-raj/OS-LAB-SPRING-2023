#include<iostream>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<sys/wait.h>

using namespace std;

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

// will get the input with the pipe removed
// if " ", then take the input in single string
// delemit using a space not preceded by a back slash
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
        printf("Temp is %s\n", temp);

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

int main(){
    char buf[200], name[200];
    cout<<"Enter file path : ";
    fgets(buf, 200, stdin);
    // cout<<"Before : "<<buf<<endl;
    remove_spaces(buf);
    cout<<"After : "<<buf<<endl;
    // make_arr(buf);
    // collect_file(4,buf,name);
    // if(fork() == 0){
    //     char **cmdarr = make_arr(buf);
    //     execvp(cmdarr[0], cmdarr);
    //     free(cmdarr);
    // }
    cout<<count_pipes(buf)<<endl;

    wait(0);
    return 0;
}