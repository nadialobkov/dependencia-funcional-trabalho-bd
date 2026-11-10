#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dpf.h"

#define CLOSURE 1
#define MINCOVER 2
#define KEYS 3
#define NORMALFORM 4

int main(int argc, char *argv[]) {

    char *caminho = NULL;
    for (int i = 1; i < argc - 1; i++) {
        if (strcmp(argv[i], "--fds") == 0) {
            caminho = argv[i + 1];
            i++; // pula o próximo argumento, que é o caminho do arquivo
        }
    }
    if (caminho == NULL) {
        fprintf(stderr, "Uso: %s <comando> --fds <arquivo.fds> [--X ATRIBUTOS]\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *entrada = fopen(caminho, "r");
    if (!entrada) {
        fprintf(stderr, "Erro ao abrir o arquivo %s.\n", caminho);
        return EXIT_FAILURE;
    }

    dpf_t *dpf = leitura_dpf(entrada);
    if (!dpf) {
        fprintf(stderr, "Erro na leitura do DPF.\n");
        return EXIT_FAILURE;
    }

    imprime_dpf(dpf);
    libera_dpf(dpf);
    fclose(entrada);

    return 0;
}