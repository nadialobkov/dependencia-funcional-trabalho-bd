#ifndef DPF_H
#define DPF_H

// ESTRUTURA ------------------------------------------------------------------------------

typedef struct dpf_t {
    char *atrib;    // vetor de atributos (ex. [A, B, C])
    int quant;      // quantidade de atributos
    char **lhs;     // matriz de bitmaps dos determinantes
    char **rhs;     // matriz de bitmaps dos dependentes
} dpf_t;

// FUNCOES --------------------------------------------------------------------------------

// faz a leitura a partir da entrada padrao e retorna um ponteiro para a estrutura dpf_t
// ex. entrada esperada:
//     U = {A,B,C,D,E}
//     F = { A->BC, C->D, BD->E }
// retorna NULL em caso de erro
dpf_t *cria_dpf();

// libera a memoria alocada para a estrutura dpf_t
void libera_dpf(dpf_t *dpf);

#endif