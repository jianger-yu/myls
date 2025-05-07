#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<dirent.h>
#include<pthread.h>
#include<sys/stat.h>
#include<unistd.h>
#include<time.h>
#include<pwd.h>
#include<grp.h>
#include<stdbool.h>
#define max_road 30000
char ca[300]={0};
struct my_dr{
    char name[256];
    long unsigned int ino;
} ;
extern void ptfl(struct my_dr,char *);
char isFile(char *s)//传入文件路径，返回文件类型（ls -l首位的显示格式）
{
    struct stat st;
    if(lstat(s,&st)==-1){
        perror("25:stat:");
        return ' ';
    }
    if((st.st_mode & S_IFMT)==S_IFREG)//常规文件
        return '-';
    else if((st.st_mode & S_IFMT)==S_IFDIR)//目录
        return 'd';
    else if((st.st_mode & S_IFMT)==S_IFCHR)//字符设备
        return 'c';
    else if((st.st_mode & S_IFMT)==S_IFBLK)//块设备
        return 'b';
    else if((st.st_mode & S_IFMT)==S_IFIFO)//FIFO 或管道
        return 'p';
    else if((st.st_mode & S_IFMT)==S_IFSOCK)//套接字
        return 's';
    else if((st.st_mode & S_IFMT)==S_IFLNK)//符号链接
        return 'l';
    return ' ';
}
bool cmp(char *a,char*b)//返回a时间是否新于b时间
{
    struct stat sta,stb;
    if(lstat(a,&sta)==-1){
        perror("sta");
        exit(1);
    }
    if(lstat(b,&stb)==-1){
        perror("stb");
        exit(1);
    }
    time_t mta = sta.st_mtime;
    struct tm * ta = localtime(&mta);
    int yea=ta->tm_year,mo=ta->tm_mon,da=ta->tm_mday,hou=ta->tm_hour,mi=ta->tm_min;
    time_t mtb = stb.st_mtime;
    struct tm * tb = localtime(&mtb);
    if(yea!=tb->tm_year) return yea  >  tb->tm_year;
    if(mo!=tb->tm_mon)   return mo   >  tb->tm_mon;
    if(da!=tb->tm_mday)  return da   >  tb->tm_mday;
    if(hou!=tb->tm_hour) return hou  >  tb->tm_hour;
    if(mi!=tb->tm_min)   return mi   >  tb->tm_min;
    return false;
}

bool cmp2(char *a,char*b){
    for(int i=0;i<256;i++){
        if(a[i]!=b[i])
            return a[i]<b[i];
    }
    return false;
}

void zisort(struct my_dr *arr,int l,int r,char *s)
{
    if(l>=r) return ;
    int i = l - 1, j = r + 1, x = (l + r) >> 1;
    while(i<j)
    {
        do i++;     while(cmp2(arr[i].name,arr[x].name));
        do j--;     while(cmp2(arr[x].name,arr[j].name));
        if(i<j) //swap(arr[i],arr[j])
        {
            struct my_dr tmp= arr[i];
            arr[i]=arr[j];
            arr[j]=tmp;
        }
    }
    zisort(arr, l, j, s);
    zisort(arr, j + 1, r, s);
}

void msort(struct my_dr *arr,int l,int r,char *s)
{
    if(l>=r) return ;
    int i = l - 1, j = r + 1, x = (l + r) >> 1;
    char road[max_road],roadx[max_road];
    sprintf(roadx,"%s/%s",s,arr[x].name);
    while(i<j)
    {
        do {
            i++;
            sprintf(road,"%s/%s",s,arr[i].name);//构建绝对路径
        } 
        while(cmp(road,roadx));
        do {
            j--;
            sprintf(road,"%s/%s",s,arr[j].name);//构建绝对路径
        } 
        while(cmp(roadx,road));
        if(i<j) //swap(arr[i],arr[j])
        {
            struct my_dr tmp= arr[i];
            arr[i]=arr[j];
            arr[j]=tmp;
        }
    }
    msort(arr, l, j, s);
    msort(arr, j + 1, r, s);
}

