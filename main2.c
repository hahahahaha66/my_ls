#include <asm-generic/ioctls.h>
#include<pwd.h>
#include<fcntl.h>
#include<termios.h>
#include<time.h>
#include<stdio.h>
#include"uthash.h"
#include<stdlib.h>
#include<getopt.h>
#include<unistd.h>
#include<dirent.h>
#include<string.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/ioctl.h>
#include<stdbool.h>
#include<linux/limits.h>
#include<bits/getopt_ext.h>
#define MAX_DEPTH 100

#define _GNU_SOURCE
struct haha{
    char* name;
    time_t tim;
};
typedef struct {
    char path[PATH_MAX]; // 记录路径
    ino_t inode;         // 记录 inode
    UT_hash_handle hh;   // 哈希表的句柄
} VisitedPath;
VisitedPath *visited_paths = NULL;

// 检查路径是否已访问
bool is_path_visited(const char *path) {
    VisitedPath *s;
    HASH_FIND_STR(visited_paths, path, s);
    return s != NULL;
}

// 添加路径到集合
void add_path(const char *path,ino_t inode) {
    if (is_path_visited(path)) return;

    VisitedPath *s = malloc(sizeof(VisitedPath));
    if (!s) {
        perror("Failed to allocate memory for hash entry");
        exit(EXIT_FAILURE);
    }
    strncpy(s->path, path, PATH_MAX);
    s->inode=inode;
    HASH_ADD_STR(visited_paths, path, s);
}

