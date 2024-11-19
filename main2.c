#include<pwd.h>
#include<time.h>
#include<stdio.h>
#include<stdlib.h>
#include<getopt.h>
#include<unistd.h>
#include<dirent.h>
#include<string.h>
#include<sys/stat.h>
#include<linux/limits.h>
#include<bits/getopt_ext.h>

#define _GNU_SOURCE
struct haha{
    char* name;
    time_t tim;
};

void printf_mode(mode_t st_mode ){
    char temp[10];
   sprintf(temp, "%c%c%c%c%c%c%c%c%c",(st_mode&S_IRUSR)?'r':'-',(st_mode&S_IWUSR)?'w':'-',
   (st_mode&S_IXUSR)?'x':'-',(st_mode&S_IRGRP)?'r':'-',(st_mode&S_IWGRP)?'w':'-',
   (st_mode&S_IRGRP)?'x':'-',(st_mode&S_IROTH)?'r':'-',(st_mode&S_IWOTH)?'w':'-',
   (st_mode&S_IXOTH)?'x':'-');
   temp[9]='\0';
   printf("%s ",temp);
}

int compare(const void*a,const void*b){
    time_t m=((struct haha*)a)->tim;
    time_t n=((struct haha*)b)->tim;
    return (m-n)==0?0:(m-n);
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
void list_directory(const char*path,int*option){
    struct stat sta;
    struct dirent* dir;
    struct passwd* passwd1;
    struct passwd* passwd2;
    struct haha* haha=(struct haha*)malloc(sizeof(struct haha));
    int countfile=0;
    int temp=0;
    DIR *p1;
    p1=opendir(path);
    if(option[2]){
        printf("%s:\n",path);
    }

    while((dir=readdir(p1))!=NULL){
        if(!option[0]){
            if(dir->d_name[0]=='.')
            continue;
        }
        stat(dir->d_name,&sta);
        haha[countfile].name=dir->d_name;
        haha[countfile].tim=sta.st_mtime;
        countfile++;
        haha=realloc(haha,(countfile+1)*sizeof(struct haha));
    }
    
    if(option[3]){
        int (*com)(const void* a,const void* b)=&compare;
        qsort(haha,countfile,sizeof(struct haha),com);
    }

    if(option[4]){
        invert(haha,countfile);
    }

    while((haha[temp++].name)!=NULL){
        if(option[1]){
            passwd1=getpwuid(sta.st_uid);
            passwd2=getpwuid(sta.st_gid);
            printf("%-8s ",dir->d_name);
            printf_mode(sta.st_mode);
            printf("%ld ",sta.st_nlink);
            printf("%s ",passwd1->pw_name);
            printf("%s ",passwd2->pw_name);
            printf("%ld ",sta.st_size);
            printf("%s",ctime(&(sta.st_mtime)));
            if(option[6]){
                printf("%ld ",(sta.st_size)/512);
            }
            if(option[5]){
                printf("%ld ",sta.st_ino);
            }
        }
        else{
            printf("%s  ",dir->d_name);
            if(option[6]){
                printf("%ld ",(sta.st_size)/512);
            }
            if(option[5]){
                printf("%ld ",sta.st_ino);
            }
        }
    }
    free(haha);
        
    if(option[2]){
        char ful_path[PATH_MAX];
        DIR *p2;
        p2=opendir(path);
        struct dirent* dir;
        struct stat sta;
        printf("\n");
        while((dir=readdir(p2))!=NULL){
            if(!option[0]){
                if(dir->d_name[0]=='.')
                continue;
            }
        snprintf(ful_path,sizeof(ful_path),"%s/%s",path,dir->d_name);
        stat(ful_path,&sta);
        if(S_ISDIR(sta.st_mode)){
            list_directory(ful_path,option);
            //printf("\n");
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
        strncpy(buf,argv[optind],PATH_MAX-1);
        buf[PATH_MAX-1]='\0';
        temp=1;
    }
    if(!temp){
        getcwd(buf,sizeof(buf));
    }
    list_directory(buf, option);
    return 0;
}