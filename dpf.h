#ifndef DPF_H
#define DPF_H

#include <stdio.h>

#define VAZIO ' '
#define MAX_BUFFER 150          // tamanho maximo do buffer para leitura das linhas do arquivo
#define MAX_DEPENDENCIAS 25     // quantidade maxima de dependencias de uma dpf

// ESTRUTURA ------------------------------------------------------------------------------

typedef struct dpf_t {
    char *atrib;        // vetor de atributos (ex. [A, B, C])
    int quant_atrib;    // quantidade de atributos
    int quant_deps;     // quantidade de dependencias funcionais
    char **lhs;         // matriz dos determinantes
    char **rhs;         // matriz dos dependentes
} dpf_t;


typedef struct chaves_t {
    char **chaves;      // matriz de chaves candidatas
    int quant;          // quantidade de chaves candidatas
} chaves_t;

// FUNCOES --------------------------------------------------------------------------------

// faz a leitura a partir da entrada padrao e retorna um ponteiro para a estrutura dpf_t
// ex. entrada esperada:
//     U = {A,B,C,D,E}
//     F = { A->BC, C->D, BD->E }
// retorna NULL em caso de erro
dpf_t *leitura_dpf(FILE *entrada);

// libera a memoria alocada para a estrutura dpf_t
void libera_dpf(dpf_t *dpf);

// retorna uma copia de uma dpf
dpf_t *copia_dpf(dpf_t *dpf);

// imprime a estrutura dpf_t
void imprime_dpf(dpf_t *dpf);

// imprime apenas as dependencias funcionais da estrutura dpf_t
void imprime_dependencias(dpf_t *dpf);

// imprime atributos formatados
void imprime_atributos(char *atributos);

// imprime uma unica dependencia funcional formatada
void imprime_dep(char *lhs, char *rhs);

// imprime um conjunto de atributos formatado
void imprime_atrib(char *atributos);

// recebe uma dpf e limpa as linhas que tem dependencias vazias (sem determinantes e dependentes)
// ajusta a quantidade de dependencias da dpf e posicão das linhas validas
void remove_vazios(dpf_t *dpf);

// recebe uma dpf e ordena as linhas das dependencias de acordo com a ordem dos atributos
void ordena_dependencias(dpf_t *dpf);

// recebe um atributo e retorna o indice correspondente no vetor de atributos
// retorna -1 se o atributo nao for encontrado
int get_atrib_index(dpf_t *dpf, char c);

// verifica se todos os atributos de um conjunto estao presentes em outro conjunto
// retorna 1 se todos os atributos estiverem presentes, 0 caso contrario
int contem_atributos(char *atributos, char *conjunto);

// verifica se dois conjuntos de atributos sao iguais
// retorna 1 se os conjuntos forem iguais, 0 caso contrario
int atributos_iguais(char *atributos1, char *atributos2);

// retorna vetor de atributos nao-primos da dpf
// ou seja, atributos que nao fazem parte de nenhuma chave candidata
char *atributos_nao_primos(dpf_t *dpf, chaves_t *candidatas);

// recebe um conjunto de atributos e um bitmap e retorna uma string com os atributos correspondentes
char *get_atributos(char *atributos, char *bitmap);

// aloca espaço na memoria para a estrutura chaves_t
// recebe quantidade de atributos refernte a dpf relacionada
chaves_t *aloca_chaves(int atributos);

// libera a memoria alocada para a estrutura chaves_t
void libera_chaves(chaves_t *chaves);

// imprime a estrutura chaves_t formatada
void imprime_chaves(chaves_t *chaves);

#endif