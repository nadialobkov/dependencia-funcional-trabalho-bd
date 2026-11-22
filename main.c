#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dpf.h"
#include "fdtool.h"


int main(int argc, char *argv[]) {

    // leitura do arquivo de dependencias funcionais
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

    // criacão da estrutura dpf_t a partir do arquivo de entrada
    dpf_t *dpf = leitura_dpf(entrada);
    if (!dpf) {
        fprintf(stderr, "Erro na leitura do DPF.\n");
        return EXIT_FAILURE;
    }


    // identificação do comando a ser executado
    if (strcmp(argv[1], "closure") == 0) {
        if (argc < 6 || strcmp(argv[4], "--X") != 0) {
            fprintf(stderr, "Uso: %s closure --fds <arquivo.fds> --X <atributos>\n", argv[0]);
            libera_dpf(dpf);
            fclose(entrada);
            return EXIT_FAILURE;
        }
        char *atributos = argv[5];
        char *fecho_resultado = fecho(dpf, atributos);
        imprime_atributos(fecho_resultado);
        free(fecho_resultado);
    } 
    else if (strcmp(argv[1], "mincover") == 0) {
        dpf_t *min_cover = cobertura_min(dpf);
        imprime_dependencias(min_cover);
        libera_dpf(min_cover);
    } 
    else if (strcmp(argv[1], "keys") == 0) {
        chaves_t *candidatas = chaves_candidatas(dpf);
        imprime_chaves(candidatas);
        libera_chaves(candidatas);
    } 
    else if (strcmp(argv[1], "normalform") == 0) {
        forma_normal(dpf);
    } 
    else {
        fprintf(stderr, "Comando desconhecido: %s\n", argv[1]);
        libera_dpf(dpf);
        fclose(entrada);
        return EXIT_FAILURE;
    }


    libera_dpf(dpf);
    fclose(entrada);

    return 0;
}