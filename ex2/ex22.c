// Itay Sova 31326069
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>


// functions decleration:
void readlines(int conf_fd,char* files_path, char* input_path, char* output_path);
char* find_c_filename(char* dir);
int compile(char* c_file, int errors_fd);
int compare(char* output_path,char* output_c_fd);
int execute(int input_fd,int output_c_fd,int errors_fd);
// functions implementation:
//This function reads the configuration file and saves each 3 rows in the given variables(files_path, input_path, output_path)
//On failure of read it exits and returns -1
void readlines(int conf_fd,char* files_path, char* input_path, char* output_path) {
    char buffer[2]={'\0'};
    int line = 1;
    int bytes_read_from_conf;
    bytes_read_from_conf = read(conf_fd,buffer,1);
    if(bytes_read_from_conf == -1) {
            perror("Error in: read");
            exit(-1);
        }
     while(bytes_read_from_conf != 0){
        if(buffer[0] =='\n') {
            line +=1;
        } else {
            if(line == 1){
                strcat(files_path,buffer);
            } else if(line == 2) {
                strcat(input_path,buffer);
            } else if(line == 3) {
                strcat(output_path,buffer);
            }
        }
        bytes_read_from_conf = read(conf_fd,buffer,1);
        if(bytes_read_from_conf == -1) {
            perror("Error in: read");
            exit(-1);
        }
    }

}
// function that get a path to a dir, in which to look for a c file. the function will return the name of the file
// or "not found" if there is mo c file
char* find_c_filename(char* dir) {
    DIR *pDir;
    struct dirent *pDirent;
    struct stat file_stat;
	pDir = opendir(dir);
	if(pDir == NULL){
		perror("Error in: opendir");
		return "Error in: opendir";
	}
    if(chdir(dir) < 0){
            perror("chdir failed");
            }
	while((pDirent = readdir(pDir)) != NULL) {
        if(stat(pDirent ->d_name, &file_stat) == -1) {
            perror("stat");
            exit(-1);
            }
        if(S_ISREG(file_stat.st_mode)){
            char* extension = strrchr(pDirent->d_name, '.');
            if(extension != NULL && extension[1] =='c') {
                closedir(pDir);
                if(chdir("..")<0){
                    perror("chdir failed");
                    }
                return pDirent->d_name;
                }
            } 
    }

    closedir(pDir);
    if(chdir("..")<0){
        perror("chdir failed");
        }
    return "Not found";
}
// function that gets the c file name and error text file descriptor and compile the file in the local dir
int compile(char* c_file, int errors_fd){
    char *argsv[] = {"gcc", "-o", "c.out", c_file,NULL};
    int ret_code;
    int ret_duperror;
    int pid,stat;
    setbuf(stdout, NULL);  
    if(pid=fork()==0 && c_file != "Not found") {   
        ret_duperror=dup2(errors_fd,2);
        close(errors_fd);
        int ret_code=execvp("gcc", argsv);
        if(ret_code==-1) {
            perror("Error in: exec");
            return -1;
        }
     } else {
                waitpid(pid,&stat,0);
                return WEXITSTATUS(stat);
            }
}
// function for comparinf the results of the execute func that returns the result of the comparison
int compare(char* output_path,char* output_c_fd){
    char *argsv[] = {"./comp.out",output_path,output_c_fd,NULL};
    int ret_code;
    int pid,stat;
    setbuf(stdout, NULL);
    if((pid=fork())==0) {
       ret_code=execv(argsv[0],argsv);
       if(ret_code==-1) {
            perror("Error in: exec");
            return -1;
        }
    } else {
        waitpid(pid,&stat,0);
        return WEXITSTATUS(stat);
    }
}
// function for execute of c.out files. return 0 if successful or -1 if exec failed
int execute(int input_fd,int output_c_fd,int errors_fd) {
    int ret_code;
    int ret_dupout;
    int ret_dupin;
    int ret_duperror;
    int pid,stat;
    setbuf(stdout, NULL);
    if(pid=fork()==0) {
        ret_dupout=dup2(output_c_fd, 1);
        ret_dupin=dup2(input_fd, 0);
        ret_duperror=dup2(errors_fd,2);
        close(errors_fd);
        close(output_c_fd);
        close(input_fd);
        int ret_code=execlp("./c.out","./c.out",NULL);           
        if(ret_code==-1) {
            perror("Error in: exec");
            return -1;
        }
        } else {
            waitpid(pid,&stat,0);
            return 0;
        }
    }



