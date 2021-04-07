// Shell starter file
// You may make any changes to any part of this file.

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <limits.h>

#define COMMAND_LENGTH 1024
#define NUM_TOKENS (COMMAND_LENGTH / 2 + 1)

#define HISTORY_DEPTH 1024
//function declaration
void command_split(char* tokens[], char *history);
void command_history(char*tokens[],_Bool* in_background);
//variable declaration
char history[HISTORY_DEPTH][COMMAND_LENGTH];
char temp[COMMAND_LENGTH][COMMAND_LENGTH];
volatile sig_atomic_t flag = 0;
int command = 0;
int hist = 0;
/**
 * Command Input and Processing
 */



int tokenize_command(char *buff, char *tokens[])
{
	int token_count = 0;
	_Bool in_token = false;
	int num_chars = strnlen(buff, COMMAND_LENGTH);
	for (int i = 0; i < num_chars; i++) {
		switch (buff[i]) {
		// Handle token delimiters (ends):
		case ' ':
		case '\t':
		case '\n':
			buff[i] = '\0';
			in_token = false;
			break;

		// Handle other characters (may be start)
		default:
			if (!in_token) {
				tokens[token_count] = &buff[i];
				token_count++;
				in_token = true;
			}
		}
	}
	tokens[token_count] = NULL;
	return token_count;
}

/**
 * Read a command from the keyboard into the buffer 'buff' and tokenize it
 * such that 'tokens[i]' points into 'buff' to the i'th token in the command.
 * buff: Buffer allocated by the calling code. Must be at least
 *       COMMAND_LENGTH bytes long.
 * tokens[]: Array of character pointers which point into 'buff'. Must be at
 *       least NUM_TOKENS long. Will strip out up to one final '&' token.
 *       tokens will be NULL terminated (a NULL pointer indicates end of tokens).
 * in_background: pointer to a boolean variable. Set to true if user entered
 *       an & as their last token; otherwise set to false.
 */
void read_command(char *buff, char *tokens[], _Bool *in_background)
{
	*in_background = false;

	// Read input
	int length = read(STDIN_FILENO, buff, COMMAND_LENGTH-1);

	if ( (length < 0) && (errno !=EINTR) ){
    perror("Unable to read command. Terminating.\n");
    exit(-1);  /* terminate with error */
	}

	// Null terminate and strip \n.
	buff[length] = '\0';
	if (buff[strlen(buff) - 1] == '\n') {
		buff[strlen(buff) - 1] = '\0';
	}

	// Tokenize (saving original command string)
	int token_count = tokenize_command(buff, tokens);
	if (token_count == 0) {
		return;
	}

	// Extract if running in background:
	if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0) {
		*in_background = true;
		tokens[token_count - 1] = 0;
	}
}

