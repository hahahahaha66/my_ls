#include<linux/limits.h>
#include<pwd.h>
#include<getopt.h>
#include<bits/getopt_ext.h>
#include<stdio.h>
#include<unistd.h>
#include<dirent.h>
#include<string.h>
#include<sys/stat.h>
#include<time.h>
#include<stdlib.h>

#define _GNU_SOURCE
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
        printf("%s  ",dir->d_name);
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
void printf_mode(mode_t st_mode ){
    char temp[10];
   sprintf(temp, "%c%c%c%c%c%c%c%c%c",(st_mode&S_IRUSR)?'r':'-',(st_mode&S_IWUSR)?'w':'-',
   (st_mode&S_IXUSR)?'x':'-',(st_mode&S_IRGRP)?'r':'-',(st_mode&S_IWGRP)?'w':'-',
   (st_mode&S_IRGRP)?'x':'-',(st_mode&S_IROTH)?'r':'-',(st_mode&S_IWOTH)?'w':'-',
   (st_mode&S_IXOTH)?'x':'-');
   temp[9]='\0';
   printf("%s ",temp);
}
void list_directory_information(const char*path){
    DIR *p;
    p=opendir(path);
    struct stat sta;
    struct dirent* dir;
    struct passwd* passwd1;
    struct passwd* passwd2;
    while((dir=readdir(p))!=NULL){
        stat(dir->d_name,&sta);
        passwd1=getpwuid(sta.st_uid);
        passwd2=getpwuid(sta.st_gid);
        printf("%-8s ",dir->d_name);
        printf_mode(sta.st_mode);
        printf("%ld ",sta.st_nlink);
        printf("%s ",passwd1->pw_name);
        printf("%s ",passwd2->pw_name);
        printf("%ld ",sta.st_size);
        printf("%s",ctime(&(sta.st_mtime)));
        //printf("\n");
    }
    closedir(p);
}
void list_directory_recursion(const char*path){
    char ful_path[1024];
    printf("%s:\n",path);
    list_directory(path);
    DIR *p;
    p=opendir(path);
    struct dirent* dir;
    struct stat sta;
    printf("\n");
    while((dir=readdir(p))!=NULL){
        if(dir->d_name[0]=='.')
        continue;
        snprintf(ful_path,sizeof(ful_path),"%s/%s",path,dir->d_name);
        stat(ful_path,&sta);
        if(S_ISDIR(sta.st_mode)){
            list_directory_recursion(ful_path);
            printf("\n");
        }
    }
    closedir(p);
}
int main(int argc,char*argv[]){
    char buf[PATH_MAX]={0};
    int opt;
    if(argc==1){
        getcwd(buf,sizeof(buf));
        list_directory( buf);
    }
    while((opt=getopt(argc,argv,"alR"))!=-1){
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
            case'l':
            if(optind<argc&&argv[optind][0]!='-'){
                strncpy(buf,argv[optind],PATH_MAX-1);
                buf[PATH_MAX-1]='\0';
                optind++;
            }
            else{
                getcwd(buf,sizeof(buf));
            }
            list_directory_information(buf);
            break;
            case'R':
            if(optind<argc&&argv[optind][0]!='-'){
                strncpy(buf,argv[optind],PATH_MAX-1);
                buf[PATH_MAX-1]='\0';
                optind++;
            }
            else{
                getcwd(buf,sizeof(buf));
            }
            list_directory_recursion(buf);
            break;
        }
    }
    return 0;
}