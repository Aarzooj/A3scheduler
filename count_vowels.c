#include <stdio.h>
#include <string.h>

int main(){
    char vowels[]={'a','A','e','E','i','I','o','O','u','U'};
    char str[100] = "Hello how are you";
    int count = 0;
    for (int i=0;i<strlen(str);i++){
        if (strchr(vowels,str[i])){
            count++;
        }
    }
    printf("Count: %d\n",count);
    return 0;
}

// int main(){
//     char str[100],c;
//     printf("Enter string: ");
//     scanf("%[^\n]%*c",str);
//     printf("Enter character to check: ");
//     scanf("%c",&c);
//     if (strchr(str,c)){
//         printf("%c is in string",c);
//     }else{
//         printf("%c is not in string",c);
//     }
// }