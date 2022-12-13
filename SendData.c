#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/mman.h>
#include <arpa/inet.h>
#include <time.h>
#include <pthread.h>
#include <sys/stat.h>



int sumBin(int num) {

    if (num == 0) {
        return 0;
    }
   
   // Stores binary representation of number.
    int binaryNum[32]; // Assuming 32 bit integer.
    int i=0;
   
    for ( ;num > 0; ){
        binaryNum[i++] = num % 2;
        num /= 2;
    }
    int sum=0;
    for (int j = i-1; j >= 0; j--){
        sum+=binaryNum[j];
    }
    return sum;
   
}
int checksum(char* str){
    int sum=0;
    int length = strlen(str);
    for (int i = 0; i < length; i++)
    {   
        sum+=sumBin(str[i]);
    }
    return sum;
    
}
int checksumFile(char * file)
{    
    int first = open(file,O_RDONLY);
    if(first == -1)
    {
        perror("error opening first file: ");
        return -1;
    }
    char data[1000];
    int bytesRead,sum = 0; 
    while(1)
    {
        bzero(data, 1000);
        //reading the data and checking validity
        bytesRead = read(first,data,1000);
        if(bytesRead < 0)
        {
            close(first);
            perror("Error while reading: ");
            break;
        }
        else if(bytesRead == 0)
        {
            break;
        }
        sum+=checksum(data);
    }
    close(first);
    return sum;
}
int sendTCP()
{
    int fd = open("largeData.txt",O_RDONLY);
    if(fd <0)
    {
        perror("error opening file");
        exit(1);
    }
    printf("opened file\n");
    int client_sock;
    struct sockaddr_in addr;
    
    client_sock = socket(AF_INET,SOCK_STREAM,0);
    if(client_sock < 0)
    {
        perror("error opening socket");
        close(client_sock);
        exit(1);
    }
    printf("opend socket\n");
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9999);
    inet_pton(AF_INET,"127.0.0.1",&(addr.sin_addr));
    if(connect(client_sock,(struct sockaddr*)&addr,sizeof(addr)) < 0 ){
        perror("error connecting");
        exit(0);
    }

    printf("connected socket\n");

    clock_t t;
    t = clock();
    char buffer[1024];
    int amountRead = read(fd,buffer,1023);
    int amounrWritten;
    int check = 0;
    while(amountRead != 0)
    {
        check += checksum(buffer);
        amounrWritten = write(client_sock,buffer,1023);
        memset(buffer,'\0',1024);
        amountRead = read(fd,buffer,1023);
    }
    t = clock() - t;
    double time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds
    printf("it took %f second to send data\n", time_taken);
    printf("checksum: %d\n",check);
    close(client_sock);
    close(fd);
    return 0;
}

int sendUDP()
{
    int fd = open("largeData.txt",O_RDONLY);
    if(fd <0)
    {
        perror("error opening file");
        exit(1);
    }
    printf("opened file\n");
    int client_sock;
    struct sockaddr_in6 addr;
    
    client_sock = socket(AF_INET6,SOCK_DGRAM,0);
    if(client_sock < 0)
    {
        perror("error opening socket");
        close(client_sock);
        exit(1);
    }
    printf("opend socket\n");
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(12345);
    addr.sin6_addr = in6addr_any;
    
    printf("connected socket\n");

    clock_t t;
    t = clock();
    char buffer[1024];
    int amountRead = 0;
    int amounrWritten;
    int check = 0;
    int bufferAmount = 0;
    while((amountRead  = read(fd,buffer,1023))!= 0)
    {
        check += checksum(buffer);
        amounrWritten = sendto(client_sock,&buffer,1023,0,(const struct sockaddr*)&addr,sizeof(addr));
        memset(buffer,'\0',1024);
    }
    t = clock() - t;
    double time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds
    printf("it took %f second to send data\n", time_taken);
    printf("checksum: %d\n",check);
    sendto(client_sock,"",0,0,(const struct sockaddr*)&addr,sizeof(addr));
    close(client_sock);
    close(fd);
    return 0;
}

