#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: ./myrmdir <directory>\n");
        return 1;
    }

    if (rmdir(argv[1]) == -1)
    {
        perror("rmdir");
        return 1;
    }

    printf("Removed: %s\n", argv[1]);

    return 0;
}
