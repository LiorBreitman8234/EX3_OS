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
#include <arpa/inet.h>
#include <time.h>



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
    for (int i = 0; i < strlen(str); i++)
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


int main()
{
    int sock;
    int acceptedSocket;
    struct sockaddr_in server;
    size_t len;
    sock = socket(AF_INET,SOCK_STREAM,0);
    if(sock < 0)
    {
        perror("error opening socket");
        close(sock);
        exit(1);
    }
    printf("opened socket\n");
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(9999);

    if(bind(sock, (struct sockaddr *) &server, sizeof(server))<0)
    {
        perror("error binding");
        exit(1);
    }
    printf("binded socket\n");

    if(listen(sock,1) <0){
        perror("error listening");
        exit(1);
    }
    printf("socket listening\n");


    len = sizeof(server);
    acceptedSocket = accept(sock, (struct sockaddr *) &server,&len);
    if(acceptedSocket < 0)
    {
        perror("error connecting");
        exit(1);
    }

    printf("socket conncted\n");
    char buffer[1024];
    int fd = open("copied.txt",O_CREAT|O_WRONLY | O_TRUNC, S_IRWXU|S_IRWXG|S_IRWXO);
    clock_t t;
    t = clock();
    int amountRead = 0;
    while((amountRead =read(acceptedSocket,buffer,1024)) != 0)
    {
        printf("read %d bytes\n",amountRead);
        write(fd,buffer,1024);
    }
    t = clock() - t;
    double time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds
    printf("it took %f second to recv data", time_taken);
    close(accept);
    close(fd);
    return 0;
}