/*excute the build in command 
*exit()
*pwd()
*cd()
*help()
*history
*/
void run_buildtin(char* tokens[], char *buff,_Bool *in_background){
    char builtin_command[7][100] ={"exit", "pwd", "cd", "help", "history", "!!", "!" };
    char builtin_command_desc[7][100] ={" - Exit the shell program", " - Display the current working directory.", " - Change the current working directory.", " - Display help information on internal commands.", " - Allows the user access up to the 10 most recently entered commands." , " - Run last command", " - Run selected command"};
    
    command_history(tokens, in_background);
    command++;
    //EXIT
	if (strcmp(tokens[0], "exit") == 0)
	{
		if(tokens[1] == NULL)
		{
			exit(0);
		}
		else
		{
			perror("Failed to exit");
		}
	}
    //PWD
	else if (strcmp(tokens[0],"pwd") == 0)
	{
		char cwd_buffer[500];
		char *cwd = getcwd(cwd_buffer, 500);
		cwd = strcat(cwd,"\n");
		if (cwd == NULL)
		{
			perror("Failed to obtain Work Directory");
		}
		else
		{
			write(STDOUT_FILENO,cwd,strlen(cwd));
		}
		return;
	}
    //CD
    else if (strcmp(tokens[0],"cd") == 0){
        if (tokens[1] == NULL){
            perror ("Expected argument to \"cd\"\n");
        }
        else{
            chdir (tokens[1]);
        }
        return;
    }
    //HELP
    else if (strcmp(tokens[0],"help") == 0){
        if (tokens[1] == NULL){
            for (int i =0; i<7; i++){
                write(STDOUT_FILENO, builtin_command[i], strlen(builtin_command[i]));
                write(STDOUT_FILENO, builtin_command_desc[i], strlen(builtin_command_desc[i]));
                write(STDOUT_FILENO, "\n", strlen("\n"));
            }
            return;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     
        }
        else if (tokens[2] != NULL){
            perror ("Too many arguments to \"help\"\n");
            return;
        }
        else if (tokens[1] != NULL){
            for (int i =0; i<7; i++){
                if (strcmp(tokens[1],builtin_command[i]) == 0){
                    write(STDOUT_FILENO, builtin_command[i], strlen(builtin_command[i]));
                    write(STDOUT_FILENO, builtin_command_desc[i], strlen(builtin_command_desc[i]));
                    write(STDOUT_FILENO, "\n", strlen("\n"));
                    return;
                }
            }
            write(STDOUT_FILENO, tokens[1], strlen (tokens[1]));
            write(STDOUT_FILENO, " is an external command or application", strlen(" is an external command or application"));
            write(STDOUT_FILENO, "\n", strlen("\n"));
            return;
            }
        return;
    }
    //history
    else if (strcmp(tokens[0],"history") == 0){
        int count = command;
        int counter = 0;
        int n = command;
        char display[command];

        for (int i = (count-1); i>=0 && counter <10; i--){
            sprintf(display,"%d\t",i);
            write(STDOUT_FILENO, display, strlen(display));
            write(STDOUT_FILENO, "\t", strlen("\t"));
            write(STDOUT_FILENO, history[n-1], strlen(history[n-1]));
            write(STDOUT_FILENO, "\n", strlen("\n"));
            n--;
            counter++;
        }
        return;
    }
    
    //!!
    else if (strcmp(tokens[0],"!!")==0){
        if (command == 0){
            write (STDOUT_FILENO, "No past command", strlen("No past command"));
            write (STDOUT_FILENO, "\n",strlen("\n"));
            return;
        }
        write (STDOUT_FILENO, "COMMAND TO BE EXECUTED", strlen("COMMAND TO BE EXECUTED"));
        write(STDOUT_FILENO, "\t", strlen("\t"));
        write(STDOUT_FILENO, history[hist], strlen(history[hist]));
        write(STDOUT_FILENO, "\n", strlen("\n"));
        command_split(tokens, history[hist]);
        run_buildtin(tokens,history[hist],in_background);
        for (int i =0; i< COMMAND_LENGTH; i++){
            for(int n=0; n<COMMAND_LENGTH; n++){
                temp[i][n] = '\0';
            }
            strcpy(temp[i]," ");
        }
        return;
    }
    
    // !n
    else if (strncmp(tokens[0],"!", 1)==0){
        int i =0;
        char num[100];
        char test[100];
        strcpy (test,tokens[0]);
        for (i = 1; i<strlen(test);i++){
            num[i-1]=test[i];
        }
        int number = atoi(num);
        if (number == 0||number > command){
            write (STDOUT_FILENO, "Unknown history number",strlen("Unknown history number"));
            write (STDOUT_FILENO, "\n",strlen("\n"));
        }
        write (STDOUT_FILENO, "COMMAND TO BE EXECUTED", strlen("COMMAND TO BE EXECUTED"));
        write(STDOUT_FILENO, "\t", strlen("\t"));
        write(STDOUT_FILENO, history[number], strlen(history[number]));
        write (STDOUT_FILENO, "\n",strlen("\n"));
        command_split(tokens,history[number]);
        run_buildtin(tokens, history[number],in_background);
        for (int i=0; i<COMMAND_LENGTH;i++){
            for (int x=0;x<COMMAND_LENGTH;x++){
                temp[i][x]='\0';
            }
            strcpy(temp[i],"");
        }
        return;
    }
	return;
}

