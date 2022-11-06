// Itay Sova  (removed)
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/wait.h>
// global pid
pid_t glob_pid;
// the server side:
//global name for the query file name:
char* name_or_query_file = "to_srv.txt";
// function declaration:
void activate_child(pid_t *pid_c);
int compute_query(int first_num, int operation, int sec_num, char* answer_buff);
void print_ans_to_file(int pid, char* ans);
void read_query_file();
void signal_hand (int sig);
void alarm_hand (int sig);
void manual_end_hand (int sig);
void activate_child(pid_t *pid_c);

// functions implementation:
void activate_child(pid_t *pid_c){
    if((*pid_c = fork()) == -1){
        printf("ERROR_FROM_EX4\n");
    } else if(*pid_c == 0 ){
        //child process
        // giving the entire service
        read_query_file();
        //killing child process
        raise(SIGUSR2);
    }
}

int compute_query(int first_num, int operation, int sec_num, char* answer_buff){
    int answer;
    switch (operation) {
        case 1:
            answer = first_num + sec_num;
            break;
        case 2:
            answer = first_num - sec_num;
            break;
        case 3:
            answer = first_num * sec_num;
            break;
        case 4:
            if (sec_num == 0){
                strcat(answer_buff,"CANNOT_DIVIDE_BY_ZERO\n");
                return -1;
            }
            answer = first_num / sec_num;
            break;
    }
    char final_ans[100];
    sprintf(final_ans, "%d", answer);
    strcat(answer_buff, final_ans);
    return 0;
}

void print_ans_to_file(int pid, char* ans){
    char pid_in_char[50] = {'\0'};
    sprintf(pid_in_char, "%d", pid);
    char new_ans_file_name[100] = {'\0'};
    strcat(new_ans_file_name,"to_client_");
    strcat(new_ans_file_name,pid_in_char);
    strcat(new_ans_file_name,".txt");
    int ans_fd = open(new_ans_file_name, O_RDWR|O_CREAT|O_TRUNC|O_APPEND, 0777);
    write(ans_fd,ans,strlen(ans));
    close(ans_fd);
}
// the function of the server which does the reading of the srv file, computation and writing the answer
// to the client (when getting a signal from the client that func will be activated in fork)
void read_query_file() {
    char buffer[2] = {'\0'};
    int line = 1;
    char ans[100] = {'\0'};
    int query_fd = open("to_srv.txt", O_RDWR);
    if (query_fd < 0) {
//        perror("Error in: open");
        printf("ERROR_FROM_EX4\n");
        exit(-1);
    }
    int bytes_read_from_query = read(query_fd, buffer, 1);
    if (bytes_read_from_query < 0) {
//        perror("Error in: read");
        printf("ERROR_FROM_EX4\n");
        close(query_fd);
        exit(-1);
    }
    char client_pid[100] = {'\0'};
    char first_num[100] = {'\0'};
    char operation[100] = {'\0'};
    char sec_num[100] = {'\0'};
    while (bytes_read_from_query != 0) {
        // if the current char is white space go to next line
        if (isspace(buffer[0]) != 0) {
            line += 1;
        } else {
            if (line == 1) {
                strcat(client_pid, buffer);
            } else if (line == 2) {
                strcat(first_num, buffer);
            } else if (line == 3) {
                strcat(operation, buffer);
            } else if (line == 4) {
                strcat(sec_num, buffer);
            }
        }
        // read the next byte
        bytes_read_from_query = read(query_fd, buffer, 1);
        if (bytes_read_from_query == -1) {
//            perror("Error in: read");
            printf("ERROR_FROM_EX4\n");
            close(query_fd);
            exit(-1);
        }
    }
    close(query_fd);
    // delete to_srv
    if(remove("to_srv.txt") == -1){
//        perror("Error in: remove");
        printf("ERROR_FROM_EX4\n");
        exit(-1);
    }
    // convert to ints:
    int pid = atoi(client_pid);
    int number_1 = atoi(first_num);
    int number_2 = atoi(sec_num);
    int oper = atoi(operation);
    // compute the query and write it into a file
    compute_query(number_1,oper,number_2, ans);
    print_ans_to_file(pid,ans);
    // signal for the client to notify ans is ready
    if (kill(pid,SIGUSR1) != 0){
//      if failed to send signal to client - exit
        printf("ERROR_FROM_EX4\n");
        exit(-1);
    }
}

void signal_hand (int sig)
{
    signal(SIGUSR1, signal_hand);
    // function that fork a child and give service to the client
    activate_child(&glob_pid);
}

void alarm_hand (int sig)
{
    signal(SIGALRM, alarm_hand);
    printf("The server closed because no service request was received for the last 60 seconds\n");
    while (wait(NULL) != -1);
    exit(-1);
}

// signal for manually killing the child
void manual_end_hand (int sig)
{
    signal(SIGUSR2, manual_end_hand);
    while (wait(NULL) != -1);
    exit(-1);
}

int main() {
    signal(SIGUSR1, signal_hand);
    signal(SIGALRM, alarm_hand);
    signal(SIGUSR2, manual_end_hand);
    int num_of_clients_max = 10, num_of_clients_so_far = 0;
    while(1){
        alarm(60);
        pause();
    }
    while (wait(NULL) != -1);
    return 0;
}
