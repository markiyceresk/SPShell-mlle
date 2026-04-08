#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <sys/wait.h>

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

struct cmd {
    char source[255];
    char *tokens[256];
    char *sub[256];
} cmd;

int history_count = 0;

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

// PARSING

void parse_command() {
    char quote;
    char buffer[255];
    size_t len = 0;
    buffer[0] = '\0';
    int i, t = 0;
    for (i = 0; cmd.source[i] != '\0'; i++) {
        len = strlen(buffer);
        if (cmd.source[i] != '\0' && cmd.source[i] != ' ' && cmd.source[i] != '\t' &&
                    cmd.source[i] != '>' && cmd.source[i] != '<' && !(cmd.source[i] == '2' &&
                    cmd.source[i+1] == '>') && cmd.source[i] != ';' && cmd.source[i] != '$') {
            if (cmd.source[i] == '\"' || cmd.source[i] == '\'') {
                quote = cmd.source[i];
                i++;
                while (cmd.source[i] != '\0' && cmd.source[i] != quote) {
                    buffer[len] = cmd.source[i];
                    i++; len++;
                }
                buffer[len] = '\0';
            } else { buffer[len] = cmd.source[i]; buffer[len + 1] = '\0'; len++; }

        } else if (cmd.source[i] == ' ' || cmd.source[i] == '\t') {
            if (buffer[0] != '\0') {
                cmd.tokens[t++] = strdup(buffer);
                buffer[0] = '\0';
            }
        } else if (cmd.source[i] == '>') {
            if (buffer[0] != '\0') {
                cmd.tokens[t++] = strdup(buffer);
                buffer[0] = '\0';
            }
            if (cmd.source[i+1] && cmd.source[i+1] == '>') {
                cmd.tokens[t++] = ">>";
                i++;
            } else {
                cmd.tokens[t++] = ">";
            }
        } else if (cmd.source[i] == '<') {
            if (buffer[0] != '\0') {
                cmd.tokens[t++] = strdup(buffer);
                buffer[0] = '\0';
            }
            cmd.tokens[t++] = "<";
        } else if (cmd.source[i] == '2' && cmd.source[i+1] && cmd.source[i+1] == '>') {
            if (buffer[0] != '\0') {
                cmd.tokens[t++] = strdup(buffer);
                buffer[0] = '\0';
            }
            if (cmd.source[i+2] && cmd.source[i+2] == '>') {
                cmd.tokens[t++] = "2>>";
                i += 2;
            } else {
                cmd.tokens[t++] = "2>";
                i++;
            }
        } else if (cmd.source[i] == ';') {
            if (buffer[0] != '\0') {
                cmd.tokens[t++] = strdup(buffer);
                buffer[0] = '\0';
            }
            cmd.tokens[t++] = ";";
        } else if (cmd.source[i] == '$') {
            char varname[256] = {0};
            int varlen = 0;
            i++;
            while (cmd.source[i] && ((cmd.source[i] >= 'a' && cmd.source[i] <= 'z') ||
                        (cmd.source[i] >= 'A' && cmd.source[i] <= 'Z') || cmd.source[i] == '_')) {
                varname[varlen++] = cmd.source[i];
                i++;
            }
            varname[varlen] = '\0';
            i--;  // важно
            char *val = getenv(varname);
            strcat(buffer, val);
        }
}
if (buffer[0] != '\0') {
    cmd.tokens[t++] = strdup(buffer);
}
cmd.tokens[t] = NULL;
}

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

