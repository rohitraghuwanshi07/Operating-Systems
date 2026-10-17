#include <stdio.h>

#define PERM_R 1
#define PERM_W 2
#define PERM_X 4

typedef struct {
    int id;
    const char *name;
    int min_ring;   // 0 = Kernel ... 3 = User
    int perms;      // bitmask of R/W/X
} Segment;

void check_access(int cpu_ring, Segment s, int req) {
    printf("Ring %d -> Seg %d (%s), min_ring %d, req %d, allowed %d: ",
           cpu_ring, s.id, s.name, s.min_ring, req, s.perms);

    if (cpu_ring > s.min_ring) {
        printf("TRAP: Privilege Violation\n");
        return;
    }
    if ((req & s.perms) != req) {
        printf("PERMISSION DENIED\n");
        return;
    }
    printf("SUCCESS\n");
}

int main(void) {
    Segment kcode = {0, "Kernel Code", 0, PERM_R | PERM_X};
    check_access(3, kcode, PERM_R);   // expect TRAP
    check_access(0, kcode, PERM_R);   // expect SUCCESS
    return 0;
}
