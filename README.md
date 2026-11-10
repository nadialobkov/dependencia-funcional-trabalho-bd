# Trabalho Banco de Dados - Dependência Funcional

- Nadia Luana Lobkov (GRR20232381)

## Objetivo

Implementar, em linguagem C, uma ferramenta de linha de comando (fdtool) para manipular Dependências Funcionais (DFs) em esquemas relacionais com atributos no intervalo A..Z.

A ferramenta deverá implementar:

1. cálculo do fecho X+ de um conjunto de atributos X ⊆ U, dado um conjunto de dependências
funcionais F;
2. cálculo da cobertura mínima de F, incluindo:
   - RHS unitário;
   - remoção de atributos estranhos;
   - remoção de DFs redundantes;
3. identificação de todas as chaves candidatas, isto é, todos os conjuntos mínimos K tais que
K+ = U;
4. verificação das formas normais BCNF e 3FN.


## Estrutura

A estrutura escolhida para guardar o conjunto de dependências funcionais foi a seguinte:

```c
typedef struct dpf_t {
    char *atrib;        // vetor de atributos (ex. [A, B, C])
    int quant_atrib;    // quantidade de atributos
    int quant_deps;     // quantidade de dependencias funcionais
    char **lhs;         // matriz dos determinantes
    char **rhs;         // matriz dos dependentes
} dpf_t;
```

Então se a entrada recebida for:

```txt
U = {A,B,C,D,E}
F = { A->BC, C->D, BD->E }
```

A estrutura `dpf_t` guarda os valores da seguinte maneira:

| **Campo**     | **Valor**|
|---------------+----------|
| `atrib`       | ABCDE    |
| `quant_atrib` | 5        |
| `quant_deps`  | 3        |