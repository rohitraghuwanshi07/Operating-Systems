#include <stdio.h>
#include <string.h>

int main(void)
{
    FILE *file;
    char line[256];
    char search[100];

    printf("Enter the string to search: ");
    fgets(search, sizeof(search), stdin);

    /* Remove newline from search string */
    search[strcspn(search, "\n")] = '\0';

    file = fopen("demo.txt", "r");

    if (file == NULL)
    {
        perror("fopen");
        return 1;
    }

    printf("\nLines containing \"%s\":\n", search);

    while (fgets(line, sizeof(line), file) != NULL)
    {
        if (strstr(line, search) != NULL)
        {
            printf("%s", line);
        }
    }

    fclose(file);

    return 0;
}
