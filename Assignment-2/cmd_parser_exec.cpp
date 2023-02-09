#include<iostream>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<sys/wait.h>

using namespace std;

struct command{
    char **cmdarr;
    bool input_red;
    char inputfile[100];
    bool output_red;
    char outputfile[100];
    bool amp;
};

void remove_spaces(char *buf)
{
    char temp[200];
    int index = 0;
    int flag = 0;

    for (int i = 0; buf[i] != '\0'; i++)
    {   
        if(buf[i] == '\n') continue;
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

void collect_file(int x, char *buf, char *name){
    // searching starts from index i in this function
    int index = 0;
    for (int i = x; buf[i] != '\0'; i++)
    {
        if(i == x && buf[i] == ' ') continue;  // ignore the space if there

        if(buf[i] == '"'){
            // take the file name till encounter "
            name[index++] = buf[i];
            while(buf[i] != '"'){
                name[index++] = buf[i++];
            }
            name[index++] = '"';
            name[index++] = '\0';

            break;
        }
        else{
            // take the file name till encounter space or end of line
            // what abput & in the last ???

            while(buf[i] != ' ' && buf[i] != '\0'){
                name[index++] = buf[i++];
            }
            name[index++] = '\0';

            break;
        }
    }

    // printf("%s\n", name);
}

char** make_arr(char *cmd){

    int index = 0;
    char temp[100];
    char **cmdarr;
    cmdarr = (char **)malloc(sizeof(char *));
    cmdarr[index] = (char *)malloc(100*sizeof(char));

    int cnt = 0;
    int flag = 0;
    for (int i = 0; cmd[i] != '\0'; i++)
    {   
        // remove the starting spaces
        if(flag == 0 && cmd[i] == ' ') continue;
        if(cmd[i] == '\\'){
            temp[cnt++] = cmd[i++];
            temp[cnt++] = cmd[i];
            continue;
        }
        
        flag = 1;
        cnt = 0;
        while(cmd[i] != ' ' && cmd[i] != '\0'){
            temp[cnt++] = cmd[i];
            i++;
        }
        temp[cnt++] = '\0';
        // printf("Temp is %s\n", temp);

        // copy temp into the cmdarr
        strcpy(cmdarr[index++], temp);

        // realloc cmdarr
        cmdarr = (char **)realloc(cmdarr, (index+1)*sizeof(char *));
        cmdarr[index] = (char *)malloc(100*sizeof(char));

        if(cmd[i] == '\0') break;
    }

    cmdarr[index] = NULL;
    return cmdarr;
}



struct command *command_parser(char *buf){
    // makes the command structure and returns it
    struct command *ptr = (struct command *)malloc(sizeof(struct command));

    ptr->input_red = false;
    ptr->output_red = false;

    int infile = 0;
    int outfile = 0;
    int index = 0;
    int flag = 1;
    bool amp = false;
    char temp[200];  // stores the command before I/O redirections
    char filename[200];

    for (int i = 0; buf[i] != '\0'; i++)
    {
        if(buf[i] == '>'){
            // contains output file redirection
            flag = 0;
            ptr->output_red = true;
            collect_file(i+1, buf, ptr->outputfile);
        }

        else if(buf[i] == '<'){
            flag = 0;
            ptr->input_red = true;
            collect_file(i+1, buf, ptr->inputfile);
        }

        else if(buf[i] == '&'){
            flag = 0;
            amp = true;
        }

        else{
            // space at the last of temp, any problem ??
            if(flag) temp[index++] = buf[i];
        }
    }

    temp[index++] = '\0';
    char **cmdarr = make_arr(temp);
    ptr->cmdarr = cmdarr;

    return ptr;
}

void execute_command(struct command *cmd){
    // taking input output redirections for the command
    

    // executing the command
    execvp(cmd->cmdarr[0], cmd->cmdarr);

}

int main(){
    char cmd[200];
    printf("Enter command : ");
    // gets(cmd);
    fgets(cmd, 200, stdin);

    remove_spaces(cmd);
    printf("Parsed command : %s\n",cmd);

    struct command *temp = command_parser(cmd);

    cout<<temp->cmdarr[0]<<" "<<temp->outputfile<<" "<<temp->inputfile<<endl;
    execute_command(temp);
}