int main(int argc,char **argv){
    //reading the conf file and deviding its content to its component
    int conf_fd = open(argv[1],O_RDONLY);
    char cwd[256]; // saving the primary cwd
    char students_dir[256];
    char* c_File;
    if (getcwd(cwd, sizeof(cwd)) == NULL)
      perror("getcwd() error");
    int wstatus=0;
    char files_path[150]={'\0'}; 
    char input_path[150]={'\0'};
    char output_path[150]={'\0'};
    readlines(conf_fd, files_path, input_path, output_path);
    //a file descriptor for the input file given
    int input_fd = open(input_path, O_RDONLY);
    if(input_fd < 0) {
        perror("Input file not exist\n");       
        exit(-1);
    }
    // opening a file for the output a c file of the student
    int output_c_fd = open("outputc.txt", O_RDWR|O_CREAT|O_TRUNC|O_APPEND, 0777);
    if(output_c_fd < 0) {
        perror("Error in: open");
        exit(-1);
    }
    // a file descriptor for the correct output to compare (got the file from the conf.txt)
    int correct_output_fd = open(output_path, O_RDONLY);
    if(correct_output_fd < 0) {
        perror("Output file not exist\n");
        exit(-1);
    }
    // opening a file for the results (in csv format)
    int results_fd = open("results.csv", O_RDWR|O_CREAT|O_TRUNC|O_APPEND, 0777);
    if(results_fd < 0) {
        perror("Error in: open");
        exit(-1);
    }
    // opening a file for errors
    int errors_fd = open("errors.txt", O_RDWR|O_CREAT|O_TRUNC|O_APPEND, 0777);
    if(errors_fd < 0) {
        perror("Error in: open");
        exit(-1);
    }
    // checking if the input path is a dir:
    struct stat sb;
    struct stat files_dir;
    if(stat(files_path, &sb) == -1) {
        perror("Not a valid directory\n");
        exit(-1);
    }
    if(!S_ISDIR(sb.st_mode)){
        perror("Not a valid directory\n");
        exit(-1);
    }
    //if the path is a valid dir change to it
    if(chdir(files_path) < 0){
    perror("chdir failed");
    }
    // settind students dir
    getcwd(students_dir,sizeof(students_dir));
    // then, opening the current dir and checking all of its content to be dirs
    DIR *pDir; 
    struct dirent *pDirent; 
    if ( (pDir = opendir(".")) == NULL){
        perror("opendir failed");
        exit(1);
    } 
    // looping through the directory, printing the directory entry name
    while ( (pDirent = readdir(pDir) ) != NULL ) {
        if(stat(pDirent ->d_name, &files_dir) == -1) {
            perror("stat");
            exit(-1);
            }
        if(S_ISDIR(files_dir.st_mode) && strcmp(pDirent->d_name,".") != 0  && strcmp(pDirent->d_name,"..") != 0 ){
            c_File = find_c_filename(pDirent->d_name);
            //change dir to compile the last file found
            if(chdir(pDirent ->d_name)<0){
                perror("chdir failed");
                }
            if(c_File != "Not found"){
                // activate the function compile
                int compile_result = compile(c_File,errors_fd);
                // if there is a compilation error of a file
                if(compile_result == 1){
                    if(write(results_fd,pDirent->d_name,strlen(pDirent->d_name)) < 0) {
                        continue;
                    }
                    char* compile_error = ",10,COMPILATION_ERROR\n";
                    if(write(results_fd,compile_error,strlen(compile_error)) < 0) {
                        continue;
                    }
                } else if(compile_result == 0){
                    // execute if the compile func returned 1 then the compilation was successful
                    int execute_result = execute(input_fd, output_c_fd, errors_fd);
                    if(execute_result == -1) {
                        continue;
                    }
                    // change to main dir to compare the current output file to the given correct output
                    if(chdir(cwd)==-1) {
                        perror("Error in: chdir");
                        exit(-1);
                        }
                    int compare_result = compare(output_path, "outputc.txt");
                    if(compare_result == -1) {
                        continue;
                        }
                    if(chdir(students_dir)==-1) {
                        perror("Error in: chdir");
                        exit(-1);
                        }
                    if(chdir(pDirent->d_name)==-1) {
                        perror("Error in: chdir");
                        exit(-1);
                        }
                    // delete the c.out file if exists
                    if(remove("c.out") == -1){
                        perror("Error in: remove");
                        exit(-1);
                        }
                    // since the compare function uses ex21, the func returns value from (1,2,3)
                    // check the result and update the output file accordingly
                    if(compare_result == 1){
                        char* excellent =strcat(pDirent->d_name,",100,EXCELLENT\n");
                        if(write(results_fd,excellent,strlen(excellent)) < 0){
                            continue;
                            }
                        }
                    else if(compare_result == 2){
                        char* wrong =strcat(pDirent->d_name,",50,WRONG\n");
                        if(write(results_fd,wrong,strlen(wrong)) < 0){
                            continue;
                            }
                        }
                    else if(compare_result == 3) {
                        char* similar = strcat(pDirent->d_name,",75,SIMILAR\n");
                        if(write(results_fd,similar,strlen(similar)) < 0 ){
                            continue;
                            }
                        }
                }
            }else{
                wait(&wstatus); 
                char* no_c_file_error = strcat(pDirent->d_name,",0,NO_C_FILE\n");
                if(write(results_fd,no_c_file_error,strlen(no_c_file_error)) < 0){
                    continue;
               }
            }
            // go to primary dir to reset files we closed:
            if(chdir(cwd)==-1) {
                perror("Error in: chdir");
                exit(-1);
            }
            input_fd = open(input_path, O_RDONLY);
            if(input_fd < 0) {
                perror("Input file not exist\n");       
                exit(-1);
                }
            output_c_fd = open("outputc.txt", O_RDWR|O_CREAT|O_TRUNC|O_APPEND, 0777);
            if(output_c_fd < 0) {
                 perror("Error in: open");
                 exit(-1);
                 }
            correct_output_fd = open(output_path, O_RDONLY);
            if(correct_output_fd < 0) {
                perror("Output file not exist\n");
                exit(-1);
                }
            //change dir back to student dir scanning the students dir
            if(chdir(students_dir) < 0){
                perror("chdir failed");
                }
            }
        }
        // closing all the dirs and files that the program opened
        if(closedir(pDir) < 0){
            perror("Error in: close");
            exit(-1);
            }
        if(close(input_fd) < 0){
            perror("Error in: close");
            exit(-1);
            }
        if(close(output_c_fd) < 0){
            perror("Error in: close");
            exit(-1);
            }
        if(close(errors_fd) < 0){
            perror("Error in: close");
            exit(-1);
            }
        if(close(conf_fd) < 0){
            perror("Error in: close");
            exit(-1);
            }
        if(close(correct_output_fd) < 0){
            perror("Error in: close");
            exit(-1);
            }
        if(close(results_fd) < 0){
            perror("Error in: close");
            exit(-1);
            }
        // change to the original path to delete files created there
        if(chdir(cwd)==-1) {
            perror("Error in: chdir");
            exit(-1);
            }
        // delete the output file
        if(remove("outputc.txt") == -1){
            perror("Error in: remove");
            exit(-1);
            }
    return 0;    
}
