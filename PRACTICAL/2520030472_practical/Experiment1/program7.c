#include<stdio.h>
#include<unistd.h>
int main()
{
pid_t pid = fork();
if(pid == 0)
{
printf("Child executing Is...\n");
execl("/bin/Is","Is","-1",NULL);
}
else
{
printf("Parent Waiting...\n");
}
return 0;
}
