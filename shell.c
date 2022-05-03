#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <linux/kernel.h>
#include <sys/sysinfo.h>
#include <libgen.h>
#include <signal.h>
#include <sys/wait.h>

int loop = 1;
int main(void) {

    system("clear");

    char input[1024];

    while (loop) {

        prompt(); //implement custom promt

        fgets(input, 1024, stdin); // get input

        router(input); //set router to given input
    }

    return 0;
}

void prompt() {

    char * username; // init username
    char hostname[HOST_NAME_MAX + 1]; //init machine name

    username = getlogin(); // populate username

    gethostname(hostname, HOST_NAME_MAX + 1); // populate machine name

    char cwd[PATH_MAX];

    purple_color();
    printf("%s", hostname);  // set color and text for hostname

    yellow_color();
    printf("@");  // set color and text for @ sign

    red_color();
    printf("%s", username); // set color and text for machine name

    reset();

    printf(":~$ "); // add :~$ from assigment

}

void router(char input[1024]) {

    char function [10] = ""; // function list
    int fun_counter = 0; // counter how many functions there are


    // get function from given input string that would operate shell
    for (int i = 0; i < strlen(input); i++) {
        if (input[i] != ' ' || (int) input[i] == 0) {
            function [fun_counter] = input[i];
            fun_counter++;
        } else {
            break;
        }
    }


    function [strcspn(function, "\n")] = 0;

    if (strcmp(function, "cmd/copy") == 0) { //check if shell command is copy
        copy(input);
    } else if (strcmp(function, "cmd/history") == 0) { //check if shell command is history
        history_log();
    } else if (strcmp(function, "cmd/free") == 0) { //check if shell command is free
        command_free();
    } else if (strcmp(function, "cmd/fortune") == 0) { //check if shell command is fortune
        fortune();
    } else if(strcmp(function, "cmd/ibu") == 0){ //check if shell command is custom IBU art
        for (int i = 0; i < 50; i++){
            system("sleep 0.01");
            system("clear");
            ibuLogo(i); // the IBU moves 50 spaces
        }
        system("clear");
    }else if ((int)function [0] != 0) { //check if shell command is invalid and display message
        printf("%s: command not found\n", function);
    }

}


#define MAX_LINE 80
#define BUFFER_SIZE 50
#define buffer
char history[10][BUFFER_SIZE];
int count = 0;


void purple_color() {
    printf("\033[0;35m"); // Set bash color purple
}

void yellow_color() {
    printf("\033[0;33m"); // Set bash color yellow
}

void red_color() {
    printf("\033[0;31m"); // Set bash color red
}

void reset() {
    printf("\033[0m"); // Reset bash color
}

void copy(char input[]) {
    FILE * fp1, * fp2;
    char source[100]; //initialize source name
    char destination[100]; //initialize destination name

    printf("Enter source and destination file name:"); //print message for user to enter source and destination
    scanf("%s %s", source, destination); //scan for source and destination names
    fp1 = fopen(source, "r");  // open file with source name
    fp2 = fopen(destination, "w"); // open file with destination name
    if (!fp1 || !fp2) {
        printf("error occured"); //print error if one of files went wrong
    }
    char c;
    while ((c = fgetc(fp1)) != EOF) {
        fputc(c, fp2); //copy content from file1 to file2
    }
    fclose(fp1); //close file1
    fclose(fp2); //close file1
}

void displayHistory() {

    printf("shell command history:\n");

    int i;
    int j = 0;
    int histCount = count;

    for (i = 0; i < 10; i++) {
        printf("%d.  ", histCount);
        while (history[i][j] != '\n' && history[i][j] != '\0') {
            printf("%c", history[i][j]); //print history log if exists
            j++;
        }
        printf("\n");
        j = 0;
        histCount--;
        if (histCount == 0)
            break;
    }
    printf("\n");
}

