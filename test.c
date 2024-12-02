#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include<sys/stat.h>

int main(){
    struct stat a;
    stat("test.c",&a);
    printf("%d",a.st_uid);
    return 0;
}