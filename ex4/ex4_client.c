// Itay Sova  (removed)
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <dirent.h>
#include <ctype.h>
#include <time.h>

// this is the client side

char self_pid[100] = {'\0'};

// function decleration:
int find_txt_file(char* file_name);
void read_ans_file(int conf_fd);
void write_query_to_srv(char* srv_pid, char* first_num, char* operation, char* sec_num);
void signal_hand (int sig);
int input_check(int argc,char **argv);

// function for finding if a file exist in current dir
// if so, it returns 1. otherwise 0
int find_txt_file(char* file_name){
    DIR *pDir;
    struct dirent *pDirent;
    if ( (pDir = opendir(".")) == NULL){
        //perror("opendir failed");
        printf("ERROR_FROM_EX4\n");
        exit(-1);
    }
    while ( (pDirent = readdir(pDir) ) != NULL ) {
        if(strcmp(pDirent->d_name,file_name) == 0){
            if(closedir(pDir) < 0){
                //perror("Error in: close");
                printf("ERROR_FROM_EX4\n");
                exit(-1);
            }
            return 1;
        }
    }
    if(closedir(pDir) < 0){
        printf("ERROR_FROM_EX4\n");
        //perror("Error in: close");
        exit(-1);
    }
    return 0;
}
// function for reading the file returned from the server:
void read_ans_file(int conf_fd) {
    char buffer[101]={'\0'};
    char* answer;
    int bytes_read_from_conf;
    bytes_read_from_conf = read(conf_fd,buffer,100);
    if(bytes_read_from_conf == -1) {
//        perror("Error in: read");
        printf("ERROR_FROM_EX4\n");
        close(conf_fd);
        exit(-1);
    }
    answer = buffer;
    if (strcmp(answer,"CANNOT_DIVIDE_BY_ZERO\n") == 0){
        printf("CANNOT_DIVIDE_BY_ZERO\n");
        close(conf_fd);
    } else {
        printf("%s\n",answer);
        close(conf_fd);
    }
}

void write_query_to_srv(char* srv_pid, char* first_num, char* operation, char* sec_num){
    int query_fd = open("to_srv.txt", O_RDWR|O_CREAT|O_TRUNC|O_APPEND, 0777);
    int self_int_pid = getpid();
    char pid_for_file[100];
    sprintf(pid_for_file, "%d", self_int_pid);
    strcat(self_pid, pid_for_file);
    write(query_fd,self_pid,strlen(self_pid));
    write(query_fd,"\n",strlen("\n"));
    write(query_fd,first_num,strlen(first_num));
    write(query_fd,"\n",strlen("\n"));
    write(query_fd,operation,strlen(operation));
    write(query_fd,"\n",strlen("\n"));
    write(query_fd,sec_num,strlen(sec_num));
    close(query_fd);
    int pid_srv = atoi(srv_pid);
    if (kill(pid_srv,SIGUSR1) != 0){
        // if the client cant signal the server exit with error
        printf("ERROR_FROM_EX4\n");
        exit(-1);
    }
}


void alarm_hand (int sig)
{
    signal(SIGALRM, alarm_hand);
    printf("\nClient closed because no response was received from the server for 30 seconds\n");
    exit(-1);
}
// signal for the answer from the server
void signal_hand (int sig)
{
    signal(SIGUSR1, signal_hand);
    // getting the pid of self
    char ans_file_name[100] = {'\0'};
    strcat(ans_file_name,"to_client_");
    strcat(ans_file_name,self_pid);
    strcat(ans_file_name, ".txt");
    int answer_fd = open(ans_file_name, O_RDONLY);
    if (answer_fd < 0){
        printf("ERROR_FROM_EX4\n");
        exit(-1);
    }
    read_ans_file(answer_fd);
    // delete client ans file
    if(remove(ans_file_name) == -1){
        printf("ERROR_FROM_EX4\n");
//        perror("Error in: remove");
        exit(-1);
    }
    exit(0);
}

// input check return 1 if the input is valid and 0 if not
int input_check(int argc,char **argv){
    // checking all the input to be int type numbers:
    int arg_index = 1;
    // for loop for all args
    for (;arg_index < argc ; arg_index++){
        char input[100] = "";
        int length,i;
        strcpy(input, argv[arg_index]);
        length = strlen (input);
        // check if the number is 0
        if (atoi(input)==0){
            // if the input in atoi is 0 check if it is the number 0 or that its not a number at all
            if (input[0] == '0'){
                // continue to the next arg
                continue;
            } else {
                // if atoi(input)==0 and input[0] != '0' then the arg is not a number
                return 0;
            }
        }
        for (i=0;i<length; i++){
            //
            if (!isdigit(input[i])){
                if (i == 0 && input[i] == '-' ){
                    // if the 1st char is not a digit it may be a "-"
                    continue;
                }
                return 0;
            }
        }
    }
    // checking that p3 to be an operator between 1-4:
    int operator_i = atoi(argv[3]);
    if (operator_i > 4 || operator_i < 1){
        return 0;
    }
    return 1;
}


int main(int argc,char **argv) {
    if(argc < 5){
        printf("ERROR_FROM_EX4\n");
        exit(-1);
    }
    // input check
    if (input_check(argc,argv) != 1){
        printf("ERROR_FROM_EX4\n");
        exit(-1);
    }
    // time for random
    srand(time(NULL));
    int time_to_wait;
    // declaring signals:
    signal(SIGUSR1, signal_hand);
    signal(SIGALRM, alarm_hand);
    // check if the to_srv file exist and if so wait random num of seconds and check again
    // after 10 times it print error massage and exit
    int num_of_times_in_loop = 0;
    while (find_txt_file("to_srv.txt") == 1){
        num_of_times_in_loop +=1;
        time_to_wait = rand() % 6;
        sleep(time_to_wait);
        if (num_of_times_in_loop == 10){
            printf("ERROR_FROM_EX4\n");
            exit(-1);
        }
    }
    write_query_to_srv(argv[1],argv[2],argv[3],argv[4]);
    alarm(30); // alarm for 30 sec for serv answer
    pause(); // pause until ans file is ready
    return 0;
}
