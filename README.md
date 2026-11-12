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

A estrutura escolhida para guardar o conjunto de dependências funcionais foi uma `struct` que armazena o vetor de atributos, a quantidade de atributos, a quantidade de dependencias funcionais, uma matriz com os atributos do lado esquerdo de todas as dependências funcionais e uma matriz contendo o lado direito. 

```c
typedef struct dpf_t {
    char *atrib;        // vetor de atributos (ex. [A, B, C])
    int quant_atrib;    // quantidade de atributos
    int quant_deps;     // quantidade de dependencias funcionais
    char **lhs;         // matriz dos determinantes
    char **rhs;         // matriz dos dependentes
} dpf_t;
```

Seja $a_i$ o elemento na posição $i$ do vetor `atrib`, temos as matrizes `lhs`e `rhs` de elementos $b_{ji}$ onde $j$ corresponde ao número da dependência funcional.

### Exemplo

Dada a seguinte entrada:

```txt
U = {A,B,C,D,E}
F = { A->BC, C->D, BD->E }
```

A estrutura `dpf_t` guarda os valores da seguinte maneira:

```txt
- atrib:          ABCDE

- quant_atrib:    5
- quant_deps:     3

- lhs:            A____
                  __C__
                  _B_D_

- rhs:            _BC__
                  ___D_
                  ____E  
```

> Obs: Os "_" estão escritos no exemplo apenas para melhor a visualização. Na prática, os vetores contêm espaços vazios (" ").

## Leitura da Entrada

A função `main` faz a análise dos argumentos passados na linha de comando, identifica o caminho para o arquivo de extensão `.fds` e chama a função `leitura_dpf` para fazer o *parsing* do arquivo e armazenar as informações na estrutura `dpf_t`.

```c
dpf_t *leitura_dpf(FILE *entrada);
```

## Fecho

Seja o fecho $X^+$. O cálculo do fecho segue a seguinte lógica já conhecida:

1. Criamos um vetor `fecho` com o mesmo tamanho da quantidade de atributos que vai guardar o fecho do conjunto $X$. Inicialmente, copiamos o conjunto $X$ direto no vetor de fecho, pois sabemos que $X \subseteq X^+$.
2. Percorremos as dependências funcionais.