void hdir(char *s)//传入目录路径，*处理*该目录下所有内容
{
    DIR *dp;
    struct dirent * di;
    //struct dirent ** arr=(struct dirent **)malloc(sizeof(struct dirent *)*20);
    struct my_dr *arr=(struct my_dr *)malloc(sizeof(struct my_dr )*20);
    if(arr==NULL){
        perror("malloc");
        exit(1);
    }
    int cnt=0,size=20;
    //打开目录
    dp=opendir(s);
    if(dp==NULL){
        perror("opendir");
        exit(1);
    }
    //读目录
    while((di=readdir(dp))!=NULL){
        if(ca['a']==0&&di->d_name[0]=='.')continue;
        if(cnt>=size)//扩容
        {
            size*=2;
            arr=(struct my_dr *)realloc(arr,sizeof(struct my_dr )*size);
        }
        strcpy(arr[cnt].name,di->d_name);
        arr[cnt++].ino=di->d_ino;
    }
    //按字典序预排序
    zisort(arr,0,cnt-1,s);

    //数组处理排序
    if(ca['t']>0) msort(arr,0,cnt-1,s);
    char road[max_road];
    //按顺序打印、递归
    if(ca['r']==0)
    {
        for(int i=0;i<cnt;i++){
            ptfl(arr[i],s);
        }
        //打印完换行
        printf("\n");
        if(ca['R']>0)
        for(int i=0;i<cnt;i++){
            if(strcmp(arr[i].name,".")==0||strcmp(arr[i].name,"..")==0) continue;
            sprintf(road,"%s/%s",s,arr[i].name);
            if(isFile(road)!='d') continue;
            printf("%s:\n",road);
            hdir(road);
        }
    }
    else
    {
        for(int i=cnt-1;i>=0;i--){
            ptfl(arr[i],s);
        }
        if(ca['R']>0)
        for(int i=cnt-1;i>=0;i--){
            if(strcmp(arr[i].name,".")==0||strcmp(arr[i].name,"..")==0) continue;
            sprintf(road,"%s/%s",s,arr[i].name);
            if(isFile(road)!='d') continue;
            printf("%s:\n",road);
            hdir(road);
        }
    }
    //关闭目录
    free(arr);
    closedir(dp);
}

void ptfl(struct my_dr di,char * s){
    struct stat st;
    char road[max_road];
    sprintf(road,"%s/%s",s,di.name);
    if(lstat(road,&st)==-1){
        perror("stat");
        return;
    }
    if(ca['i']>0)//打印inode号
        printf("%lu ",di.ino);
    if(ca['s']>0)//打印文件占用块大小
        printf("%lu ",st.st_blksize);
    if(ca['l']>0)//
    {
        //文件类型
        printf("%c",isFile(road));
        //输出9位权限 rwx...
        if(st.st_mode&S_IRUSR) printf("r"); else printf("-");
        if(st.st_mode&S_IWUSR) printf("w"); else printf("-");
        if(st.st_mode&S_IXUSR) printf("x"); else printf("-");
        if(st.st_mode&S_IRGRP) printf("r"); else printf("-");
        if(st.st_mode&S_IWGRP) printf("w"); else printf("-");
        if(st.st_mode&S_IXGRP) printf("x"); else printf("-");
        if(st.st_mode&S_IROTH) printf("r"); else printf("-");
        if(st.st_mode&S_IWOTH) printf("w"); else printf("-");
        if(st.st_mode&S_IXOTH) printf("x "); else printf("- ");
        //硬链接数量
        printf("%lu ",st.st_nlink);
        //用户id 组id
        struct passwd *suid = getpwuid(st.st_uid);
        printf("%s ",suid->pw_name);
        struct group * sgid = getgrgid(st.st_gid);
        printf("%s ",sgid->gr_name);
        //所占大小
        printf("%ld ",st.st_size);
        //时间戳
        time_t mtime = st.st_mtime;
        struct tm * stm = localtime(&mtime);
        printf("%2d月  %2d %02d:%02d ",stm->tm_mon,stm->tm_mday,stm->tm_hour,stm->tm_min);
        //文件名
        if(isFile(road)=='d')
            printf("\033[;34m%s\033[0m\n",di.name);//蓝色
        else if(isFile(road)=='l')
            printf("\033[;31m%s\033[0m\n",di.name);//红色
        else   printf("%s\n",di.name);
    }
    else {
        //文件名
        if(isFile(road)=='d')
            printf("\033[;34m%s\033[0m\t",di.name);//蓝色
        else if(isFile(road)=='l')
            printf("\033[;31m%s\033[0m\t",di.name);//红色
        else   printf("%s\t",di.name);
        if(ca['s']>0||ca['i']>0) printf("\n");
    }
}

int main(int argc,char* argv[]){
    int cnt=0;
    //录入所有参数
    for(int i=1;i<argc;i++){
        if(argv[i][0]=='-'){
            for(int j=1;argv[i][j]!='\0';j++){
                ca[argv[i][j]]++;
            }
        }
    }
    //处理文件
    for(int i=1;i<argc;i++){
        if(argv[i][0]!='-'){
            if(isFile(argv[i])=='d') hdir(argv[i]);
            else {
                DIR *dp;
                struct dirent *di;
                struct my_dr dr;
                char road[max_road];
                if(getcwd(road,256)==NULL){
                    perror("getcwd");
                    exit(1);
                }
                dp=opendir(road);
                if(dp==NULL){
                    perror("di opendir");
                    exit(1);
                }
                while((di=readdir(dp))!=NULL){
                    if(strcmp(di->d_name,argv[i])==0) break;
                }
                strcpy(dr.name,di->d_name);
                dr.ino=di->d_ino;
                ptfl(dr,road);
                closedir(dp);
            }
            cnt++;
        }
    }
    if(cnt==0) hdir((char*)".");
    return 0;
}

