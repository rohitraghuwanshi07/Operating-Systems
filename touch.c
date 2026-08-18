#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main(void)
{
    int fd;

    fd = open("demo.txt", O_CREAT, 0644);

    if (fd == -1)
    {
        perror("open");
        return 1;
    }

    printf("File created/opened successfully.\n");

    close(fd);

    return 0;
}
