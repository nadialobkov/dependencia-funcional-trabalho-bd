#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "dpf.h"

#define MAX_BUFFER 100
#define MAX_DEPENDENCIAS 25


int get_atrib_index(dpf_t *dpf, char c) {
    for (int i = 0; i < dpf->quant_atrib; i++) {
        if (dpf->atrib[i] == c) {
            return i;
        }
    }
    return -1; // retorna -1 se o atributo nao for encontrado
}

// consome caracteres da entrada até encontrar o caractere alvo ou EOF
int pula_ate(FILE *entrada, char alvo) {
    int ch;
    while ((ch = fgetc(entrada)) != EOF) {
        if (ch == alvo) {
            return 1;
        }
    }
    return 0;
}

dpf_t *leitura_dpf(FILE *entrada) {

    // aloca memoria para a estrutura dpf_t
    dpf_t *dpf = malloc(sizeof(dpf_t));
    if (!dpf) {
        return NULL;
    }

    int i = 0; // contador
    char c; // caractere lido
    char *atrib_buff = malloc(MAX_BUFFER * sizeof(char)); // vetor de atributos
    
    // inicia leitura do conjunto de atributos
    if (!pula_ate(entrada, '{')) {
        free(atrib_buff);
        free(dpf);
        return NULL; // erro na leitura
    }
    while ((c = fgetc(entrada)) != '}') {
        if (c == EOF || i >= MAX_BUFFER - 1) {
            free(atrib_buff);
            free(dpf);
            return NULL; // erro na leitura
        }
        if (c != ',' && c !=';' && c != ' ') { // ignora virgulas, ponto e virgula e espacos
            atrib_buff[i] = c;
            i++;
        }
    }
    atrib_buff[i] = '\0'; // adiciona o caractere nulo ao final

    dpf->quant_atrib = i; 
    dpf->atrib = malloc((i + 1) * sizeof(char));
    strncpy(dpf->atrib, atrib_buff, i + 1); 

    free(atrib_buff);

    // aloca memoria para as matrizes de bitmaps
    dpf->lhs = malloc(MAX_DEPENDENCIAS * sizeof(char *));
    for (i = 0; i < MAX_DEPENDENCIAS; i++) {
        dpf->lhs[i] = malloc(dpf->quant_atrib * sizeof(char));
    }
    dpf->rhs = malloc(MAX_DEPENDENCIAS * sizeof(char *));
    for (i = 0; i < MAX_DEPENDENCIAS; i++) {
        dpf->rhs[i] = malloc(dpf->quant_atrib * sizeof(char));
    }

    // inicializa as matrizes de bitmaps com zeros
    for (i = 0; i < MAX_DEPENDENCIAS; i++) {
        for (int j = 0; j < dpf->quant_atrib; j++) {
            dpf->lhs[i][j] = 0;
            dpf->rhs[i][j] = 0;
        }
    }

    // inicia leitura do conjunto de dependencias funcionais
    if (!pula_ate(entrada, '{')) {
        libera_dpf(dpf);
        return NULL; // erro na leitura
    }
    i = 0; 
    bool left_side = true; // indica se estamos lendo o lado esquerdo ou direito da dependencia
    while ((c = fgetc(entrada)) != '}') {
        if (c == EOF || i >= MAX_DEPENDENCIAS) {
            libera_dpf(dpf);
            return NULL; // erro na leitura
        }
        if (c == ',' || c == ';') {
            left_side = true;
            i++; // passa para a proxima dependencia
            continue;
        }
        if (c == '-') {
            left_side = false; 
            pula_ate(entrada, '>'); // pula o caractere '>'
            continue;
        }
        if (c != ' ') {
            int index = get_atrib_index(dpf, c);
            if (index == -1) {
                // atributo nao encontrado, libera memoria e retorna NULL
                libera_dpf(dpf);
                return NULL;
            }
            if (left_side) {
                dpf->lhs[i][index] = 1;
            } else {
                dpf->rhs[i][index] = 1;  
            }
        }
    }

    dpf->quant_deps = i + 1; // atualiza a quantidade de dependencias lidas

    return dpf;
}

void libera_dpf(dpf_t *dpf) {

    if (dpf) {
        free(dpf->atrib);
        for (int i = 0; i < MAX_DEPENDENCIAS; i++) {
            free(dpf->lhs[i]);
            free(dpf->rhs[i]);
        }
        free(dpf->lhs);
        free(dpf->rhs);
        free(dpf);
    }
} 

void imprime_dpf(dpf_t *dpf) {
    printf("Atributos: ");
    for (int i = 0; i < dpf->quant_atrib; i++) {
        printf("%c ", dpf->atrib[i]);
    }
    printf("\nDependencias Funcionais:\n");
    for (int i = 0; i < dpf->quant_deps; i++) {
        bool has_dependency = false;
        for (int j = 0; j < dpf->quant_atrib; j++) {
            if (dpf->lhs[i][j] == 1 || dpf->rhs[i][j] == 1) {
                has_dependency = true;
                break;
            }
        }
        if (has_dependency) {
            printf("Dependencia %d: ", i + 1);
            for (int j = 0; j < dpf->quant_atrib; j++) {
                if (dpf->lhs[i][j] == 1) {
                    printf("%c", dpf->atrib[j]);
                }
            }
            printf(" -> ");
            for (int j = 0; j < dpf->quant_atrib; j++) {
                if (dpf->rhs[i][j] == 1) {
                    printf("%c", dpf->atrib[j]);
                }
            }
            printf("\n");
        }
    }
}