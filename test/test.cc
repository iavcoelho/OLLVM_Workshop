#include <stdio.h>

extern "C" void arithmetic() {
    volatile int a = 10;
    volatile int b = 5;

    int res_add = a + b; // Expected: 15
    int res_sub = a - b; // Expected: 5
    int res_xor = a ^ b; // Expected: 15
    int res_and = a & b; // Expected: 0
    int res_or  = a | b; // Expected: 15

    printf("ADD: %d + %d = %d\n", a, b, res_add);
    printf("SUB: %d - %d = %d\n", a, b, res_sub);
    printf("XOR: %d ^ %d = %d\n", a, b, res_xor);
    printf("AND: %d & %d = %d\n", a, b, res_and);
    printf("OR : %d | %d = %d\n", a, b, res_or);;
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
    
    arithmetic();

    return 0;
}
