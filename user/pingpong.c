#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


int
main(int argc, char *argv[])
{
  if(argc > 1){
    fprintf(2, "Usage: pingpong\n");
    exit(1);
  }
  int p1[2];
  int p2[2];
  pipe(p1);
  pipe(p2);
  int pid = fork();
  if (pid != 0){
    close(p1[0]);
    close(p2[1]);
    write(p1[1], "a", 1);
    char m_p[1];
    read(p2[0], m_p, 1);
    int my_pid_parent = getpid();
    printf("%d: received pong\n", my_pid_parent);
    close(p1[1]);
    close(p2[0]);
    exit(0);
  }
  else{
    close(p2[0]);
    close(p1[1]);
    char m[1];
    read(p1[0], m, 1);
    int my_pid = getpid();
    printf("%d: received ping\n", my_pid);
    write(p2[1],m,1);
    close(p1[0]);
    close(p2[1]);
    exit(0);
  }


  exit(0);
}
