#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include<sys/stat.h>

int get_digit_count(int num){
    if(num==0)return 1;
    int count=0;
    while(num!=0){
        num/=10;
        count++;
    }
    return count;
}
int main(){
    printf("%d\n",get_digit_count(100));
    return 0;
}