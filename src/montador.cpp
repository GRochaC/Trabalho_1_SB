#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "ctype.h"

#define bool int
#define true 1
#define false 0

int main(int argc, char* argv[]) {
    if(argc != 2) return -1;

    FILE* file_ptr = fopen(argv[1], "r");

    if(file_ptr == NULL) return -1;
    else {
        char* linha = (char*) malloc(sizeof(char) * 100);
        char** codigo = (char**) malloc(200000 * sizeof(char *));
        int total_linhas = 0;
        while(fgets(linha,100,file_ptr) != NULL) {
            *(linha + strcspn(linha,"\n")) = '\0';

            for(char *ptr = linha; *ptr != '\0'; ptr++) {
                *ptr = toupper(*ptr);
                
                if(*ptr == ';') {
                    *ptr = '\0';
                    break;
                }

            }

            codigo[total_linhas] = (char*) malloc(sizeof(linha));
            strcpy(codigo[total_linhas],linha);

            total_linhas++;
        }

        free(linha);

        fclose(file_ptr);


        free(codigo);
    }

    return 0;
}