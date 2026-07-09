#define _DEFAULT_SOURCE
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

enum editorKey{
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    PAGE_UP,
    PAGE_DOWN,
    HOME_KEY,
    END_KEY,
};

typedef struct {int size; char *chars;} erow;

struct editorConfig { int screenrows; int screencols;
int cx; int cy; int numrows; erow *row;};

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
int editorReadKey(){
    int nread;
    char c; char seq[2];
    while((nread = read(STDIN_FILENO, &c, 1)) !=1){
    if(nread == -1 && errno != EAGAIN) die("Read");
    }
    if(c =='\x1b'){
            read(STDIN_FILENO, &seq[0], 1);
            read(STDIN_FILENO, &seq[1], 1);
        if(seq[0] == '['){
        switch(seq[1]){
            char trash;
            case 'A':
                return ARROW_UP;
            break;
            case 'B':
                return ARROW_DOWN;
            break;
            case 'C':
                return ARROW_RIGHT;
            break;
            case 'D':
                return ARROW_LEFT;
            break;
            case '5':
                read(STDIN_FILENO, &trash, 1);
                return PAGE_UP;
            break;
            case '6':
                read(STDIN_FILENO, &trash, 1);
                return PAGE_DOWN;
            break;
            case 'H':
                read(STDIN_FILENO, &trash, 1);
                return HOME_KEY;
            break;
            case 'F':
                read(STDIN_FILENO, &trash, 1);
                return END_KEY;
            break;
            }
        }
        return '\x1b';
    }
    return c ;
}
void editorMoveCursor(int key){
switch(key){
    case ARROW_LEFT:
        if(E.cx != 0){
        E.cx--;
        }
    break;
    case ARROW_RIGHT:
        if(E.cx < E.screencols - 1){
        E.cx++;
        }
    break;
    case ARROW_UP:
        if(E.cy != 0){
        E.cy--;
        }
    break;
    case ARROW_DOWN:
        if(E.cy < E.screenrows - 1 ){
         E.cy++;
        }
    break;
    }
}
void editorProcessKeypress(){
    int c = editorReadKey();
    switch(c){
        case CTRL_KEY('q'):
            exit(0);
        break;
        case PAGE_UP:
        for (int count = 0; count < E.screenrows; count++){
        editorMoveCursor(ARROW_UP);
        }
        break;
        case PAGE_DOWN:
        for(int count = 0; count < E.screenrows; count++){
            editorMoveCursor(ARROW_DOWN);
        }
        break;
        case HOME_KEY:
            E.cx = 0;
        break;
        case END_KEY:
            E.cx = E.screencols - 1;
        break;
        default:
            editorMoveCursor(c);
        break;
    }
  }
void editorDrawRows(struct abuf *ab){
    int i;
    char welcome[] = "---My Editor V1.0---";
    int num = strlen (welcome);
    if(num > E.screencols){
                num = E.screencols;
    }
    for(i = 0; i < E.screenrows; i++){
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
                if(i >= E.numrows){
                abAppend(ab, "~", 1);
                }else{
                    int len = E.row[i].size;
                    if(E.row[i].size > E.screencols){
                    len = E.screencols;
                    }
                    abAppend(ab, E.row[i].chars, len);
                }
            }
        if(i < E.screenrows - 1)
        abAppend(ab, "\r\n", 2);
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
    E.numrows = 0;
    if(getWindowSize( &E.screenrows, &E.screencols ) == -1)
    die ("getWindowSize");
}
void editorAppendRow( char *s, size_t len){
    E.row =  realloc(E.row, (E.numrows + 1) * sizeof(erow));
    E.row[E.numrows].size = len;
    E.row[E.numrows].chars = malloc(len + 1);
    memcpy(E.row[E.numrows].chars, s, len);
    E.row[E.numrows].chars[len] = '\0';
    E.numrows++;

}
void editorOpen(const char *filename){
    FILE *fp = fopen(filename, "r");
    if(fp==NULL){
    die("fopen");
    }
    // variaveis de controle
    char *line = NULL; size_t linecap = 0; ssize_t linelen;
     linelen = getline(&line, &linecap, fp);
        while(linelen != -1){
            if(line[linelen - 1] == '\n' || line[linelen - 1] =='\r'){
                linelen--;

            }
            editorAppendRow(line, linelen);
            linelen = getline(&line, &linecap, fp);
        }
        free(line);
        fclose(fp);
}

int main (int argc, char *argv[]){
    initEditor();
    if (argc >= 2){
        editorOpen(argv[1]);
    }
    enableRawMode();
    while(1){
        editorRefreshScreen();
        editorProcessKeypress();
    }
    return 0;
}
