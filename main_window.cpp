#include <curses.h>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <menu.h>
#include <cstring>
#include <string>
#include <vector>
#include <stdlib.h>
#include <algorithm>
#include <cerrno>

#include "song.h"
#include "library.h"

using namespace std;

class LSTWINDOW {
  public:
    WINDOW* win;
    MENU *menu;
};
struct InvalidChar
{
    bool operator()(char c) const {
        return !isprint((unsigned)c);
    }
};

vector <WINDOW*> wlist;
LSTWINDOW* focused;
vector<char*> choices;

int read_line(char input, char*buffer);
void init_song_menu(vector<Song>& songs);

int main(int argc, char* argv[]){
  Library lib("kevin");
  lib.add_search_path("/home/kevin/Music");
  lib.scan(); 
  initscr();
  start_color();
  cbreak();
  noecho();    
  keypad(stdscr, TRUE); 
  init_pair(1, COLOR_RED, COLOR_BLACK);
  init_pair(2, COLOR_CYAN, COLOR_BLACK);
  refresh();
  init_song_menu(lib);
  post_menu(focused->menu);
  wrefresh(focused->win);
  refresh();
  int ch=0;
    
  while(ch != KEY_F(1)){
    ch = getch();
    switch(ch){
      case KEY_DOWN:
        menu_driver(focused->menu,REQ_DOWN_ITEM);
        break;
      case KEY_UP:
        menu_driver(focused->menu,REQ_UP_ITEM);
        break;
      case ((int)':'):
        char buf[80];
        int x = getcurx(stdscr);
        int y = getcury(stdscr);
        read_line(':',buf);
        deleteln();
        move(y,x);
        break;
    }
    wrefresh(focused->win);
  }
 // getch();
  endwin();  
}

int read_line(char input,char *buffer){
  string buf = "";
  buf += input;
  int pos = 1;
  mvaddch(LINES-1,pos-1,':');
  move(LINES-1,pos);
  while(buf != ""){
    int ch = getch();
    switch(ch){
      case KEY_BACKSPACE:
        pos--;
        mvdelch(LINES-1,pos);
        if(pos==0){
          return OK;
        }
        buf.erase(pos);
        break;
      case '\n':
        strcpy(buffer,buf.c_str());
        return OK;
      case KEY_LEFT:
        if(pos==0){
          beep();
          break;
        }
        pos--;
        move(LINES-1,pos);
        break;
      case KEY_RIGHT:
        if(pos==(int)buf.length()){
          beep();
          break;
        }
        pos++;
        move(LINES-1,pos);
        break;
      default: 
        buf += ((char)ch);
        pos++;
        addch(ch);  
    }
   
  }
   
  return OK;
}

void init_song_menu(vector<Song>& songs){
  int nsongs = (int) songs.size();
  ITEM** items = (ITEM **)calloc(nsongs, sizeof(ITEM *));
  ofstream file("/home/kevin/songs.txt");
  if(!file){
    printw(" ");
    printw(strerror(errno));
    printw(" ");
  }
  file << flush;
  printw("%d  ",nsongs);
  for(int i = 0; i < nsongs; ++i){
    string choice;
    stringstream out;
    out << i;
    out << " ";
    string title;
    title += songs.at(i).get_ID3().title;
    if(title.length() <= 0){
      title += songs.at(i).get_ID3v2().frames["TIT2"].data;
      if(title.length() <= 0){
        title += songs.at(i).get_path();
      }
    }
    title = title.substr(0,30);
    out << title << setw(45- (out.str().length()+title.length()));
    out << " | ";
    string artist;
    artist += songs.at(i).get_ID3().artist;
    out << artist.substr(0,30);
//    out<<"|";
//    out<<songs.at(i).get_ID3().artist;
    choice += out.str();
    choice.erase(std::remove_if(choice.begin(),choice.end(),InvalidChar()), choice.end());
    file << out.str().c_str();
    file <<"\n"<<flush;
    char* cpy = new char[choice.size()+1];
    strcpy(cpy, choice.c_str());
    choices.push_back(cpy);
    items[i] = new_item(cpy," ");   
  }
  file.close();
  items[nsongs-1] = new_item((char *)NULL, NULL);
  LSTWINDOW* menu_win = new LSTWINDOW;
  menu_win->win = newwin(LINES-2,COLS,2,COLS/4);
  menu_win->menu = new_menu((ITEM **)items);
  printw("%d", item_count(menu_win->menu));
  set_menu_win(menu_win->menu,menu_win->win);
  set_menu_sub(menu_win->menu,derwin(menu_win->win,LINES-4,70 ,0,0));
  set_menu_format(menu_win->menu,LINES-6,1);
  set_menu_mark(menu_win->menu,"*");
  wlist.push_back(menu_win->win);
  focused = menu_win;    
  menu_opts_off(menu_win->menu,O_SHOWDESC);
}
