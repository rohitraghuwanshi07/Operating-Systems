#include <stdio.h>
#include <string.h>

int n, k;
char seq[100][32];      // tools needed, in order

// Position (after index i) where the tool is needed again; n if never.
int next_use(char *tool, int i) {
    for (int j = i + 1; j < n; j++)
        if (strcmp(seq[j], tool) == 0)
            return j;
    return n;
}

int find_tool(char belt[][32], int size, char *tool) {
    for (int j = 0; j < size; j++)
        if (strcmp(belt[j], tool) == 0)
            return j;
    return -1;
}

// Trips if the mechanic returns the least recently used tool.
int lru_trips(void) {
    char belt[100][32];
    int last_used[100];
    int size = 0, trips = 0;

    for (int i = 0; i < n; i++) {
        int pos = find_tool(belt, size, seq[i]);
        if (pos != -1) {
            last_used[pos] = i;
        } else {
            trips++;
            if (size < k) {
                strcpy(belt[size], seq[i]);
                last_used[size] = i;
                size++;
            } else {
                int old = 0;                       // smallest last_used
                for (int j = 1; j < size; j++)
                    if (last_used[j] < last_used[old])
                        old = j;
                strcpy(belt[old], seq[i]);
                last_used[old] = i;
            }
        }
    }
    return trips;
}

int main(void) {
    char belt[100][32];     // index 0 = placed on the belt earliest
    int size = 0, trips = 0;

    scanf("%d", &k);
    scanf("%d", &n);
    for (int i = 0; i < n; i++)
        scanf("%31s", seq[i]);

    for (int i = 0; i < n; i++) {
        if (find_tool(belt, size, seq[i]) != -1)
            continue;                              // already on the belt

        trips++;
        if (size < k) {
            printf("Trip %d: take %s\n", trips, seq[i]);
        } else {
            // return the tool whose next use is farthest away;
            // strict ">" keeps the earliest-placed one on a tie
            int victim = 0;
            int far = next_use(belt[0], i);
            for (int j = 1; j < size; j++) {
                int nu = next_use(belt[j], i);
                if (nu > far) {
                    far = nu;
                    victim = j;
                }
            }
            printf("Trip %d: take %s, return %s\n", trips, seq[i], belt[victim]);

            for (int j = victim; j < size - 1; j++)    // remove victim
                strcpy(belt[j], belt[j + 1]);
            size--;
        }
        strcpy(belt[size++], seq[i]);              // new tool goes at the end
    }

    printf("Total trips: %d\n", trips);
    printf("Trips if returning least recently used tool: %d\n", lru_trips());
    return 0;
}
