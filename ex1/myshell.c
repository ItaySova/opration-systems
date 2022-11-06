// Itay Sova  (removed)
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#define max_commands 100
#define max_input_size 100
int stat,ret_code;

// struct to save a unit of history (command and pid)
struct History {
   char command_history[max_input_size];
   int pid;
};
// declaring the builtin functions:
void cd(char** params);
void exit_shell();
void other_command(char** full_input, pid_t *pid);
void print_history(struct History entire_history[], size_t length);

// implementing the functions:
// The cd function
void cd(char** params){
    if(chdir(params[0]) < 0){
        perror("chdir failed");
    }
}
// exit shell function
void exit_shell(){
    exit(0);
}
// func for forking and executing other commands
void other_command(char** full_input, pid_t *pid){
    if((*pid = fork()) == -1){
        perror("fork failed");
    } else if(*pid == 0 ){
        // the child proccess
        ret_code = execvp(full_input[0],&full_input[0]); 
        // if fails exit with -1
        if(ret_code == -1){
            perror("exec failed");
            exit(-1);
        }
    }else{
        //parent proccess
        waitpid(*pid,NULL,WUNTRACED);
    }
}
// command for printing history
void print_history(struct History entire_history[], size_t length) {
  size_t i;
  for (i = 0; i < length ; i++)
  {
    entire_history[i].command_history[strcspn(entire_history[i].command_history,"\n")] = '\0';
    printf("%d %s\n", entire_history[i].pid, entire_history[i].command_history);

  }
}
// the main
int main(int argc,const char *argv[],char *envp[]) {
    // the init part:
    pid_t pid;
    // updating the path if needed
    int index;
    for(index=1; index<argc; index++){
        char* original_path = getenv("PATH");
        char new_path[1024] = {0};
        strcat(new_path,original_path);
        strcat(new_path,":");
        strcat(new_path,argv[index]);
        setenv("PATH", new_path,1);
    }
    //saving history in array of structs
    struct History entire_history[max_commands];
    // init the number of commands (will be max 100)
    int num_of_commands = 1;
    // init an array for the next user input:
    char input[max_input_size] = {0};
    char tmp_input[max_input_size] = {0}; // for history
    // init a pointer for the tokenizing input
    char* input_token;
    // init the current command
    char* current_command;
    // the final array after tokenizing
    char* input_array[max_input_size] = {0};
    // param array init:
    char* params[max_input_size] = {0};
    // the main loop of the shell execution:
    while(num_of_commands <= 100){
        printf("$ ");
        fflush(stdout);
        // geting the input with fget() function
        fgets(input,max_input_size, stdin);
        //update history command and params:
        memcpy(tmp_input,input,max_input_size);
        memcpy(entire_history[num_of_commands-1].command_history,tmp_input,strlen(tmp_input));
        // proccesing the input to command and an array of input:
        int i=0;
        input_token = strtok(input, " ");
        while(input_token != NULL){
            input_token[strcspn(input_token, "\n")] = 0;
            input_array[i] = input_token;
            if(i==0){
                current_command = input_token;
            }else{
                params[i-1] = input_token;
            }
            input_token = strtok (NULL, " ");
            i++;
        }
        
        // the execute of the command part:
        if(strcmp(current_command,"cd") == 0){
            cd(params);
        }else if(strcmp(current_command,"exit") == 0){
            exit_shell();
        }else if(strcmp(current_command,"history") == 0){
            print_history(entire_history, num_of_commands);
        }else {
            other_command(input_array, &pid);
            entire_history[num_of_commands-1].pid=pid;
        }
        // update the num of commands do far
        num_of_commands += 1;
        // resetting all the data for the next command
        printf("\n");
        current_command = "";
        memset(params, 0, max_input_size*sizeof(*params));
        memset(input_array, 0, max_input_size*sizeof(*input_array));
    }
    return EXIT_SUCCESS;
}
