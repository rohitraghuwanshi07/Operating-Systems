#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: ./myrm <file>\n");
        return 1;
    }

    if (unlink(argv[1]) == -1)
    {
        perror("unlink");
        return 1;
    }

    printf("Removed: %s\n", argv[1]);

    return 0;
}
