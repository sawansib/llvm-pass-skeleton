#include <stdio.h>

int main(int argc, const char** argv) {
    int num, j;
    int a[30] = {1, 1, 1, 1, 1, 1,
                 1, 1, 2, 3, 4, 6,
                 1, 1, 2, 3, 4, 6,
                 1, 1, 2, 3, 4, 6,
                 1, 1, 2, 3, 4, 6};
    scanf("%i", &num);
    printf("%i\n", num + 2);
    printf("%i\n", num + 4);
    for (int i = 0; i < 30; i++) {
      j = 3 * i + 1; // <i,3,1> 
      a[j] = a[j] - 2;
      i = i + 2;
    }
    // printf("%i\n", a[29]);
    return 0;
}