void run_external(char* tokens[], _Bool* in_background){

	pid_t var_pid;
	pid_t wpid;
	int status;
	var_pid = fork(); 
    command_history(tokens,in_background);
    command++;

	if(var_pid <0){
		fprintf(stderr, "Fork Failed" );
		exit (-1);
	}
	else if(var_pid == 0 ){
		execvp(tokens[0], tokens);
		//will print if execvp failed, otherwise, no return value
		perror("execute error");
		exit(-1);
	}
	else if(!in_background) {
	// Wait for child to finish
		do {
			waitpid(var_pid, &status, WUNTRACED);
		} while(!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	// Wait for all child to finish
	while((wpid = wait(&status))>0);
	// Cleanup zombie processes
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

void check_command(char* tokens[], char* buff, _Bool* in_background){
	char builtin_command[7][100] ={"exit", "pwd", "cd", "help", "history", "!!" , "!"};

    if (strncmp(tokens[0],"!", 1)==0){
        run_buildtin(tokens,buff,in_background);
        return;
    }
    
	for (int i =0; i<7; i++){
		if (strcmp(tokens[0],builtin_command[i])==0){
			run_buildtin(tokens,buff,in_background);
			return;
		}
    }
	
	run_external(tokens, in_background);
	return;
}

void handle_SIGINT(){
	write(STDOUT_FILENO, "\n", strlen("\n"));
	exit(0);
}

void help(int sig){
    char builtin_command[7][100] ={"\nexit", "pwd", "cd", "help", "history","!!" ,"!"};
    char builtin_command_desc[7][100] ={" - Exit the shell program", " - Display the current working directory.", " - Change the current working directory.", " - Display help information on internal commands.", " - Allows the user access up to the 10 most recently entered commands." , " - Run last command", "- Run selected command"};
    for (int i =0; i<7; i++){
        write(STDOUT_FILENO, builtin_command[i], strlen(builtin_command[i]));
        write(STDOUT_FILENO, builtin_command_desc[i], strlen(builtin_command_desc[i]));
        write(STDOUT_FILENO, "\n", strlen("\n"));
    }
    flag = 1;
    return;
}

void command_history(char*tokens[],_Bool* in_background){
    int flag_history = 0;
    char temp[COMMAND_LENGTH];
    strcpy(temp,tokens[0]);
  
    if (strncmp(temp, "!",1 )== 0){
        flag_history=1;
    }
    // contancate tokens into string
    for (int i = 0; (tokens[i]!=NULL); i++){
        if (tokens[i+1]!= NULL){
            strcat(temp," ");
            strcat(temp, tokens[i+1]);
        }
    }
    
    if (*in_background == 1){
        strcat(temp," ");
        strcat(temp,"&");
    }
    
    hist = command;
    if (flag_history==0){
        strcpy (history[command],temp);
    }
    else {
        strcpy(history[command],history[command-1]);
        command--;
        flag_history =0;
    }
    return;
}
//split command into tokens
void command_split(char* tokens[], char *history){
    int n = 0;
    int cent = 0;
    
    for (int i = 0; i<strlen(history); i++){
        if (history[i] == ' '||history[i] == '\0'){
            temp[cent][n] = '\0';
            cent ++;
            n = 0;
        }
        else {
            temp[cent][n] = history[i];
            n++;
        }
    }
    for (int z = 0; z< (cent+1); z++){
        tokens[z] = temp[z];
    }
    for (int y = (cent+1); tokens[y]!= NULL; y++){
        if (tokens[y]!=NULL){
            tokens[y] = NULL;
        }
    }
    return;
}
/**
 * Main and Execute Commands
 */
int main(int argc, char* argv[])
{
	/* Set up the signal handler */
	struct sigaction handler;
	handler.sa_handler = handle_SIGINT;
	handler.sa_flags= 0;
	sigemptyset(&handler.sa_mask);
	sigaction(SIGINT, &handler, NULL);
	


	char input_buffer[COMMAND_LENGTH];
	char *tokens[NUM_TOKENS];
	while (true) {
		char cwd_buffer[500];
		char *cwd = getcwd(cwd_buffer, 500);
    signal(SIGINT,help);

		// Ctrl-C
		if(flag)
		{ 
  
        	flag = 0;
        }
		if(cwd!=NULL){
			write(STDOUT_FILENO, strcat(cwd,"$ "), strlen( strcat(cwd,"$ ")));
		}
		else{
			perror("getcwd() error");
       		return 1;
		}

		// Get command
		// Use write because we need to use read() to work with
		// signals, and read() is incompatible with printf().
		// write(STDOUT_FILENO, "$ ", strlen("$ "));
		_Bool in_background = false;
		read_command(input_buffer, tokens, &in_background);
		
		
		
		    //fork returns twice, "0" for child process. "child's pid for parent process"
		// DEBUG: Dump out arguments: this is for debug purpose to test if tokenize work or not
		//need to delete later
		// char buf[MAX_DIR];
		// for (int i = 0; tokens[i] != NULL; i++) {
		// 	write(STDOUT_FILENO, "   Token: ", strlen("   Token: "));
		// 	write(STDOUT_FILENO, tokens[i], strlen(tokens[i]));
		// 	write(STDOUT_FILENO, "\n", strlen("\n"));
		// }
		

		if (in_background) {
            write(STDOUT_FILENO, "Run in background.", strlen("Run in background."));
		}

		//write(STDOUT_FILENO, "before entering checking", strlen("before entering checking"));
		//write(STDOUT_FILENO, "\n", strlen("\n"));
		check_command(tokens, input_buffer,&in_background);
		//DEBUG end 

		
		/**
		 * Steps For Basic Shell:
		 * 1. Fork a child process
		 * 2. Child process invokes execvp() using results in token array.
		 * 3. If in_background is false, parent waits for
		 *    child to finish. Otherwise, parent loops back to
		 *    read_command() again immediately.
		 */

	}
	return 0;
}
