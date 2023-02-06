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

#include <iostream>
#include <readline/readline.h>
#include <readline/history.h>
using namespace std;
int main() {
  char* input = readline("Enter a command: ");
  cout << "You entered: " << input << endl;
  free(input);
  return 0;
}

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

