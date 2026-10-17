#include <stdio.h>
#include <string.h>

int main(void) {
    int k, n;
    char ram[100][32];      // index 0 = least recently opened
    char killed[100][32];
    int size = 0, kcount = 0, total = 0;
    char app[32];

    scanf("%d", &k);
    scanf("%d", &n);

    for (int i = 0; i < n; i++) {
        scanf("%31s", app);

        // look for the app in RAM
        int pos = -1;
        for (int j = 0; j < size; j++) {
            if (strcmp(ram[j], app) == 0) {
                pos = j;
                break;
            }
        }

        if (pos != -1) {
            total += 1;                          // warm start
            for (int j = pos; j < size - 1; j++) // move app to the end
                strcpy(ram[j], ram[j + 1]);
            strcpy(ram[size - 1], app);
        } else {
            total += 4;                          // cold start
            if (size == k) {                     // RAM full: kill ram[0]
                strcpy(killed[kcount++], ram[0]);
                for (int j = 0; j < size - 1; j++)
                    strcpy(ram[j], ram[j + 1]);
                size--;
            }
            strcpy(ram[size++], app);
        }
    }

    printf("Total time: %d seconds\n", total);

    printf("Apps killed:");
    if (kcount == 0)
        printf(" none");
    for (int i = 0; i < kcount; i++)
        printf(" %s", killed[i]);
    printf("\n");

    printf("Recent apps:");
    for (int i = size - 1; i >= 0; i--)     // most recent first
        printf(" %s", ram[i]);
    printf("\n");
    return 0;
}
