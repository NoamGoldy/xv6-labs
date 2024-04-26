#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}

void
find(char *path, char* file_name)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;
  char *curr_folder = ".";
  char *parent_folder = "..";

  if((fd = open(path, O_RDONLY)) < 0){
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    fprintf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch(st.type){
  case T_DEVICE:
  case T_FILE:
    printf("%s %d %d %l\n", fmtname(path), st.type, st.ino, st.size);
    break;

  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("ls: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';

    //reading from directory fd
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
    //checking weather it is . or .. directories
       if(strcmp(de.name, curr_folder) == 0 || strcmp(de.name, parent_folder) == 0 || de.inum == 0)
            continue;
        
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if(stat(buf, &st) < 0){
        printf("ls: cannot stat %s\n", buf);
        continue;
      }
      switch (st.type) {
        case (T_FILE):
        case (T_DEVICE):
            if(0 == strcmp(de.name, file_name)){
                printf("%s\n",buf);
            }
            break;
        case (T_DIR):
            find(buf, file_name);
            break;
      }
    }
    break;
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  if(argc == 3){
    find(argv[1], argv[2]);
    exit(0);
  }
  fprintf(2, "Usage: find [folder] [name_if_file]\n");
    exit(1);
}
