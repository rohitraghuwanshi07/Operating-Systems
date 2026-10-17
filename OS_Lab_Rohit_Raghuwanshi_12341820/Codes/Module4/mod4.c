#include <stdio.h>
#include <string.h>

#define RAM_SIZE 1000
#define MAX_SEGS 10

char RAM[RAM_SIZE];

typedef struct {
    int id;
    int base;
    int size;   // payload length including '\0'
} Segment;

Segment segs[MAX_SEGS];
int seg_count = 0;

void add_segment(int id, int base, const char *payload) {
    int size = strlen(payload) + 1;
    memcpy(&RAM[base], payload, size);
    segs[seg_count++] = (Segment){id, base, size};
}

void compact_memory(void) {
    // sort segments by base address (simple bubble sort)
    for (int i = 0; i < seg_count - 1; i++)
        for (int j = 0; j < seg_count - 1 - i; j++)
            if (segs[j].base > segs[j + 1].base) {
                Segment t = segs[j];
                segs[j] = segs[j + 1];
                segs[j + 1] = t;
            }

    int next_free = 0;
    for (int i = 0; i < seg_count; i++) {
        int old_base = segs[i].base;
        int size = segs[i].size;

        if (old_base != next_free) {
            memmove(&RAM[next_free], &RAM[old_base], size);

            // clear only old bytes not covered by the new location
            int clear_start = (old_base > next_free + size) ? old_base : next_free + size;
            int clear_end = old_base + size;
            if (clear_end > clear_start)
                memset(&RAM[clear_start], 0, clear_end - clear_start);

            segs[i].base = next_free;
        }
        next_free += size;
    }
}

void print_state(const char *title) {
    printf("%s\n", title);
    for (int i = 0; i < seg_count; i++)
        printf("  Seg %d: Base %d, Payload \"%s\"\n",
               segs[i].id, segs[i].base, &RAM[segs[i].base]);
}

int main(void) {
    memset(RAM, 0, RAM_SIZE);
    add_segment(0, 200, "HELLO");
    add_segment(1, 600, "WORLD");

    print_state("Before compaction:");
    compact_memory();
    print_state("After compaction:");

    printf("RAM[0..4]  = \"%.5s\"\n", &RAM[0]);
    printf("RAM[6..10] = \"%.5s\"\n", &RAM[6]);
    printf("Old positions cleared: RAM[200]=%d, RAM[600]=%d\n", RAM[200], RAM[600]);
    return 0;
}
