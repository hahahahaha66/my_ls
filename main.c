#include <linux/limits.h>
#define _GNU_SOURCE
#include<getopt.h>
#include<bits/getopt_ext.h>
#include<stdio.h>
#include<unistd.h>
#include<dirent.h>
#include<string.h>
//no_argument：选项不带参数。
//required_argument：选项必须带参数。
//optional_argument：选项参数可选。
void list_directory(const char*path){
    DIR *p;
    p=opendir(path);
    struct dirent* dir;
    while((dir=readdir(p))!=NULL){
        if(dir->d_name[0]=='.')
        continue;
        printf("%s ",dir->d_name);
    }
    printf("\n");
    closedir(p);
}
void list_all_directory(const char*path){
    DIR *p;
    p=opendir(path);
    struct dirent* dir;
    while((dir=readdir(p))!=NULL){
        printf("%s ",dir->d_name);
    }
    printf("\n");
    closedir(p);
}
int main(int argc,char*argv[]){
    char buf[PATH_MAX]={0};
    int opt;
    if(argc==1){
        getcwd(buf,sizeof(buf));
        list_directory( buf);
    }
    while((opt=getopt(argc,argv,"al"))!=-1){
        switch(opt){
            case'a':
            if(optind<argc&&argv[optind][0]!='-'){
                strncpy(buf,argv[optind],PATH_MAX-1);
                buf[PATH_MAX-1]='\0';
                optind++;
            }
            else{
                getcwd(buf,sizeof(buf));
            }
            list_all_directory(buf);
            break;
        }
    }
    return 0;
}