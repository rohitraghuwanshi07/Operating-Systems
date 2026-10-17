#include <stdio.h>

typedef struct {
    int id;
    const char *type;
    int base;
    int limit;
    int downward;   // 1 = grows downward (stack), 0 = upward
} Segment;

int translate(Segment s, int offset) {
    if (offset < 0 || offset >= s.limit) {
        printf("Seg %d, offset %d -> SEGMENTATION FAULT\n", s.id, offset);
        return -1;
    }
    int pa = s.downward ? s.base - offset : s.base + offset;
    printf("Seg %d, offset %d -> Physical Address: %d\n", s.id, offset, pa);
    return pa;
}

int main(void) {
    Segment stack = {2, "Stack (Downward)", 4700, 400, 1};
    translate(stack, 100);   // expect 4600
    translate(stack, 500);   // expect SEGMENTATION FAULT
    return 0;
}
