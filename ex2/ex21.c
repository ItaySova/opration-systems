// Itay Sova (removed)
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>

// in the main function if the files are similar or their content is the same the function
// will return 1 or 3. the default value the program return is 2 (which means that the files are different).
int main(int argc,char **argv){
    //input check - if there are more or less than 3 arguments (argv[0] - the name of the program and the other 2)
    // we return -1
    if(argc != 3){
        perror("\ninput error\n");
        return -1;
    }
    // init buffer size
    int global_status = 1;
    int fd_1 = open(argv[1],O_RDONLY);
    if(fd_1 < 0){
        perror("Error in: open");
        exit(-1);
    }
    int fd_2 = open(argv[2],O_RDONLY);
    if(fd_2 < 0){
        perror("Error in: open");
        exit(-1);
    }
    // new version (working one):
    char char_fd1;
    char char_fd2;
    ssize_t x1;
    ssize_t x2;
    // get the first char from fd1
    x1 = read (fd_1, &char_fd1, 1); 
    if(x1 < 0){
        perror("Error in: read");
        exit(-1);
    }
    // get the first char from fd2
    x2 = read (fd_2, &char_fd2, 1);
    if(x2 < 0){
        perror("Error in: read");
        exit(-1);
    }
    if(x1 == 0 || x2 == 0){
        if (x1 == 0 && x2 == 0){
            // if both are empty then the content is identical (identical empty)
            return 1;
        }else{
            // if one file is empty and the other is not then their content is different
            return 2;
        }
    }
    // init two variables that will count the number of " ","\n" and "\r" chars with each iteration of the loop for each file and 
    // compare them in order to find if files are similar via chars that we skip in the proccess of comparing chars
    int white_chars_fd1=0,white_chars_fd2=0;
    while(1){
        // if the char is a space,\n or \r - read the next char 
        while (isspace(char_fd1) != 0){
            x1 = read (fd_1, &char_fd1, 1);
            if(x1 < 0){
                perror("Error in: read");
                exit(-1);
            }
            white_chars_fd1++;
            if (x1 == 0){
                break;
            }
        }
        // if the char is a space,\n or \r - read the next char
        while (isspace(char_fd2) != 0){
            x2 = read (fd_2, &char_fd2, 1);
            if(x2 < 0){
                perror("Error in: read");
                exit(-1);
            }
            white_chars_fd2++;
            if (x2 == 0){
                break;
            }
        }
        // if the number of spaces skiped in the last 2 loops is different between the files then the files are at least similar but not
        // identical
        if(white_chars_fd1 != white_chars_fd2 && global_status == 1){
            global_status = 3;
        }
        // init both white_chars_fd1,white_chars_fd2 back to 0 for the next iteration of the loop
        white_chars_fd1 = 0;
        white_chars_fd2 = 0;
        // in case one of the files was read to its end:
        if(x1 == 0 || x2 == 0){
            if(x1 == 0 && x2 == 0){
                // both files are done
                return global_status;
            } else {
                // onlu one file is done thus the txts are different
                return 2;
            }
        }
        // comparing the chars from that iteration:
        if(char_fd1 != char_fd2){
            // checking if similar
            if(tolower(char_fd1) == tolower(char_fd2)){
                global_status = 3;
            } else {
            //if different return 2
            return 2;
            }
        }
        // reading the next char from both fd's
        // get the next char from fd1
        x1 = read (fd_1, &char_fd1, 1);
        if(x2 < 0){
            perror("Error in: read");
            exit(-1);
           }
        x2 = read (fd_2, &char_fd2, 1);
        if(x2 < 0){
            perror("Error in: read");
            exit(-1);
           }
        // if one of the files was read entirely
        if(x1 == 0 || x2 == 0){
            if(x1 == 0 && x2 == 0){
                // both files are done
                return global_status;
            }else if(x1 == 0 && x2 != 0){
                // if file 2 is not finished we keep reading
                while (isspace(char_fd2) != 0){
                    x2 = read (fd_2, &char_fd2, 2);
                    if(x2 < 0){
                        perror("Error in: read");
                        exit(-1);
                    }
                    if (x2 == 0){
                        // if x2 == 0 file 2 is done reading and there is no actual chars
                        return 3;
                    }
                }
                // onlu one file is done thus check if there are actual char in the other left
                return 2;
            }else if(x1 != 0 && x2 == 0){
                // if file 1 is not finished we keep reading
                while (isspace(char_fd1) != 0){
                    x1 = read (fd_1, &char_fd1, 2);
                    if(x1 < 0){
                        perror("Error in: read");
                        exit(-1);
                    }
                    if (x1 == 0){
                        // if x1 == 0 file 2 is done reading and there is no actual chars
                        return 3;
                    }
                }
                // onlu one file is done thus check if there are actual char in the other left
                return 2;
            }
        }
    }

    return global_status;
}
