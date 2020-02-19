#include <unistd.h>

int main() {
    int i;
    for (i = 0; i < 5; i++) {
        chdir("/");
    }

    return 0;
}