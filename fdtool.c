#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "dpf.h"
#include "fdtool.h"

// tipos para impressao da forma normal
#define FN3_OK 1
#define FN3_VIOLATION 2
#define BCNF_OK 3
#define BCNF_VIOLATION 4


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

chaves_t *chaves_candidatas(dpf_t *dpf) {

    chaves_t *candidatas = aloca_chaves(dpf->quant_atrib);

    // atributos que aparecem apenas do lado esquerdo das dependencias 
    // devem obrigatoriamente fazer parte de todas as chaves candidatas
    char *atrib_obrigatorios = malloc((dpf->quant_atrib + 1) * sizeof(char));
    if (!atrib_obrigatorios) {
        fprintf(stderr, "Erro de alocacao de memoria.\n");
        libera_chaves(candidatas);
        return NULL;
    }
    strncpy(atrib_obrigatorios, dpf->atrib, dpf->quant_atrib + 1);

    // percorre deps para identificar atributos obrigatorios
    for (int i = 0; i < dpf->quant_deps; i++) {
        for (int j = 0; j < dpf->quant_atrib; j++) {
            if (dpf->rhs[i][j] != VAZIO) {
                // remove o atributo do lado direito da lista de obrigatorios
                int index = get_atrib_index(dpf, dpf->rhs[i][j]);
                if (index != -1) {
                    atrib_obrigatorios[index] = VAZIO;
                }
            }
        }
    }

    // verificamos se eh chave candidata por si so
    char *fecho_obrigatorios = fecho(dpf, atrib_obrigatorios);
    if (atributos_iguais(fecho_obrigatorios, dpf->atrib)) {
        // significa que os atributos obrigatorios sao suficientes para determinar todos os atributos
        strncpy(candidatas->chaves[candidatas->quant], atrib_obrigatorios, dpf->quant_atrib + 1);
        candidatas->quant++;
    }
    else {
        // para cada atributo que nao eh obrigatorio, verificamos se ele pode ser adicionado a chave candidata
        for (int i = 0; i < dpf->quant_atrib; i++) {
            if (atrib_obrigatorios[i] == VAZIO) {
                // cria um novo conjunto de atributos adicionando o atributo atual aos obrigatorios
                char *novo_conjunto = malloc((dpf->quant_atrib + 1) * sizeof(char));
                if (!novo_conjunto) {
                    fprintf(stderr, "Erro de alocacao de memoria.\n");
                    libera_chaves(candidatas);
                    free(atrib_obrigatorios);
                    return NULL;
                }
                strncpy(novo_conjunto, atrib_obrigatorios, dpf->quant_atrib + 1);
                // adiciona o atributo atual ao novo conjunto
                novo_conjunto[i] = dpf->atrib[i];
                novo_conjunto[dpf->quant_atrib] = '\0';

                // calcula o fecho do novo conjunto
                char *fecho_novo = fecho(dpf, novo_conjunto);
                if (atributos_iguais(fecho_novo, dpf->atrib)) {
                    // significa que o novo conjunto eh chave candidata
                    strncpy(candidatas->chaves[candidatas->quant], novo_conjunto, dpf->quant_atrib + 1);
                    candidatas->quant++;
                }
                free(fecho_novo);
                free(novo_conjunto);
            }
        }

    }
    free(fecho_obrigatorios);
    free(atrib_obrigatorios);

    return candidatas;
}

void imprime_forma_normal(char *lhs, char *rhs, int tipo) {

    switch (tipo) {
        case FN3_OK:
            printf("3FN: OK\n");
            break;
        case FN3_VIOLATION:
            printf("VIOLATION 3FN: ");
            imprime_dep(lhs, rhs);
            printf(" (");
            imprime_atrib(rhs);
            printf(" not prime, ");
            imprime_atrib(lhs);
            printf(" not superkey)\n");
            break;
        case BCNF_OK:
            printf("BCNF: OK\n");
            break;
        case BCNF_VIOLATION:
            printf("VIOLATION BCNF: ");
            imprime_dep(lhs, rhs);
            printf(" (");
            imprime_atrib(lhs);
            printf(" not superkey)\n");
            break;
        default:
            fprintf(stderr, "Erro: tipo de forma normal desconhecido.\n");
    }
}


