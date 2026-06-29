#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>

#define CTRL_KEY(k) ((k) & 0x1f)
#define ABUF_INIT { NULL, 0}

struct editorConfig { int screenrows; int screencols; int cx; int cy; };
struct editorConfig E;
struct termios orig_termios;
struct abuf { char *b; int len; };

void abAppend (struct abuf *ab, const char *s, int len){
    char *new = realloc(ab->b, ab->len + len);
    if(new == NULL)
    return ;
    memcpy(&new[ab ->len], s, len);
    ab->b = new;
    ab->len += len;
}
 void abFree (struct abuf *ab){
    free(ab->b);
    ab->b = NULL;
    ab->len = 0;
 }
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
    switch(c){
        case CTRL_KEY('q'):
            exit(0);
        break;

        default:
            editorMoveCursor(c);
        break;
    }
}
void editorDrawRows(struct abuf *ab){
    int i;
    char welcome[] = "*** Well Editor V1.0 ***";
    int num = strlen (welcome);
    if(num > E.screencols){
                num = E.screencols;
    }
    for (i = 0; i < E.screenrows; i++){
        abAppend(ab, "\x1b[K", 3);
            if(i == E.screenrows / 3){
                int padding = (E.screencols - num) /2;
                if(padding > 0){
                abAppend(ab, "~", 1);
                padding -= 1;
            }
    while(padding > 0 ){
        abAppend(ab, " ", 1);
        padding -= 1;
    }
        abAppend(ab, welcome, num);
            }else{
                abAppend(ab, "~", 1);
            }
        if(i < E.screenrows - 1)
        abAppend(ab, "\r\n", 2);
    }
}
void editorMoveCursor(int key){
switch(key){
    case 'a':
        E.cx--;
    break;
    case 'd':
        E.cx++;
    break;
    case 'w':
        E.cy--;
    break;
    case 's':
        E.cy++;
    break;
    }
}
void editorRefreshScreen(){
    char buff[32];
    struct abuf ab = ABUF_INIT;
    editorDrawRows(&ab);
    snprintf(buff, sizeof(buff),"\x1b[%d;%dH", E.cy + 1, E.cx + 1);
    abAppend(&ab, buff, strlen(buff));
    write(STDOUT_FILENO, ab.b, ab.len);
    abFree(&ab);

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
        }
        return 0;

    }else{
    *rows = ws.ws_row;
    *cols = ws.ws_col;
    return 0;
    }
    return 0;
  }
void initEditor(){
    E.cx = 0;
    E.cy = 0;
     if(getWindowSize( &E.screenrows, &E.screencols ) == -1)
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