int main(void) {

int c, i, k, o_k;
int reded, merged, old_in, old_out, old_out2, fd;

greeting();
while (1) {

    //getting command
    strcpy(cmd.source, getcmd());                           // get command
    history[history_count++] = strdup(cmd.source);          // add command to history
    parse_command();                                        // parsing

    c = 0;                                  // c it's current string number

    // for command lenght
    while (cmd.tokens[c] != NULL) {         // while isn't null

    // split in ";"
    i = 0;
    while (cmd.tokens[c] != NULL && strcmp(cmd.tokens[c], ";") != 0) {      // write tokens to subcommand variable
        cmd.sub[i++] = cmd.tokens[c++];                                     // ^ write
    }
    cmd.sub[i] = NULL;                                                      // last token is NULL

    // bypass ";"
    if (cmd.tokens[c] != NULL && strcmp(cmd.tokens[c], ";") == 0) {
        c++;
    }

        // if command is clear
    if (cmd.sub[0] == NULL) continue; 

    // --- out ---

    reded = 0;
    k = 0;
    while (cmd.sub[k] != NULL && strcmp(cmd.sub[k], ">>") != 0 && strcmp(cmd.sub[k], ">") != 0 &&
                strcmp(cmd.sub[k], "2>>") != 0 && strcmp(cmd.sub[k], "2>") != 0 && strcmp(cmd.sub[k], "<") != 0) k++;
    if (cmd.sub[k]) {
        o_k = k;
        reded = 1;
        k++;
        if (cmd.sub[k] == NULL) {
            fprintf(stderr, "syntax error: no file name\n");
            continue; 
        }
        old_in = dup(STDIN_FILENO);
        old_out = dup(STDOUT_FILENO);
        old_out2 = dup(STDERR_FILENO);
        fflush(stdout);
        fflush(stderr);
        char *filename = cmd.sub[k];
        k--;
        if (strcmp(cmd.sub[k], "<") == 0) {
            fd = open(filename, O_RDONLY, 0644);
            if (fd == -1) { perror("input open error"); reded = 0; continue; }
            dup2(fd, STDIN_FILENO);
        } else {
            if (strcmp(cmd.sub[k], ">>") == 0 || strcmp(cmd.sub[k], "2>>") == 0) fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
            else if (strcmp(cmd.sub[k], ">") == 0 || strcmp(cmd.sub[k], "2>") == 0) fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd == -1) { perror("file open error"); close(old_out); close(old_out2); reded = 0; continue; }
            if (strcmp(cmd.sub[k], ">>") == 0 || strcmp(cmd.sub[k], ">") == 0) dup2(fd, STDOUT_FILENO);
            else if (strcmp(cmd.sub[k], "2>>") == 0 || strcmp(cmd.sub[k], "2>") == 0) dup2(fd, STDERR_FILENO);
        }
        close(fd);
        cmd.sub[k] = NULL;
    }

    // --- out ---

        // quit
    if (strcmp(cmd.sub[0], "q") == 0 || strcmp(cmd.sub[0], "quit") == 0 || strcmp(cmd.sub[0], "exit") == 0) { printf("Bye!\n"); return 0; }

        // self-compile
    else if (strcmp(cmd.sub[0], "sc") == 0) {
        printf("Compiling myself...\n");
        pid.a = fork();
        if (pid.a == 0) {
            execl("/usr/bin/gcc", "gcc", "-o", "vos.c", "vos.bin", NULL);
            perror("execl failed");
            exit(EXIT_FAILURE);
        } else {
            wait(NULL);
            printf("Done!\n");
            execvp("./vos.bin", (char *[]){"./vos.bin", NULL});
            perror("execvp failed");
        }

        // command cd
    } else if (strcmp(cmd.sub[0], "cd") == 0) {
        if (cmd.sub[1] == NULL) { chdir(user.dir); } else { chdir(cmd.sub[1]); }

        // command clear (cls for slop os users)
    } else if (strcmp(cmd.sub[0], "clear") == 0 || strcmp(cmd.sub[0], "cls") == 0) {
        printf("\033[H\033[J");

        // command export
	} else if (strcmp(cmd.sub[0], "export") == 0 || strcmp(cmd.sub[0], "ex") == 0) {
		if (setenv(cmd.sub[1], cmd.sub[2], 1) != 0) { perror("setenv"); }
    
        // get logo
    } else if (strcmp(cmd.sub[0], "getlogo") == 0) {
        getlogo();

        // simle mkdir
	} else if (strcmp(cmd.sub[0], "md") == 0) {
        cmd.sub[0] = "mkdir";
		pid.a = fork();
		if (pid.a == 0) {
			execvp(cmd.sub[0], cmd.sub);
			perror("execvp failed");
		} else {
			wait(NULL);
		}

// ----- GAMES -----

	// command rps (rock paper scissors)
	} else if (strcmp(cmd.sub[0], "rps") == 0) {
		struct choose {
			short player;
			short computer;
			int score;
		} choose;
        choose.score = 0;
		struct termios t;
		char b;
		tcgetattr(0, &t);
    	struct termios n = t;
		n.c_lflag &= ~(ICANON | ECHO);
		tcsetattr(0, TCSANOW, &n);
		srand(time(0));
		while (printf("1. Rock\n2. Paper\n3. Scissors\n4. Quit\n") && read(0, &choose.player, 1) && choose.player != '4') {
			choose.player -= '0';
			choose.computer = rand() % 3 + 1;
			printf("\nComputer: ");
			if (choose.computer == 1) { printf("rock"); }
			else if (choose.computer == 2) { printf("paper"); }
			else if (choose.computer == 3) { printf("scissors"); }
			printf(" | ");
			if (choose.player == choose.computer) printf("Draw");
			else if ((choose.player % 3) + 1 == choose.computer) { printf("You lost"); choose.score--; }
			else { printf("You won"); choose.score++; }
			printf(" | Score: %d\n\n", choose.score);
		}
		tcsetattr(0, TCSANOW, &t);

			// ---- JUST FUNNY :) -----

			// command news
	} else if (strcmp(cmd.sub[0], "news") == 0) {
		srand(time(NULL));
		int rnew = rand() % 10;
		switch (rnew) {
			case 0:
				printf("Wheatley says, he isn't a moron! Everyone is laughing, but GLaDOS is potato.");
				break;
			case 1:
				printf("Microslop (didn't) make windows open source!!!!!!!!!");
				break;
			case 2:
				printf("I use Arch BTW");
				break;
			case 3:
				printf("Vladimir Putin says, internet is drugs!");
				break;
			case 4:
				printf("CEO of OpenAI says, vibecoding is our future! Is he right?");
				break;
			case 5:
				printf("'Tomorrow's wind will blow tomorrow' - Karych");
				break;
			case 6:
				printf("Scientists have taken on one of the most difficult questions in human history: Why do people dislike Viette's theorem so much?");
				break;
			case 7:
				printf("When you feel stupid, remember that Windows users don't use the terminal on a daily basis.");
				break;
			case 8:
				printf("Roskomnadzor has banned the letter 'A'");
				break;
			case 9:
				printf("Cheese ball.");
				break;
		}

	// command sus
	} else if (strcmp(cmd.sub[0], "sus") == 0) {
		printf("This looks kinda SUS...");

	// command hi
	} else if (strcmp(cmd.sub[0], "hi") == 0) {
		printf("Hello!");

	// command I am not a moron (reference to portal 2)
    } else if (strcmp(cmd.sub[0], "I am not a moron!") == 0) {
		printf("Yes, you are!!!\n");
		sleep(1);
		printf("You are the moron, they built to make me an idiot!");

	// command potato (reference to PWGood)
	} else if (strcmp(cmd.sub[0], "potato") == 0) {
		printf("Картошка!\n");
		sleep(1);
		printf("картошка\nкартошка\nкартошка!");

    } else {
        pid.a = fork();
        if (pid.a == 0) {
            execvp(cmd.sub[0], cmd.sub);
            perror("shell");
        } else {
            wait(NULL);
        }

    }

if (reded) { fflush(stdout); dup2(old_out, STDOUT_FILENO); close(old_out); fflush(stderr); dup2(old_out2, STDERR_FILENO); close(old_out2); fflush(stdin); dup2(old_in, STDIN_FILENO); close(old_in); }

ret_if_not_fir_col();

}}}
