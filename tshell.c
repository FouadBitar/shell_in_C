#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <signal.h>


// * interactive program: reads from terminal and writes to terminal
//read line, if valid run command, keep waiting
//takes a string and tries to execute it
//assume string points to valid executable file always available on disk (i.e. ls)
int my_system(char* cmd[]);
char* get_a_line();
void printDir();
char** tokenize(char* line);

int changeDir(char **args);
int internal_cmd(char** args, char* hist[], int* pCurrent); 
int displayHistory(char* history[], int *current);
int storeHistory(char* cmd,char* hist[], int* pCurrent);


//global variables
#define MAXIMUM_CMD_LENGTH  100
#define HISTORY_LENGTH 100

int endOfFile = 0;


//************signal section

typedef void (*sighandler_t)(int);
sighandler_t signal(int signum, sighandler_t handler);



void sigint_handler_C(int signo) {
    printf("\n\tDo you want to exit this shell (y/n)? : ");
    fflush(stdin);
    int r = getchar();
    if (r == '\n') r = getchar();
    while(r != 'n' && r != 'N' && r != 'y' && r != 'Y') {
        printf("\n\tinvalid input, enter the choice(y/Y/n/N) again : ");
        r = getchar();
        if (r == '\n') r = getchar();
    }

    if(r == 'y' || r == 'y') exit(0);
}

void sigtstp_handler_Z(int signo) {
    printf("\n\tMust use Ctl C! \n");
}

//************end of signal section







int main (int argc, char *argv[]) {

    char cmd[MAXIMUM_CMD_LENGTH];
    char *hist[HISTORY_LENGTH];
    int i;
    int current = 0;
    int* pCurrent = &current;
    int check = -1;
    
    signal(SIGINT, sigint_handler_C);
    signal(SIGTSTP, sigtstp_handler_Z);



    //storeHistory(line, hist, pCurrent);


    //initialize history buffer to be NULL
    for (i = 0; i < HISTORY_LENGTH; i++) {
        hist[i] = NULL;
    }
    
    while(1) {

        //print directory 
        printDir();

        //get line
        char* line = get_a_line();

        //save clean copy to be saved before tokenize 
        char lineCopy[MAXIMUM_CMD_LENGTH]; 
        strcpy(lineCopy, line);

        //tokenize
        char** tokenizedLine = tokenize(line);



        if(sizeof(line) > 1) {
            //check if internal command
            check = internal_cmd(tokenizedLine, hist, pCurrent);

            //if internal command was change directory and it is successful, store it
            if(check > 0) storeHistory(lineCopy, hist, pCurrent);
            else { 
                check = my_system(tokenizedLine);
                if (check > 0) storeHistory(lineCopy, hist, pCurrent);
            }
        }


        if(endOfFile == 1) break;
    }
    
    
    return 0;

}


int my_system(char* cmd[]) {
    //open a new process (child) - using fork
    //in new process, load file, execute
    int execResult = 1;
    pid_t pid = fork();

    //child
    if(pid == 0) {
        sleep(1);
        execResult = execvp(cmd[0], &cmd[0]);
        if (execResult < 0) printf("invalid command \n");
    }
    //parent
    else {
        //wait to ensure we wait for child to finish processing the command (my_system)
        wait(NULL);

    }
    return  execResult;
}


char* get_a_line() {
    //keep reading until end of line (i.e. return key)
    //should allow editing, i.e. deleting a character after it has been entered
    int buffSize = 1024;
    int position = 0;
    char *buffer = malloc(sizeof(char) * buffSize); 
    int nextChar;

    if (!buffer) {
        fprintf(stderr, "error mallocing memory \n");
        exit(1);
    }

    while(1) {
        nextChar = getchar();   //next char from stdin

        if(nextChar == EOF) endOfFile = 1;

        //if we reach end of file, or hit return key, return buffer with null character appended
        if (nextChar == EOF || nextChar == '\n' || nextChar == '\r') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = nextChar;
        }
        position++;

        if (position >= buffSize) {
            buffer[position] = '\0';
            return buffer;
        }
    }
}
#define TOKEN_DELIM " \t\r\n\a"
char** tokenize(char* line) {
    int buffSize = 1024;
    int position = 0;
    char **tokens = malloc(buffSize * sizeof(char*));
    char *token;

    if (!tokens) {
        fprintf(stderr, "allocation error\n");
        exit(-1);
    }

    token = strtok(line, TOKEN_DELIM);
    while (token != NULL || token != '\0') {
        tokens[position] = token;
        position++;
        token = strtok(NULL, TOKEN_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}


int changeDir(char** args) {
    if (args[1] == NULL) {
        fprintf(stderr, "need argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
        perror("lsh");
        }
    }
  return 1;
}


int internal_cmd(char **line, char* hist[], int* pCurrent){
  
    //token is null, return -2
    if (line[0] == NULL) {
        return -2;
    }

    //history command, on success 1, on failure -1
    if (strcmp(line[0], "history") == 0){
        displayHistory(hist, pCurrent);
        return 1;
    }

    //change directory command, on success 2, on failure -1
    else if (strcmp(line[0], "chdir") == 0 || strcmp(line[0], "cd") == 0) {
        changeDir(line);
        return 2;
    }

    //command not internal return 0
    else {
        return 0;
    }
  
    
}



int displayHistory(char* history[], int* current) {
    printf("\n");
    int curr = *current;
    int i = curr;
        int hist_num = 1;

        do {
                if (history[i]) {
                        printf("%4d  %s\n", hist_num, history[i]);
                        hist_num++;
                }

                i = (i + 1) % HISTORY_LENGTH;

        } while (i != curr);

        return 0;
    
}

int storeHistory(char* cmd, char* hist[], int* pCurrent){
    int current = *pCurrent;
    //remove the \n from fgets
    if (cmd[strlen(cmd) - 1] == '\n')
        cmd[strlen(cmd) - 1] = '\0';

    //free the memory so it does not cause memory leaks
    free(hist[current]);

    hist[current] = strdup(cmd);

    current += 1 % HISTORY_LENGTH;
    *pCurrent = current;

    return 0;
}

void printDir() { 
    char cwd[1024]; 
    getcwd(cwd, sizeof(cwd)); 
    printf("\n%s  >>> ", cwd); 
}
