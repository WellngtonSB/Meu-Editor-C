#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>

#define CTRL_KEY(k) ((k) & 0x1f)

struct editorConfig {
int screenrows;
int screencols;
};
struct editorConfig E;
struct termios orig_termios;

void die( const char *s){
    write (STDOUT_FILENO, "\x1b[2J" , 4 );
    write (STDOUT_FILENO, "\x1b[H", 3);
    perror(s);
    exit(1);
}
void disableRawMode(){
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
    die("tcsetattr");
}
void enableRawMode(){

    if(tcgetattr(STDIN_FILENO, &orig_termios)== -1)
    die("tcgetattr");
    atexit (disableRawMode);

    struct termios raw = orig_termios;
    raw.c_oflag &= ~(OPOST);
    raw.c_iflag &= ~(IXON | BRKINT | INPCK | ISTRIP);
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    raw.c_cc [VMIN] = 0;
    raw.c_cc [VTIME] = 1;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}
char editorReadKey(){
    int nread;
    char c;
    while((nread = read(STDIN_FILENO, &c, 1)) !=1) {
    if(nread == -1 && errno != EAGAIN) die("Read");
    }
    return c ;
}
void editorProcessKeypress(){
    char c = editorReadKey();
    if (c == CTRL_KEY ('q'))
    exit(0);
}
void editorDrawRows(){
    int i;
    for (i = 0; i < E.screenrows; i++){
    write (STDOUT_FILENO, "~", 1);

    if(i < E.screenrows - 1)
    write (STDOUT_FILENO, "\r\n", 2);
    }
}

void editorRefreshScreen(){
    write (STDOUT_FILENO, "\x1b[2J" , 4 );
    write (STDOUT_FILENO, "\x1b[H", 3);
    editorDrawRows();
}
int getCursorPosition(int *rows, int *cols){
    char buff[32];
    unsigned int i = 0;
    while(i < sizeof(buff) - 1 ){
    buff[i] = editorReadKey();
    i++;
    if(buff[i-1] == 'R'){
        break;
        }
    }
    buff[i] = '\0';
    if (buff[0] != '\x1b' || buff[1] != '[') return -1;
        if(sscanf( &buff[2],"%d;%d", rows, cols) != 2){
        return -1;
      }
      return 0;
}
int getWindowSize(int *rows, int *cols){
    struct winsize ws;
    if(ioctl (STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0){
    write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12);
    write(STDOUT_FILENO, "\x1b[6n", 4);
    if(getCursorPosition(rows, cols) == -1){
        *rows = 24;
        *cols = 80;
    return 0;
        }
    }else{
    *rows = ws.ws_row;
    *cols = ws.ws_col;
    return 0;
    }
    return 0;
  }
void initEditor(){
     if(getWindowSize(&E.screenrows, &E.screencols ) == -1)
    die ("getWindowSize");
}
int main (){
    initEditor();
    enableRawMode();

    while(1){
        editorRefreshScreen();
        editorProcessKeypress();
    }
    return 0;
}
