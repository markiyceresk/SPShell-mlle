#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#ifdef _WIN32                                                                                                                                                   // if it's slopos, show error
    #error "WOOP!!! WOOP!!! Microslop detected! This software is for Unix-based systems only. You must install Linux NOW!!! https://www.linux.org/pages/download/ !!!"          // ^ It's fully true
#endif
#if defined(__linux__)                                  // if it's Linux
    #define PLATFORM_NAME "Linux"                       // ^ define platform name
#elif defined(__APPLE__) && defined(__MACH__)           // if it's macOS
    #define PLATFORM_NAME "macOS"                       // ^ define platform name
#elif defined(__FreeBSD__)                              // if it's FreeBSD
    #define PLATFORM_NAME "FreeBSD"                     // ^ define platform name
#else                                                   // if it's something else, just say it's Unix-based
    #define PLATFORM_NAME "Unknown Unix"                // ^ define platform name
#endif


char dist[256];
char *history[1000000];
int history_count = 0;



struct user {
    char name[256];
    char dir[262];
} user;

struct pid {
    pid_t a;
    pid_t b;
    pid_t c;
    pid_t d;
} pid;



// GET CHAR FROM INPUT
int getch(void) {
    int r, c;
    struct termios oldt, newt;
    int ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

// RETURN IF IT'S THE NOTFIRST COLUMN, ELSE PRINT NEW LINE
void ret_if_not_fir_col() {
    struct termios oldt, newt;
    char buf[32];
    int i = 0;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    printf("\033[6n");
    fflush(stdout);

    while (i < sizeof(buf) - 1) {
        if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
        if (buf[i] == 'R') break;
        i++;
    }
    buf[i] = '\0';

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    int row, col;
    if (sscanf(buf, "\033[%d;%dR", &row, &col) == 2) {
        if (col != 1) printf("\n");
}}

// GET COMMAND
char *getcmd(void) {
    char cmd[255] = {0}, cwd[256] = {0}; // buffer for command and command path
    int ch, pos = 0, hist_count = 0; // current char, position of cursor and current position in history
    getcwd(cwd, sizeof(cwd));                                                                                       // get current working directory
    printf("%s %s > ", user.name, cwd);                                                                             // print greeting
    while (1) {

        ch = getch();

        if (ch == 27) {                                                                                             // if it's an escape sequence
            ch = getch();                                                                                           // | read the next char
            if (ch == 91) {                                                                                         // | and if it's a arrow key
            ch = getch();                                                                                           // | | read
            if (ch == 65) { if (hist_count < history_count) {                                                       // | | if it's the up arrow, go back in history
                printf("\r\033[K");                                                                                 // | | | clear current line
                printf("%s %s > ", user.name, cwd);                                                                 // | | | print greeting
                hist_count++;                                                                                       // | | | move up in history
                printf("%s", history[history_count - hist_count]);                                                  // | | | print the command from history
                pos = strlen(history[history_count - hist_count]);                                                  // | | | move position to the end of the command
                strcpy(cmd, history[history_count - hist_count]); }}                                                // | | ^ memorise the command in cmd
            else if (ch == 66) { if (hist_count > 1) {                                                              // | | else if it's the down arrow and it's not at the beginning of history, go forward in history
                printf("\r\033[K");                                                                                 // | | | clear current line
                printf("%s %s > ", user.name, cwd);                                                                 // | | | print greeting
                hist_count--;                                                                                       // | | | | move down in history
                printf("%s", history[history_count - hist_count]);                                                  // | | | | print the command from history
                pos = strlen(history[history_count - hist_count]);                                                  // | | | | move position to the end of the command
                strcpy(cmd, history[history_count - hist_count]); }                                                 // | | | ^ memorise the command in cmd
                else {                                                                                              // | | | else if it's the down arrow and it's at the beginning of history, clear the line and reset history position
                    for (int i = 0; i < pos; i++) printf("\033[D \033[D");                                          // | | | | clear current line
                    pos = 0; strcpy(cmd, "");                                                                       // | | | | reset position and clear cmd
                    hist_count = 0; }}                                                                              // | | ^ ^ reset history position
                else if (ch == 67) { if (pos < strlen(cmd)) { printf("\033[C"); pos++; } }                          // | | else if it's the right arrow, move cursor right if it's not at the end of the input
                else if (ch == 68) { if (pos > 0) { printf("\033[D"); pos--; } }                                    // ^ ^ else if it's the left arrow, move cursor left if it's not at the beginning of the input
            }

        } else if (ch == 10) {                                                                                      // if it's enter, return command
            printf("\n");                                                                                           // | new string
            return strdup(cmd);                                                                                     // ^ return a copy of cmd

        } else if (ch == 127) {                                                                                     // if it's backspace, delete char before cursor
            if (pos > 0) {                                                                                          // | if it's not at the beginning of the input
                printf("\033[D");                                                                                   // | move cursor left
                printf("%s", cmd + pos);                                                                            // | print the rest of the command after the cursor
                printf(" ");                                                                                        // | print a space to clear the last char
                for (int i = 0; i < strlen(cmd) - pos + 1; i++) printf("\033[D");                                   // | move cursor back to the original position
                memmove(cmd + pos - 1, cmd + pos, strlen(cmd) - pos + 1);                                           // | redo it in cmd
                pos--;                                                                                              // ^ move position left
            }

        } else {                                                                                                    // if it's a regular char, insert it at the cursor position
            printf("%c", ch);                                                                                       // | print the char
            printf("%s", cmd + pos);                                                                                // | print the rest of the command after the cursor
            for (int i = 0; i < strlen(cmd) - pos; i++) printf("\033[D");                                           // | move cursor back to the original position
            memmove(cmd + pos + 1, cmd + pos, strlen(cmd) - pos + 1);                                               // | redo it in cmd
            cmd[pos++] = ch;                                                                                        // ^ insert the char in cmd and move position right
}}}

// GET USER INFO
void getuser() {
    if (PLATFORM_NAME[0] == 'U') {                                                                                  // if it's an unknown Unix, just say it's a unknown user in unknown directory
        strcpy(user.name, "guest");                                                                                 // | username
        strcpy(user.dir, "unknown");                                                                                // ^ user directory

    } else if (PLATFORM_NAME[0] == 'L') {                                                                           // if it's Linux, get username and user directory
        if (getuid() == 0) {                                                                                        // | if it's root, just say it's root in root directory
            strcpy(user.name, "root");                                                                              // | | username
            strcpy(user.dir, "/root");                                                                              // | ^ user directory
        } else {                                                                                                    // | else, get username and user directory
            strcpy(user.name, getenv("USER"));                                                                      // | | username from environment variable
            sprintf(user.dir, "/home/%s", user.name);                                                               // ^ ^ user directory from /home
        }

    } else if (PLATFORM_NAME[0] == 'm') {                                                                           // if it's macOS, get username and current directory
        if (getuid() == 0) {                                                                                        // | if it's root, just say it's root in root directory
            strcpy(user.name, "root");                                                                              // | | username
            strcpy(user.dir, "/var/root");                                                                          // | ^ user directory
        } else {                                                                                                    // | else, get username and user directory
            strcpy(user.name, getenv("USER"));                                                                      // | | username from environment variable
            sprintf(user.dir, "/Users/%s", user.name);                                                               // ^ ^ user directory from /home
        }

    } else if (PLATFORM_NAME[0] == 'F') {                                                                           // if it's FreeBSD, get username and current directory
        if (getuid() == 0) {                                                                                        // | if it's root, just say it's root in root directory
            strcpy(user.name, "root");                                                                              // | | username
            strcpy(user.dir, "/root");                                                                              // | ^ user directory
        } else {                                                                                                    // | else, get username and user directory
            strcpy(user.name, getenv("USER"));                                                                      // | | username from environment variable
            sprintf(user.dir, "/usr/home/%s", user.name);                                                           // ^ ^ user directory from /usr/home
        }

    } else {                                                                                                        // else, say true
        printf("Or you are using microslop, or your system isn't supported.\n");                                    // | print true
}}

// PRINT LOGO
void getlogo() {
	printf("\033[37m\033[40m|¯¯¯¯¯¯¯\033[37m\033[44m¯¯¯¯¯¯¯¯\033[37m\033[40m¯¯¯¯¯¯¯|\033[0m\n");
	printf("\033[37m\033[40m|       \033[37m\033[44m        \033[37m\033[40m       |\033[0m\n");
	printf("\033[37m\033[40m|       \033[37m\033[44m        \033[37m\033[40m       |\033[0m\n");
	printf("\033[37m\033[40m|       \033[37m\033[44m        \033[37m\033[40m       |\033[0m\n");
	printf("\033[41m\033[37m|       \033[37m\033[45m        \033[41m\033[37m       |\033[0m\n");
	printf("\033[41m\033[37m|       \033[37m\033[45m        \033[41m\033[37m       |\033[0m\n");
	printf("\033[41m\033[37m|       \033[37m\033[45m        \033[41m\033[37m       |\033[0m\n");
	printf("\033[41m\033[37m|       \033[37m\033[45m        \033[41m\033[37m       |\033[0m\n");
	printf("\033[37m\033[40m|       \033[37m\033[44m        \033[37m\033[40m       |\033[0m\n");
	printf("\033[37m\033[40m|       \033[37m\033[44m        \033[37m\033[40m       |\033[0m\n");
	printf("\033[37m\033[40m|       \033[37m\033[44m        \033[37m\033[40m       |\033[0m\n");
	printf("\033[37m\033[40m|_______\033[37m\033[44m________\033[37m\033[40m_______|\033[0m\n");
}

// PRINT GREETING
void greeting() {
    getuser();                                                                          // get user info
    getlogo();                                                                          // print logo
    printf("\n!!! НЕТ ВОЙНЕ !!!\n\nHi, %s!\nWhat would you do?\n\n", user.name);        // print anti-war and greeting message
}

// ==================== MAIN =============================================================================

int main() {
greeting();
FILE *file;
char *strcmd, *token, *strsplcmd, *elsecmd, strfile[256];
char *cmd[255], *splcmd[255];
int i, j, k, o_k, reded, old_out, fd;
while (1) {

    // get command
    strcmd = getcmd();                // get command with greeting
    if (strcmd == NULL) continue;     // if it's empty, ask for command again
    history[history_count++] = strdup(strcmd);      // save command in history

    // ------------------------------
    // split command into subcommands
    // ------------------------------

    token = strtok(strcmd, ";");
    i = 0;
    while (token != NULL && i < 255) {
        splcmd[i++] = token;
        token = strtok(NULL, ";");
    }
    splcmd[i] = NULL;                       // null-terminate the command array

    // -------------------
    // execute subcommands
    // -------------------

    j = 0;
    i = 0;
    reded = 0;

    while (splcmd[j] != NULL) {
        reded = 0;

    strsplcmd = splcmd[j];
    elsecmd = strdup(strsplcmd);

    // split command into tokens
    token = strtok(strsplcmd, " ");            // split command into tokens
    i = 0;
    while (token != NULL && i < 255) {
        cmd[i++] = token;
        token = strtok(NULL, " ");
    }
    cmd[i] = NULL;                          // null-terminate the command array

    k = 0;
    while (cmd[k] != NULL && strcmp(cmd[k], ">>") != 0) k++;
    if (cmd[k] != NULL) {
        reded = 1;
        o_k = k;
        strfile[0] = '\0';
        k++;
        while (cmd[k] != NULL) { strcat(strfile, cmd[k]); k++; }
        fflush(stdout);
        old_out = dup(STDOUT_FILENO);
        fd = open(strfile, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (fd == -1) { perror("file open error"); close(old_out); reded = 0; continue; }
        dup2(fd, STDOUT_FILENO);
        close(fd);
        while (cmd[o_k] != NULL) { cmd[o_k] = NULL; o_k++; }
    }

// --- COMMANDS ---
    if (strcmp(cmd[0], "q") == 0) {
        printf("Bye!\n");
        break;
    } else if (strcmp(cmd[0], "rt") == 0) {
        for (i = 1; cmd[i] != NULL; i++) printf("%s ", cmd[i]);
        printf("\n");
    } else if (strcmp(cmd[0], "cd") == 0) {
        if (cmd[1] == NULL) { chdir(user.dir); } else { chdir(cmd[1]); }
    } else if (strcmp(cmd[0], "clear") == 0 || strcmp(cmd[0], "cls") == 0) {
        printf("\033[H\033[J");
			// mkdir
	} else if (strcmp(cmd[0], "md") == 0) {
		pid.a = fork();
		if (pid.a == 0) {
			execvp(cmd[0], cmd);
			perror("execvp failed");
		} else {
			wait(NULL);
		}
	} else if (strcmp(cmd[0], "export") == 0 || strcmp(cmd[0], "ex") == 0) {
		if (setenv(cmd[1], cmd[2], 1) != 0) { perror("setenv"); }

// --- ELSE ---
    } else {
        pid.a = fork();
        if (pid.a == 0) {
            execvp(cmd[0], cmd);
            perror("execvp failed");
            exit(EXIT_FAILURE);
        } else {
            wait(NULL);
        }
    }
if (reded) { fflush(stdout); dup2(old_out, STDOUT_FILENO); close(old_out); } ret_if_not_fir_col(); j++; } } free(strcmd); return 0; }

// ==================== MAIN =============================================================================
