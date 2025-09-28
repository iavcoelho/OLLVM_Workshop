#include <stdio.h>

extern "C" void my_function() {
    printf("This is the original function.\n");
}

int main() {
    my_function();
    return 0;
}
