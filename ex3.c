#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

void createData(size_t size,char* fileName)
{
    time_t t;
    srand((unsigned) time(&t));
    int fd = open(fileName,O_WRONLY |O_CREAT,S_IRWXU|S_IRWXG|S_IRWXO);
    size_t i = 0;
    while(i < size)
    {
        double curr = (double)rand() / (double)RAND_MAX;
        if(curr> 0.5)
        {
            write(fd,"1",1);
        }
        else
        {
            write(fd,"0",1);
        }
        i++;
    }
    close(fd);
}
int main()
{
    createData(104857600,"largeData.txt");
    return 0;
}