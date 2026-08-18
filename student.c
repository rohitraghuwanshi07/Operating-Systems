#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    const char *filename = "student.txt";

    const char *message =
        "Name: Rohit Raghuwanshi\n"
        "Address: Pipariya\n"
        "City : Bhilai\n"
        "Subject: Operating System\n";

    char buffer[200];
    int fd, n;

    /* STEP 1: OPEN file for writing */
    printf("Step 1: Opening '%s' for writing...\n", filename);

    fd = open(filename, O_CREAT | O_WRONLY | O_APPEND, 0644);

    if (fd == -1) {
        perror("open (write)");
        return 1;
    }

    printf("   -> Success. File descriptor = %d\n\n", fd);

    /* STEP 2: WRITE student information */
    printf("Step 2: Writing student information to the file...\n");

    n = write(fd, message, strlen(message));

    if (n == -1) {
        perror("write");
        close(fd);
        return 1;
    }

    printf("   -> Wrote %d bytes.\n\n", n);

    /* STEP 3: CLOSE the file */
    printf("Step 3: Closing the file descriptor %d...\n", fd);

    close(fd);

    printf("   -> Closed.\n\n");

    /* STEP 4: OPEN file for reading */
    printf("Step 4: Reopening '%s' for reading...\n", filename);

    fd = open(filename, O_RDONLY);

    if (fd == -1) {
        perror("open (read)");
        return 1;
    }

    printf("   -> Success. File descriptor = %d\n\n", fd);

    /* STEP 5: READ from the file */
    printf("Step 5: Reading contents from the file...\n");

    n = read(fd, buffer, sizeof(buffer) - 1);

    if (n == -1) {
        perror("read");
        close(fd);
        return 1;
    }

    buffer[n] = '\0';

    /* STEP 6: DISPLAY contents */
    printf("Step 6: Student information:\n");
    printf("----------------------------\n");
    printf("%s", buffer);
    printf("----------------------------\n\n");

    /* STEP 7: CLOSE the file */
    printf("Step 7: Closing the file descriptor %d...\n", fd);

    close(fd);

    printf("   -> Done!\n");

    return 0;
}
