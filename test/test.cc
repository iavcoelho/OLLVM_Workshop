#include <stdio.h>

extern "C" int mov() {
    // volatile to prevent optimization
    volatile int result;
    __asm__(
        "movl $42, %0"
        : "=r"(result) // "=r" means the result will be in a general-purpose register
    );
    return result;
}

extern "C" int add(int a, int b) {
    return a + b;
}

extern "C" void my_function() {
    // volatile to prevent optimization
    volatile int result = add(2, 1);

    printf("2 + 1 = %d\n", result);
}

int main() {
    my_function();
    int val = mov();
    printf("Mov result: %d\n", val);
    return 0;
}
