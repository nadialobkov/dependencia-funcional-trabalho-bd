#ifndef DPF_H
#define DPF_H

#include <stdio.h>

// ESTRUTURA ------------------------------------------------------------------------------

typedef struct dpf_t {
    char *atrib;        // vetor de atributos (ex. [A, B, C])
    int quant_atrib;    // quantidade de atributos
    int quant_deps;     // quantidade de dependencias funcionais
    char **lhs;         // matriz de bitmaps dos determinantes
    char **rhs;         // matriz de bitmaps dos dependentes
} dpf_t;

// FUNCOES --------------------------------------------------------------------------------

// faz a leitura a partir da entrada padrao e retorna um ponteiro para a estrutura dpf_t
// ex. entrada esperada:
//     U = {A,B,C,D,E}
//     F = { A->BC, C->D, BD->E }
// retorna NULL em caso de erro
dpf_t *leitura_dpf(FILE *entrada);

// libera a memoria alocada para a estrutura dpf_t
void libera_dpf(dpf_t *dpf);

// imprime a estrutura dpf_t
void imprime_dpf(dpf_t *dpf);

// recebe um atributo e retorna o indice correspondente no vetor de atributos
// retorna -1 se o atributo nao for encontrado
int get_atrib_index(dpf_t *dpf, char c);


#endif