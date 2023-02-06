#include<bits/stdc++.h>
using namespace std;
#include<ncurses.h>
signed main(){
    initscr();
    noecho();
    char ch;
    while((ch=getch())){
        mvprintw(1,0,"KEY NAME: %s -%d\n",keyname(ch),ch); 
    }
    endwin();
   
    return 0;
    
}