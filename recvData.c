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
int readTCP()
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
    bzero(buffer,1024);
    clock_t t;
    t = clock();
    int amountRead = 0;
    int check = 0;
    while((amountRead =read(acceptedSocket,buffer,1023)) != 0)
    {
        check += checksum(buffer);
        memset(buffer,'\0',1024);
    }
    t = clock() - t;
    double time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds
    printf("it took %f second to recv data\n", time_taken);
    printf("checksum for text recived: %d\n",check);
    close(accept);
    //close(fd);
    return 0;
}
int readUDP()
{
    int sock;
    int acceptedSocket;
    struct sockaddr_in6 server,client;
    sock = socket(AF_INET6,SOCK_DGRAM,0);
    if(sock < 0)
    {
        perror("error opening socket");
        close(sock);
        exit(1);
    }
    printf("opened socket\n");
    bzero(&server, sizeof(server));
    bzero(&client, sizeof(client));
    server.sin6_family = AF_INET6;
    server.sin6_port = htons(12345);
    server.sin6_addr = in6addr_any;
    if(bind(sock, (struct sockaddr *) &server, sizeof(server))<0)
    {
        perror("error binding");
        exit(1);
    }
    printf("binded socket\n");
    socklen_t len = sizeof(client);
    char buffer[1024];
    clock_t t;
    t = clock();
    int overall = 0;
    int amountRead = 0;
    int check = 0;
    printf("before read loop\n");
    while((amountRead= recvfrom(sock,&buffer,1023,0,(const struct sockaddr*)&client,&len)) > 0)
    {
        overall+= amountRead;
        printf("amound read: %d\n",overall);
        check += checksum(buffer);
        bzero(buffer,1024);
    }
    t = clock() - t;
    double time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds
    printf("it took %f second to recv data\n", time_taken);
    printf("checksum for transfered data: %d\n",check);
    close(sock);
    return 0;
}



int main()
{
   //readTCP();
   readUDP();
   return 0;
}