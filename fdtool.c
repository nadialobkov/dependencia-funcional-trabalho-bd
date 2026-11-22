#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "dpf.h"
#include "fdtool.h"


char *fecho(dpf_t *dpf, char *atributos) {

    // cria vetor para armazenar o fecho
    char *fecho = malloc((dpf->quant_atrib + 1) * sizeof(char));
    if (!fecho) {
        fprintf(stderr, "Erro de alocacao de memoria.\n");
        return NULL;
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
    return fecho;
}

dpf_t *cobertura_min(dpf_t *dpf) {

    // cria nova estrutura para armazenar a cobertura mínima
    dpf_t *cobertura = copia_dpf(dpf);
    if (!cobertura) {
        fprintf(stderr, "Erro de alocacao de memoria.\n");
        return NULL;
    }

    int contagem; // contador de atributos
 
    // 1. cada dependencia possui apenas um atributo no lado direito
    for (int i = 0; i < cobertura->quant_deps; i++) {
        contagem = 0;
        for (int j = 0; j < cobertura->quant_atrib; j++) {
            if (cobertura->rhs[i][j] != VAZIO) {
                contagem++; // contamos os atributos
                if (contagem > 1) {
                    // cria uma nova dependencia (adiciona uma linha na matriz)
                    for (int k = 0; k < cobertura->quant_atrib; k++) {
                        cobertura->lhs[cobertura->quant_deps][k] = cobertura->lhs[i][k];
                        cobertura->rhs[cobertura->quant_deps][k] = VAZIO;
                    }
                    // adiciona o atributo do lado direito a nova dependencia
                    cobertura->rhs[cobertura->quant_deps][j] = cobertura->rhs[i][j];
                    cobertura->quant_deps++;

                    // remove o atributo do lado direito da dependencia original
                    cobertura->rhs[i][j] = VAZIO;
                }
            }
        }
    }


    // 2. nao existem atributos desnecessarios no lado esquerdo das dependencias
    for (int i = 0; i < cobertura->quant_deps; i++) {
        contagem = 0;
        for (int j = 0; j < cobertura->quant_atrib; j++) {
            if (cobertura->lhs[i][j] != VAZIO) {
                contagem++;

                // ha mais de um atributo na esquerda
                if (contagem > 1) {
                    for (int k = 0; k < cobertura->quant_atrib; k++) {
                        if (cobertura->lhs[i][k] != VAZIO) {
                            // cria buffer para armazenar o atributo atual
                            char *buffer_atrib = malloc(sizeof(char) + 1);
                            buffer_atrib[0] = cobertura->lhs[i][k];
                            buffer_atrib[1] = '\0';
                            // calcula o fecho do atributo atual
                            char *fecho_temp = fecho(cobertura, buffer_atrib);

                            
                            if (contem_atributos(cobertura->rhs[i], fecho_temp)) {
                                // significa que atributo atual eh suficiente para determinar o lado direito
                                // logo podemos eliminar o resto dos atributos do lado esquerdo
                                for (int l = 0; l < cobertura->quant_atrib; l++) {
                                    if (l != k) {
                                        cobertura->lhs[i][l] = VAZIO;
                                    }
                                }
                            }

                            free(fecho_temp);
                        }
                    }
                }
            }
        }
    }


    // 3. nao existem dependencias redundantes
    for (int i = 0; i < cobertura->quant_deps; i++) {
        // cria uma copia da cobertura sem a dependencia atual
        dpf_t *copia_temp = copia_dpf(cobertura);
        if (!copia_temp) {
            fprintf(stderr, "Erro de alocacao de memoria.\n");
            libera_dpf(cobertura);
            return NULL;
        }
        // remove a dependencia atual da copia
        for (int j = 0; j < copia_temp->quant_atrib; j++) {
            copia_temp->lhs[i][j] = VAZIO;
            copia_temp->rhs[i][j] = VAZIO;
        }

        // calcula o fecho do lado esquerdo da dependencia atual na copia
        char *fecho_temp = fecho(copia_temp, cobertura->lhs[i]);
        if (contem_atributos(cobertura->rhs[i], fecho_temp)) {
            // significa que a dependencia atual eh redundante
            // logo podemos eliminar ela da cobertura
            for (int j = 0; j < cobertura->quant_atrib; j++) {
                cobertura->lhs[i][j] = VAZIO;
                cobertura->rhs[i][j] = VAZIO;
            }
        }

        libera_dpf(copia_temp);
        free(fecho_temp);
    }
    
    // remove as linhas vazias da cobertura resultantes do passo 3
    remove_vazios(cobertura);

    return cobertura;
}