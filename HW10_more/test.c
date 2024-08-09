#include <stdio.h>
#include <unistd.h>
int globvar = 6;
int main()
{
int var;
pid_t pid;
var = 88;
printf("перед вызовом функции fork\n");
if ((pid = fork()) < 0) {
    perror("fork");
    } 
else if (pid == 0) {
        globvar++;
        var++;
      } else {
      sleep(2);
      }
printf("pid = %ld, globvar = %d, var = %d\n", (long)getpid(), globvar, var);
return 0;
}