// 清理路径集合
void free_visited_paths() {
    VisitedPath *current, *tmp;
    HASH_ITER(hh, visited_paths, current, tmp) {
        HASH_DEL(visited_paths, current);
        free(current);
    }
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

void printf_time(time_t time){
    struct tm* time_info=localtime(&time);
    char tim[100];
    strftime(tim,sizeof(tim),"%m月%d日 %H:%M",time_info);
    printf("%s ",tim);
}

int compare(const void*a,const void*b){
    time_t m=((struct haha*)a)->tim;
    time_t n=((struct haha*)b)->tim;
    return (n>m)-(n<m);
}

void invert(struct haha*haha,int len){
    int left=0;
    int right=len-1;
    while(left<right){
        struct haha*temp=&haha[left];
        haha[left]=haha[right];
        haha[right]=*temp;
        left++;
        right--;
    }
} 

void configure_terminal(int enable){
    struct termios t;
    tcgetattr(STDIN_FILENO,&t);
    if(!enable){
        t.c_lflag &=~(ECHO |ICANON);
    }
    else{
        t.c_lflag |=(ECHO |ICANON);
    }
    tcsetattr(STDIN_FILENO,TCSANOW,&t);
}

int get_now(){
    configure_terminal(0);
    printf("\033[6n");
    fflush(stdout);
    char buf[32];
    int i=0;
    while(1){
        if(read(STDIN_FILENO,&buf[i],1)==-1){
            perror("read");
            configure_terminal(1);
            return 1;
        }
        if(buf[i]=='R') break;
        i++;
    }
    configure_terminal(1);
    char* cleaned_buf = buf;
    while(*cleaned_buf == ' ' || *cleaned_buf == '\n' || *cleaned_buf == '\r') {
        cleaned_buf++;  
    }
    int len = strlen(cleaned_buf);
    while(len > 0 && (cleaned_buf[len - 1] == ' ' || cleaned_buf[len - 1] == '\n' || cleaned_buf[len - 1] == '\r')) {
        cleaned_buf[--len] = '\0';  
    }
    int row,col;
    if(sscanf(buf+2,"%d;%d",&row,&col)==2){
        return col;
    }
    else{
        return -1;
    }
}

void list_directory(const char*path,int*option){
    struct stat sta;
    struct dirent* dir;
    struct passwd* passwd1;
    struct passwd* passwd2;
    struct winsize w;
    struct haha* haha=NULL;
    int countfile=0;
    static int depth=0;
    char full_path[PATH_MAX];
    DIR *p1;
    p1=opendir(path);
    if(!p1){
        perror("opendir");
        return ;
    }
    if(ioctl(STDOUT_FILENO,TIOCGWINSZ,&w)==-1){
        perror("ioctl failed");
        return ;
    }
    char real_path[PATH_MAX];
    if(realpath(path,real_path)==NULL){
        perror("realpath");
        closedir(p1);
        return ;
    }
    if(strcmp(path,"//dev/fd")==0){
        closedir(p1);
        return ;
    }
    // if(strcmp(path,"//sys")==0){
    //     closedir(p1);
    //     return ;
    // }
    // if(strcmp(path,"//proc")==0){
    //     closedir(p1);
    //     return ;
    // }
    if(option[2]){
        printf("%s:\n",path);
    }

    while((dir=readdir(p1))!=NULL){
        if(!option[0]&&(dir->d_name[0]=='.')){
            continue;
        }
        snprintf(full_path, sizeof(full_path), "%s/%s", path, dir->d_name);
        if (lstat(full_path, &sta) == -1) {
            perror("stat failed");
            continue;
        }
        struct haha *new_haha = realloc(haha, (countfile + 1) * sizeof(struct haha));
        if (!new_haha) {
            free(haha);
            perror("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
        haha = new_haha;
        haha[countfile].name=dir->d_name;
        haha[countfile].tim=sta.st_mtime;
        countfile++;
    }
    
    if(option[3]){
        qsort(haha,countfile,sizeof(struct haha),compare);
    }

    if(option[4]){
        invert(haha,countfile);
    }

    for(int i=0;i<countfile;i++){
        if(haha[i].name){
            char a='\t';
            snprintf(full_path, sizeof(full_path), "%s/%s", path, haha[i].name);
            if (lstat(full_path, &sta) == -1) {
                perror("stat failed");
                continue;
            }
            if(option[1]){
                passwd1=getpwuid(sta.st_uid);
                passwd2=getpwuid(sta.st_gid);
                printf_mode(sta.st_mode);
                printf("%ld ",sta.st_nlink);
                printf("%s ",passwd1->pw_name);
                printf("%s ",passwd2->pw_name);
                printf("%-ld ",sta.st_size);
                //printf("%s",ctime(&(sta.st_mtime)));
                printf_time(sta.st_mtime);
                a='\n';
            }
            if(option[6]){
                printf("%ld ",(sta.st_size)/512);
            }
            if(option[5]){
                printf("%ld ",sta.st_ino);
            }
            if(w.ws_col-get_now()-strlen(haha[i].name)<25){
                printf("\n");
            }
            if(sta.st_mode&S_IXUSR){
                if((haha[i].name)[0]=='.'){
                    printf("\033[1;34m%-20s\033[0m%c",haha[i].name,a); 
                }
                else{
                    printf("\033[1;32m%-20s\033[0m%c",haha[i].name,a);
                }
            }
            else{
                printf("%-20s%c",haha[i].name,a);
            }
        }
    }
    if(!option[1]){
        printf("\n");
    }
    free(haha);
        
    if(option[2]){
        DIR *p2=opendir(path);
        if(!p2){
            perror("opendir");
            return ;
        }
        printf("\n");
        while((dir=readdir(p2))!=NULL){
            if(!option[0]){
                if(dir->d_name[0]=='.')
                continue;
            }
            snprintf(full_path,sizeof(full_path),"%s/%s",path,dir->d_name);
            if(stat(full_path,&sta)==-1){
                continue;
            }
            if(S_ISDIR(sta.st_mode)&&(strcmp(dir->d_name, ".") != 0)&&(strcmp(dir->d_name, "..") != 0)){
                // if(strcmp(full_path,"//dev/fd")==0){
                //     continue;
                // }
                if(!is_path_visited(full_path)){
                    add_path(full_path,sta.st_ino);
                    printf("\n");
                    if(depth<MAX_DEPTH){
                        depth++;
                        list_directory(full_path,option);
                    }
                    else{
                        return ;
                    }
                    
                }
            }
        }
    closedir(p2);
    }
    //printf("\n");
    closedir(p1);
    
}
int main(int argc,char*argv[]){
    char buf[PATH_MAX]={0};
    int opt;
    int option[8]={0};
    int temp=0;
    if(argc==1){
        getcwd(buf,sizeof(buf));
        list_directory(buf,option);
    }
    while((opt=getopt(argc,argv,"alRtris"))!=-1){
        switch(opt){
            case'a':
            option[0]=1;
            break;
            case'l':
            option[1]=1;
            break;
            case'R':
            option[2]=1;
            break;
            case't':
            option[3]=1;
            break;
            case'r':
            option[4]=1;
            break;
            case'i':
            option[5]=1;
            break;
            case's':
            option[6]=1;
            break;
        }
    }
   if(optind<argc){
    for(int i=optind;i<argc;i++){
        strncpy(buf,argv[i],PATH_MAX-1);
        buf[PATH_MAX-1]='\0';
        list_directory(buf, option);
        if(i<argc-1){
            printf("\n");
        }
    }
   }
   else{
    getcwd(buf,sizeof(buf));
    list_directory(buf, option);
   }
    free_visited_paths();
    return 0;
}