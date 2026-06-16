#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <errno.h>
#define CTRL_KEY(k) ((k) & 0x1f)

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
    for (i = 0; i < 24; ++i){
    write(STDOUT_FILENO, "~", 1);
    write (STDOUT_FILENO, "\r\n", 3);
    }
}
void editorRefreshScreen(){
    write (STDOUT_FILENO, "\x1b[2J" , 4 );
    write (STDOUT_FILENO, "\x1b[H", 3);
    editorDrawRows();
}
int main (){
    enableRawMode();

    while(1){
        editorRefreshScreen();
        editorProcessKeypress();
    }
    return 0;
}