void forma_normal(dpf_t *dpf) {

    bool fn3 = true; // flag para indicar se a dpf esta na 3FN
    bool bcnf = true; // flag para indicar se a dpf esta na BCNF

    // vamos assumir que a dpf esta em 1FN sempre, pois nao ha multivalorados nem atributos compostos
    // verificamos se a dpf esta em 2FN

    // calculamos as chaves candidatas da dpf
    chaves_t *candidatas = chaves_candidatas(dpf);
    if (!candidatas) {
        fprintf(stderr, "Erro de alocacao de memoria.\n");
        return;
    }

    // verificacao de 2FN

    // se ha uma unica chave candidata nao composta, a dpf esta em 2FN
    if (candidatas->quant == 1 && strlen(candidatas->chaves[0]) == 1) {
        // dpf esta em 2FN
        libera_chaves(candidatas);
    } else {
        // analizamos cada dependencia funcional para verificar se ha dependencias parciais
        for (int i = 0; i < dpf->quant_deps; i++) {
            // verificamos se o lado esquerdo da dependencia que contem atributos nao-prmimos
            // do lado direito eh um subconjunto de alguma chave

            char *nao_primos = atributos_nao_primos(dpf, candidatas);
            if (contem_atributos(dpf->rhs[i], nao_primos)) {
                // significa que o lado direito da dependencia contem atributos nao-primos
                // logo verificamos se o lado esquerdo eh um subconjunto de alguma chave candidata
                for (int j = 0; j < candidatas->quant; j++) {
                    if (contem_atributos(dpf->lhs[i], candidatas->chaves[j]) && !atributos_iguais(candidatas->chaves[j], dpf->lhs[i])) {
                        // significa que o lado esquerdo da dependencia eh um subconjunto de uma
                        fn3 = false;
                        break;
                    }
                }
            }
        }
    }

    // verificacao de 3FN

    if (fn3) {
        // se a dpf esta em 2FN, verificamos se ha dependencias transitivas
        for (int i = 0; i < dpf->quant_deps; i++) {
            // verificamos se o lado direito da dependencia contem atributos nao-primos
            char *nao_primos = atributos_nao_primos(dpf, candidatas);
            if (contem_atributos(dpf->rhs[i], nao_primos)) {
                // significa que o lado direito da dependencia contem atributos nao-primos
                // logo verificamos se o lado esquerdo eh uma chave candidata
                bool lhs_eh_chave = false;
                for (int j = 0; j < candidatas->quant; j++) {
                    if (atributos_iguais(candidatas->chaves[j], dpf->lhs[i])) {
                        lhs_eh_chave = true;
                    }
                }
                if (!lhs_eh_chave) {
                    // significa que o lado esquerdo da dependencia nao eh uma chave candidata
                    fn3 = false;
                    imprime_forma_normal(dpf->lhs[i], dpf->rhs[i], FN3_VIOLATION);
                }
            }
        }
    }

    if (fn3) {
        imprime_forma_normal(NULL, NULL, FN3_OK);
    }

    // verificacao de BCNF
    // se a dpf esta em 3FN, verificamos se ha dependencias que violam a BCNF
    for (int i = 0; i < dpf->quant_deps; i++) {
        // verificamos se o lado esquerdo da dependencia eh uma superchave
        char *fecho_lhs = fecho(dpf, dpf->lhs[i]);
        if (!atributos_iguais(fecho_lhs, dpf->atrib)) {
            // significa que o lado esquerdo da dependencia nao eh uma superchave
            bcnf = false;
            imprime_forma_normal(dpf->lhs[i], dpf->rhs[i], BCNF_VIOLATION);
            free(fecho_lhs);
        }
    }

    if (bcnf) {
        imprime_forma_normal(NULL, NULL, BCNF_OK);
    }
}