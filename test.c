#include <stdio.h>

int main() {
    // char ch;
    // printf("ch: [%c][%d]\n", ch, ch);

    struct Data {
        int a[5];
    };

    int *elem;
    int a[5] = {1, 2, 3, 4, 5};
    struct Data data;
    memcpy(data.a, a, sizeof(int) * 5);
    printf("%d\n", data.a[4]);

    // elem = a;
    // printf("elem[4] = %d\n", elem[4]);

    return 0;
}