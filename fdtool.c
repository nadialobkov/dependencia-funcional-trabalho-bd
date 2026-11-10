#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "dpf.h"
#include "fdtool.h"


void fecho(dpf_t *dpf, char *atributos) {

    // cria vetor para armazenar o fecho
    char *fecho = malloc((dpf->quant_atrib + 1) * sizeof(char));
    if (!fecho) {
        fprintf(stderr, "Erro de alocacao de memoria.\n");
        return;
    }
    memset(fecho, VAZIO, dpf->quant_atrib);
    fecho[dpf->quant_atrib] = '\0';

    // inicializa o fecho com os atributos fornecidos
    for (int j = 0; atributos[j] != '\0'; j++) {
        int index = get_atrib_index(dpf, atributos[j]);
        if (index != -1) {
            fecho[index] = atributos[j];
        }
    }

    // percorre as dependencias funcionais 
    bool mudou = true;
    while (mudou) {
        mudou = false;
        for (int i = 0; i < dpf->quant_deps; i++) {
            // verifica se o lado esquerdo da dependencia esta contido no fecho
            if (contem_atributos(dpf->lhs[i], fecho)) {
                // adiciona os atributos do lado direito ao fecho
                for (int j = 0; j < dpf->quant_atrib; j++) {
                    if (dpf->rhs[i][j] != VAZIO && fecho[j] == VAZIO) {
                        fecho[j] = dpf->rhs[i][j];
                        mudou = true;
                    }
                }
            }
        }
    }

    // imprime o fecho
    printf("Fecho: %s\n", fecho);
    free(fecho);
}