int formatCommand(char inputBuffer[], char * args[], int * flag) {
    int length;
    int i;
    int start;
    int ct = 0;

    length = read(STDIN_FILENO, inputBuffer, MAX_LINE);

    start = -1;
    if (length == 0)
        exit(0);
    if (length < 0) {
        printf("Command not read\n");
        exit(-1);
    }

    for (i = 0; i < length; i++) {
        switch (inputBuffer[i]) {
            case ' ':
            case '\t':
                if (start != -1) {
                    args[ct] = & inputBuffer[start];
                    ct++;
                }
                inputBuffer[i] = '\0';
                start = -1;
                break;

            case '\n':
                if (start != -1) {
                    args[ct] = & inputBuffer[start];
                    ct++;
                }
                inputBuffer[i] = '\0';
                args[ct] = NULL; // no more args
                break;

            default:
                if (start == -1)
                    start = i;
                if (inputBuffer[i] == '&') {
                    * flag = 1;
                    inputBuffer[i] = '\0';
                }
        }
    }

    args[ct] = NULL;

    if (strcmp(args[0], "history") == 0) {
        if (count > 0) {

            displayHistory();
        } else {
            printf("\nNo Commands in the history\n");
        }
        return -1;
    } else if (args[0][0] - '!' == 0) {
        int x = args[0][1] - '0';
        int z = args[0][2] - '0';

        if (x > count) {
            printf("\nNo Such Command in the history\n");
            strcpy(inputBuffer, "Wrong command");
        } else if (z != -48) {
            printf("\nNo Such Command in the history. Enter <=!9 (buffer size is 10 along with current command)\n");
            strcpy(inputBuffer, "Wrong command");
        } else {

            if (x == -15) {
                strcpy(inputBuffer, history[0]);
            } else if (x == 0) {
                printf("Enter proper command");
                strcpy(inputBuffer, "Wrong command");
            } else if (x >= 1) {
                strcpy(inputBuffer, history[count - x]);

            }

        }
    }
    for (i = 9; i > 0; i--)
        strcpy(history[i], history[i - 1]);

    strcpy(history[0], inputBuffer);
    count++;
    if (count > 10) {
        count = 10;
    }
}

void history_log() {
    char inputBuffer[MAX_LINE];
    int flag;
    char * args[MAX_LINE / 2 + 1];
    int should_run = 1;

    pid_t pid, tpid;
    int i;

    while (should_run) {
        flag = 0;
        printf("history>"); //print history to flag logs in shell
        fflush(stdout);
        if (-1 != formatCommand(inputBuffer, args, & flag)) {
            pid = fork(); //instance of curren fork

            if (pid < 0) { //check if fork failed or succeseed

                printf("fork failed.\n");
                exit(1); // exit when fail
            } else if (pid == 0) {

                if (execvp(args[0], args) == -1) {

                    printf("error executing command\n");
                }
            } else {
                i++;
                if (flag == 0) {
                    i++;
                    wait(NULL);
                }
            }
        }
    }
}

void command_free() {
    int * ptr; // init list
    int n, i;

    n = 5;
    printf("elements size %d\n", n);
    ptr = (int * ) malloc(n * sizeof(int)); // allocate memory to list

    printf("memory is allocated\n");

    free(ptr); //free list memory
    printf("memory is free now\n");
}

int random_int(int Size) { //function retreive random number for given size
    int k;
    k = 0;
    srand((unsigned) time(NULL));
    k = rand() % Size;
    return k;
}


const char* fortunes_list[] = {
        "freebsd-tips", "osfortune", "zippy",
        "startrek", "debian-hints",
        "limerick", "fortunes2", "linuxcookie" // fortunes commands so you could pack eny of these
};
const size_t fortunes_size = sizeof(fortunes_list)/sizeof(*fortunes_list) - 1;
void fortune() {
    execlp("/usr/games/fortune", fortunes_list[random_int(fortunes_size)], "-l", NULL, NULL);
}

void print(char *line, int number){
    for(int i = 0; i < number; i++){
        printf(" "); // add spaces to simulate moving
    }
    printf("%s\n", line); // when spaces added add car line content
}

void ibuLogo(int spaces){
    purple_color();

    char *line1 = "####  ########   ###   ###";
    char *line2 = "####  ###   ###  ###   ###";
    char *line3 = "####  #########  ###   ###";
    char *line4 = "####  ###   ###  ###   ###";
    char *line5 = "####  ########   #########";

    print(line1, spaces);
    print(line2, spaces);
    print(line3, spaces);
    print(line4, spaces);
    print(line5, spaces);
    reset();
}

void forkbomb(){
    red_color();

    /**
     * The forkbomb command is simple in nature, all we need is a infinite loop and the fork command
     * forkbomb weren't implemented so you can thank me later for not breaking your PC ;)
     */

    while(1){
        fork();
    }

}