#include <stdio.h>

#define MAX_HOLES 10

typedef struct {
    int base;
    int size;
} Hole;

Hole holes[MAX_HOLES] = {
    {200, 300},   // Hole A
    {700, 200}    // Hole B
};
int hole_count = 2;

void print_holes(void) {
    for (int i = 0; i < hole_count; i++)
        printf("  Hole: Base %d (Size %d)\n", holes[i].base, holes[i].size);
}

int worst_fit_allocate(int req_size) {
    int worst = -1;

    // find the largest hole that fits the request
    for (int i = 0; i < hole_count; i++) {
        if (holes[i].size >= req_size &&
            (worst == -1 || holes[i].size > holes[worst].size))
            worst = i;
    }

    if (worst == -1) {
        printf("Allocation of %d failed: no hole large enough\n", req_size);
        return -1;
    }

    int alloc_base = holes[worst].base;
    int leftover = holes[worst].size - req_size;

    printf("Requested size: %d\n", req_size);
    printf("Chosen hole: Base %d (Size %d)\n", alloc_base, holes[worst].size);

    if (leftover > 0) {
        holes[worst].base += req_size;   // leftover becomes the new hole
        holes[worst].size = leftover;
        printf("New leftover hole: Base %d (Size %d)\n",
               holes[worst].base, holes[worst].size);
    } else {
        for (int i = worst; i < hole_count - 1; i++)
            holes[i] = holes[i + 1];
        hole_count--;
        printf("No leftover; hole removed\n");
    }
    return alloc_base;
}

int main(void) {
    printf("=== WORST FIT ===\n");
    printf("Holes before:\n");
    print_holes();
    worst_fit_allocate(150);
    printf("Holes after:\n");
    print_holes();
    return 0;
}