checkMMAP()
{
    int fd = open("largeData.txt",O_RDWR);

    printf("checksumFile: %d\n",checksumFile("largeData.txt"));
    if(fd < 0)
    {
        perror("error opening file");
        exit(1);
    }
    printf("opened file\n");
    struct stat st;
    if(fstat(fd,&st) == -1)
    {
        perror("erorr st\n");
        exit(1);
    }
    size_t file_size = st.st_size;
    clock_t tWrite;
    tWrite = clock();
    char* addr = mmap(0,file_size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
    if(addr == MAP_FAILED)
    {
        perror("falied mapping");
        exit(1);
    }
    tWrite = clock() - tWrite;
    double time_taken = ((double)tWrite)/CLOCKS_PER_SEC; // in seconds
    //printf("string : %s, len: %d\n",addr,strlen(addr));
    printf("time to write file: %f\n", time_taken);
    printf("length: %ld\n",strlen(addr));
    //printf("write checksum: %d\n",checksum(addr));
  
    int fork_id = fork();
    if(fork_id == -1)
    {
        perror("error opening process");
        exit(1);
    }
    if(fork_id == 0)
    {
        clock_t tRead;
        printf("in child\n");
        char toCopy[1024];
        tRead = clock();
        int index = 0;
        int check = 0;
        while(index <= file_size)
        {
            bzero(toCopy,1023);
            for(int i = 0; i < 1023;i++)
            {   
                if(index > file_size)
                {
                    break;
                }
                toCopy[i] = addr[index];
                index++; 
            }
            check += checksum(toCopy);
        }
        tRead = clock() - tRead;
        printf("index: %d\n",index);
        double time_taken = ((double)tRead)/CLOCKS_PER_SEC; // in seconds
        printf("time to read file: %f\n", time_taken);
        printf("read checksum: %d\n",check);
    }
    else
    {
        waitpid(fork_id,NULL,0);
    }
    close(fd);
    munmap(addr,file_size);
    return 0;
    

}

size_t file_size;
int thread_fd;
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;
int check = 0;
int ThreadIndex = 0;
char* ThreadBuffer;
void* threadFunc_1(void* args)
{
    printf("first thread func\n");
    thread_fd = open("test.txt",O_RDONLY);
    if(thread_fd < 0)
    {
        perror("error opening file");
        exit(1);
    }
    printf("opened file\n");
    struct stat st;
    
    if(fstat(thread_fd,&st) == -1)
    {
        perror("erorr st\n");
        exit(1);
    }
    file_size = st.st_size; 
    ThreadBuffer= (char*)calloc(file_size,sizeof(char));
    int amount_read = 0;
    pthread_mutex_lock(&file_mutex);
    while(ThreadIndex < file_size)
    {
        amount_read = read(thread_fd,ThreadBuffer+ThreadIndex,1023);
        if(amount_read < 0)
        {
            perror("error reading");
            exit(1);
        }
        ThreadIndex += amount_read;
    }
    
    pthread_mutex_unlock(&file_mutex);
}
void* threadFunc_2(void* args){
    printf("second thread func\n");
    pthread_mutex_lock(&file_mutex);
    check = checksum(ThreadBuffer);
    int fileCheck = checksumFile("largeData.txt");
    if(check == fileCheck)
    {
        printf("file written successfully\n");
    }
    else
    {
        printf("-1");
    }
    pthread_mutex_unlock(&file_mutex);
}



void threadCheck()
{
    pthread_t thread1, thread2;
    int check1,check2;
    printf("in thread func\n");
    check1 = pthread_create(&thread1, NULL, threadFunc_1,NULL);
    check2 = pthread_create(&thread2, NULL, threadFunc_2,NULL);

    pthread_join(thread1,NULL);
    pthread_join(thread2,NULL);
    printf("thread 1 returned: %d\n",check1);
    printf("thread 2 returned: %d\n",check2);    
}

int main()
{
    //sendTCP();
    //sendUDP();
    //checkMMAP();
    threadCheck();
    return 0;
}