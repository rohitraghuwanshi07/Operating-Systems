#include <stdio.h>
#include <string.h>

int cap, s;
char name[100][32];
int size_of[100];

int find_song(char *nm) {
    for (int i = 0; i < s; i++)
        if (strcmp(name[i], nm) == 0)
            return i;
    return -1;
}

// policy: 1 = LRU, 0 = FIFO
// stored[] is kept in deletion order: stored[0] is deleted next.
//   FIFO: a hit does not change the order.
//   LRU : a hit moves the song to the end (most recently played).
void simulate(int policy, int p, int plays[],
              int *hits, int *misses, int *down, int stored[], int *count) {
    int used = 0, cnt = 0;
    *hits = *misses = *down = 0;

    for (int i = 0; i < p; i++) {
        int song = plays[i];

        int pos = -1;
        for (int j = 0; j < cnt; j++)
            if (stored[j] == song) {
                pos = j;
                break;
            }

        if (pos != -1) {
            (*hits)++;
            if (policy == 1) {                       // LRU: move to the end
                for (int j = pos; j < cnt - 1; j++)
                    stored[j] = stored[j + 1];
                stored[cnt - 1] = song;
            }
        } else {
            (*misses)++;
            if (size_of[song] > cap)                 // too big: streamed
                continue;

            while (used + size_of[song] > cap) {     // delete only as needed
                used -= size_of[stored[0]];
                for (int j = 0; j < cnt - 1; j++)
                    stored[j] = stored[j + 1];
                cnt--;
            }
            stored[cnt++] = song;
            used += size_of[song];
            *down += size_of[song];
        }
    }
    *count = cnt;
}

void print_result(char *label, int hits, int misses, int down, int stored[], int count) {
    printf("%s: hits %d, misses %d, downloaded %d MB, stored:", label, hits, misses, down);
    for (int i = 0; i < count; i++)
        printf(" %s", name[stored[i]]);
    printf("\n");
}

int main(void) {
    int p, plays[100];
    char buf[32];

    scanf("%d", &cap);
    scanf("%d", &s);
    for (int i = 0; i < s; i++)
        scanf("%31s %d", name[i], &size_of[i]);
    scanf("%d", &p);
    for (int i = 0; i < p; i++) {
        scanf("%31s", buf);
        plays[i] = find_song(buf);
    }

    int h1, m1, d1, c1, h2, m2, d2, c2;
    int st1[100], st2[100];

    simulate(1, p, plays, &h1, &m1, &d1, st1, &c1);   // LRU
    simulate(0, p, plays, &h2, &m2, &d2, st2, &c2);   // FIFO

    print_result("LRU", h1, m1, d1, st1, c1);
    print_result("FIFO", h2, m2, d2, st2, c2);

    if (d1 < d2)
        printf("Better policy: LRU\n");
    else if (d2 < d1)
        printf("Better policy: FIFO\n");
    else
        printf("Better policy: Both equal\n");
    return 0;
}
