#include <stdio.h>

int main(){
    int n,factorial=1;
    n = 20;
    for (int i=1;i<=n;i++){
        factorial*=i;
    }
    printf("Factorial of %d = %d",n,factorial);
    return 0;
}