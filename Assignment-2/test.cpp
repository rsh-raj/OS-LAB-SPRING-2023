// // #include <iostream>
// // #include <string>

// // using namespace std;

// // int main() {
// //   string command;
// //   int cursor = 0;

// //   while (true) {
// //     char c = getchar();
// //     if (c == '\n') {
// //       cout << command << endl;
// //       break;
// //     } else if (c == '\x01') {  // ASCII code for 'ctrl + 1'
// //       cursor = 0;
// //     } else if (c == '\x09') {  // ASCII code for 'ctrl + 9'
// //       cursor = command.length();
// //     } else if (c == '\x7F') {  // ASCII code for 'backspace'
// //       if (cursor > 0) {
// //         cursor--;
// //         command.erase(cursor, 1);
// //       }
// //     } else {
// //       command.insert(cursor, 1, c);
// //       cursor++;
// //     }
// //     cout << '\r' << command << " ";
// //     for (int i = 0; i < cursor; i++) {
// //       cout << '\b';
// //     }
// //   }

// //   return 0;
// // }

// #include <iostream>
// #include <readline/readline.h>
// #include <readline/history.h>
// using namespace std;
// int main() {
//   char* input = readline("Enter a command: ");
//   cout << "You entered: " << input << endl;
//   free(input);
//   return 0;
// }

// #include <iostream>
// #include <readline/readline.h>
// #include <readline/history.h>
// using namespace std;
// int main() {
//   char* input = readline("Enter a command: ");
//   // rl_bind_key('\x01', rl_beg_of_line);  // ASCII code for 'ctrl + 1'
//   // rl_bind_key('\x09', rl_end_of_line);  // ASCII code for 'ctrl + 9'
//   cout << "You entered: " << input << endl;
//   free(input);
//   return 0;
// }

#include <iostream>
#include <vector>
#include <glob.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

std::vector<std::string> glob(const std::string& pattern) {
    glob_t glob_result;
    memset(&glob_result, 0, sizeof(glob_result));

    int return_value = glob(pattern.c_str(), GLOB_TILDE, NULL, &glob_result);
    if (return_value != 0) {
        globfree(&glob_result);
        return std::vector<std::string>();
    }

    std::vector<std::string> files;
    for (unsigned int i = 0; i < glob_result.gl_pathc; ++i) {
        files.push_back(std::string(glob_result.gl_pathv[i]));
    }

    globfree(&glob_result);
    return files;
}

int main(int argc, char** argv) {
    while (true) {
        std::cout << "$ ";
        std::string line;
        std::getline(std::cin, line);

        std::vector<std::string> tokens;
        std::string token;
        for (unsigned int i = 0; i < line.size(); ++i) {
            if (line[i] == ' ') {
                tokens.push_back(token);
                token.clear();
            } else if (line[i] == '*' || line[i] == '?') {
                std::vector<std::string> results = glob(token + line.substr(i));
                tokens.insert(tokens.end(), results.begin(), results.end());
                token.clear();
                break;
            } else {
                token += line[i];
            }
        }
        if (!token.empty()) {
            tokens.push_back(token);
        }

        if (tokens.empty()) {
            continue;
        }

        std::vector<char*> argv;
        for (unsigned int i = 0; i < tokens.size(); ++i) {
            argv.push_back((char*)tokens[i].c_str());
        }
        argv.push_back(NULL);

        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
        } else if (pid == 0) {
            execvp(argv[0], &argv[0]);
            perror("execvp");
            return 1;
        } else {
            int status;
            waitpid(pid, &status, 0);
        }
    }

    return 0;
}
/*
Wildcards can be used with many shell commands, including:

    ls: To list the files and directories in a directory that match a specific pattern.

    cp: To copy files that match a specific pattern.

    mv: To move files that match a specific pattern.

    rm: To remove files that match a specific pattern.

    grep: To search for text patterns in files that match a specific pattern.

    find: To search for files and directories that match a specific pattern.

    sed: To perform text transformations on files that match a specific pattern.

    tar: To create or extract archives of files that match a specific pattern.
*/

// now handeling the wildcards * and ?
        // else if (buf[i] == '*' || buf[i] == '?')
        // {
        //     string line(buf);

        //     vector<string> wildCardResult = expandWildcard(temp + line.substr(i));
        //     for (int j = 0; j < wildCardResult.size(); j++)
        //     {
        //         for (int k = 0; k < wildCardResult[j].length(); k++)
        //         {
        //             temp[index++] = wildCardResult[j][k];
        //         }
        //         temp[index++] = ' ';
        //     }
        // }