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

    clock_t t   ;
    t = clock();
    char buffer[1024];
    int amountRead = read(fd,buffer,1024);
    int amounrWritten;
    while(amountRead != 0)
    {
        amounrWritten = write(client_sock,buffer,1024);
        printf("wrote %d bytes\n",amounrWritten);
        amountRead = read(fd,&buffer,1024);
    }
    t = clock() - t;
    double time_taken = ((double)t)/CLOCKS_PER_SEC; // in seconds
    printf("it took %f second to send data", time_taken);
    close(client_sock);
    close(fd);
    return 0;
}