#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: ./mymkdir <directory>\n");
        return 1;
    }

    if (mkdir(argv[1], 0755) == -1)
    {
        perror("mkdir");
        return 1;
    }

    printf("Created: %s\n", argv[1]);

    return 0;
}
