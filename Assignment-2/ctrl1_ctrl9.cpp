#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <cstdlib>

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

int main(int argc, char **argv)
{
    rl_add_defun("beginning-of-line", beginning_of_line, 49);
    rl_add_defun("end-of-line", end_of_line, 57);
    while (1)
    {
        char *input = readline("> ");
        if (!input)
            break;
        printf("You entered: %s\n", input);
        free(input);
    }
    return 0;
}
