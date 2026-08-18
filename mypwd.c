#include <stdio.h>
#include <unistd.h>
#include <limits.h>

int main(void)
{
    char path[PATH_MAX];

    if (getcwd(path, sizeof(path)) == NULL)
    {
        perror("getcwd");
        return 1;
    }

    printf("%s\n", path);

    return 0;
}
