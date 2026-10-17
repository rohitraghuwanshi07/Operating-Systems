#include <stdio.h>

// FIFO simulation with k slots.
// Returns total time; mem[] holds the stored IDs (earliest first).
int simulate(int k, int n, int ids[], int mem[], int *count) {
    int size = 0, total = 0;

    for (int i = 0; i < n; i++) {
        int found = 0;
        for (int j = 0; j < size; j++) {
            if (mem[j] == ids[i]) {
                found = 1;
                break;
            }
        }

        if (found) {
            total += 1;                    // hit: order does not change
        } else {
            total += 5;                    // miss: ask the server
            if (size == k) {               // memory full: remove earliest
                for (int j = 0; j < size - 1; j++)
                    mem[j] = mem[j + 1];
                size--;
            }
            mem[size++] = ids[i];          // store the new ID at the end
        }
    }

    *count = size;
    return total;
}

int main(void) {
    int k, n, t;
    int ids[1000], mem[1001], tmp[1001];
    int count;

    scanf("%d", &k);
    scanf("%d", &n);
    for (int i = 0; i < n; i++)
        scanf("%d", &ids[i]);
    scanf("%d", &t);

    int total = simulate(k, n, ids, mem, &count);
    printf("Total time with %d slots: %d seconds\n", k, total);
    printf("Memory at end:");
    for (int i = 0; i < count; i++)
        printf(" %d", mem[i]);
    printf("\n");

    // Try 1, 2, 3 ... slots. More slots do not always help,
    // so we cannot binary search; check each one in order.
    int answer = -1;
    for (int s = 1; s <= n; s++) {
        int c;
        if (simulate(s, n, ids, tmp, &c) <= t) {
            answer = s;
            break;
        }
    }

    if (answer == -1)
        printf("Minimum slots for time <= %d: Not possible\n", t);
    else
        printf("Minimum slots for time <= %d: %d\n", t, answer);
    return 0;
}
