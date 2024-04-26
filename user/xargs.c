#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"
#define MAX_LINE_LEN 1000


// input: file descriptor, buffer, max
// output: copies the content of fd to buf until the end of the line
// or until max, return number of chars in line
int parse_file_to_line(int fd, char buf[], int max){
    char c;
    int i,err_flag;
    for(i = 0; i < max; i++){
        err_flag = read(fd, &c, 1);
        if(err_flag < 1){
            return 0;
        }
        buf[i] = c;
        if(c == '\n')
            break;
    }
    buf[++i] = '\0';
    return i;
}


// input: string, pointer to array of strings
// output: number of words, puting in args[i] the ith word in buf, in offset offset
int parse_buf_to_words(char buf[], char* args[], int offset){
    int i=0, idx_in_word=0, num_of_words=offset,curr_word_start_idx=0;
    int buf_len = strlen(buf);
    for(i=0; i <= buf_len; i++){
        if(buf[i] != ' ' && buf[i] != '\n'){
            idx_in_word++;
        }
        else{
            if(buf[i] == ' '){
                buf[i] = '\0';
                args[num_of_words] = buf + curr_word_start_idx;
                curr_word_start_idx = i+1;
                num_of_words++;
            }
            else{
                if(buf[i] == '\n'){
                    buf[i] = '\0';
                    args[num_of_words] = buf + curr_word_start_idx;
                    curr_word_start_idx = i+1;
                    num_of_words++;
                }
            }
        }
    }
    return num_of_words;
}



int 
main(int argc, char *argv[])
{

  if(argc < 2){
    fprintf(2, "Usage: xarg [command [initial-arguments]]\n");
    exit(1);
  }

char buf[MAX_LINE_LEN];
char* a[MAXARG];


int l = 0;
//copying argv to a
for(l = 0; l < argc-1; l++){
    a[l] = argv[l+1];
}
  while(0 < parse_file_to_line(0, buf, MAX_LINE_LEN)){
    parse_buf_to_words(buf, a, l);
    int pid = fork();
    if(pid == 0){
        exec(argv[1], a);
    }
    else {
        wait(0);
    }
  }
    


  exit(0);
}
