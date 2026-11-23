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

## Organização do projeto

O projeto está organizado da seguinte forma:

```txt
├── dpf.c                   # implementação de funções auxiliares da estrutura dpf_t e chaves_t
├── dpf.h                   # declarações de funções e estruturas
├── exemplos/               # exemplos criados para testar casos específicos
│   └── ...
├── fdtool.c                # implementação dos algoritmos principais
├── fdtool.h                # declarações
├── main.c                  # função principal
├── makefile
├── README.md               # relatório
└── testes_normalform/      # entradas fornecidas
    └── ...
```

- `main.c`: ponto de entrada do programa. Contém apenas a função `main`, que trata os argumentos da linha de comando, chama a leitura do arquivo e executa o comando escolhido.
- `dpf.h` e `dpf.c`: definição das estruturas `dpf_t` e `chaves_t` e tudo o que se refere a elas: leitura do arquivo `.fds`, alocação, cópia e liberação de memória, funções de impressão e operações auxiliares sobre conjuntos de atributos (`contem_atributos`, `atributos_iguais`, `atributos_nao_primos`, `get_atrib_index`, `remove_vazios`).
- `fdtool.h` e `fdtool.c`: implementação dos algoritmos da ferramenta: `fecho`, `cobertura_min`, `chaves_candidatas` e `forma_normal`.
- `makefile`: compilação do projeto.
- `README.md`: este relatório.

## Compilação e Execução

A compilação é feita com o comando `make`, que gera o executável `fdtool`. O comando `make clean` remove o executável.

```txt
./fdtool closure --fds <arquivo.fds> --X <atributos>
./fdtool mincover --fds <arquivo.fds>
./fdtool keys --fds <arquivo.fds>
./fdtool normalform --fds <arquivo.fds>
```

O arquivo `.fds` possui o conjunto de atributos e o conjunto de dependências funcionais, como no exemplo abaixo:

```txt
U = {A,B,C,D,E}
F = { A->BC, C->D, BD->E }
```

## Estrutura

### `dpf_t`

A estrutura escolhida para guardar o conjunto de dependências funcionais foi uma `struct` que armazena o vetor de atributos, a quantidade de atributos, a quantidade de dependencias funcionais, uma matriz com os atributos do lado esquerdo de todas as dependências funcionais e uma matriz contendo o lado direito. 

O vetor de atributos é alocado de maneira que contenha o espaço exato para armazenar os atributos da relação, visto que estes não mudam. A matriz de dependências (lado esquerdo e direito) tem suas colunas com espaço exato para os possíveis atributos, porém a diferença é que a quantidade de linhas é definida pelo valor em `MAX_DEPENDENCIAS`. A ideia é que esse valor seja maior que as dependências existentes, pois em cálculos de cobertura mínima linhas novas podem ser adicionadas.

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

#### Exemplo

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

### `chaves_t`

De maneira análoga, há uma outra estrutura chamada `chaves_t` que foi criada para armazenar os pares de chaves candidatas. Ela consiste em uma matriz `MAX_DEPENDENCIAS` $\times$ `quant_atrib` e uma variável que guarda a quantidade de linhas que estão sendo usadas na prática.

```c
typedef struct chaves_t {
    char **chaves;      // matriz de chaves candidatas
    int quant;          // quantidade de chaves candidatas
} chaves_t;
```

#### Exemplo

Dada a seguinte entrada:

```txt
U = {A,B,C,D,E}
F = { AB->C, AC->B, BC->A }
```

A estrutura `chaves_t` guarda os valores da seguinte maneira:

```txt
- atrib:          ABCDE

- quant:          5

- lhs:            A____
                  __C__
                  _B_D_
                  _____
                  _____  
                  ...
```


## Leitura da Entrada

A função `main` faz a análise dos argumentos passados na linha de comando, identifica o caminho para o arquivo de extensão `.fds` e chama a função `leitura_dpf` para fazer o *parsing* do arquivo e armazenar as informações na estrutura `dpf_t`.

```c
dpf_t *leitura_dpf(FILE *entrada);
```
## Função `main`

A `main` não implementa nenhum algoritmo, ela apenas coordena o parsing da entrada do programa e chamada das funcões.


## Fecho

Seja o fecho $X^+$. O cálculo do fecho segue a seguinte lógica já conhecida:

1. Criamos um vetor `fecho` com o mesmo tamanho da quantidade de atributos que vai guardar o fecho do conjunto $X$. Inicialmente, copiamos o conjunto $X$ direto no vetor de fecho, pois sabemos que $X \subseteq X^+$.
2. Percorremos as dependências funcionais e usamos uma flag `mudou` para sabermos quando parar.
3. Em cada iteração, vemos se o lado esquerdo está contido no fecho atual, se sim adicionamos o lado direito dentro do fecho.
4. Se finalizarmos uma varredura completa e não houver mudanças no fecho, encerramos o cálculo.

#### Exemplo

```txt
U = {A,B,C,D,E}
F = { A->BC, C->D, BD->E }
```

```txt
$ ./fdtool closure --fds entrada.fds --X A
ABCDE
```

O fecho começa com `A`. Na primeira varredura, `A->BC` adiciona `B` e `C`, `C->D` adiciona `D` e `BD->E` adiciona `E`. Na segunda varredura nada muda, então o cálculo termina.

#### Complexidade

Sejam $n$ a quantidade de atributos e $m$ a quantidade de dependências. A verificação de contenção (`contem_atributos`) custa $O(n^2)$, pois para cada atributo é feita uma busca com `strchr` no conjunto. Cada varredura percorre as $m$ dependências, então custa $O(m \cdot n^2)$. Como uma varredura só se repete se pelo menos um atributo foi adicionado ao fecho, são feitas no máximo $n$ varreduras (mais uma para confirmar que nada mudou), resultando em $O(m \cdot n^3)$ no pior caso.

Como o alfabeto é limitado a A..Z, o custo na prática é pequeno. O fecho é a operação base dos demais algoritmos, então seu custo se multiplica em todos eles.

## Cobertura Mínima

Para o cálculo da cobertura mínima usamos uma **cópia da estrutura original**, assim poderiamos "excluir" linhas (tornar elas vazias) sem alterar nossas dependências originais. Assim que ela satisfazer as condições da cobertura mínima, salvamos o estado atual correto na estrutura `dpf_t *cobertura`.

1. **Cada dependência possui apenas um atributo no lado direito:** Iteramos pela lista de atributos do lado direito e contamos estes. Se a contagem for maior que um, preenchemos uma nova linha com esse atributo e excluimos ele da linha atual.
2. **Não existem atributos desnecessários:** Para as dependências com mais de um atributo no lado esquerdo, iteramos pelos atributos do lado esquerdo e criamos um buffer para armazenar o atributo atual analizado. Calculamos o fecho apenas desse atributo. Caso o lado direito estiver contido no fecho, isso significa que o atributo sozinho já determina o lado direito, logo os demais atributos do lado esquerdo são desnecessários e são excluídos.
3. **Não existem dependências redundantes:** Para cada dependência, criamos uma cópia da cobertura atual sem ela e calculamos o fecho do seu lado esquerdo nessa cópia. Se o lado direito estiver contido no fecho, a dependência pode ser obtida pelas demais e portanto é redundante, então ela é excluída (a linha fica vazia). Ao final, a função `remove_vazios` retira as linhas vazias e ajusta a quantidade de dependências.

#### Exemplo

```txt
U = {A,B,C}
F = { A->BC, B->C, AB->C }
```

```txt
$ ./fdtool mincover --fds entrada.fds
A -> B
B -> C
```

No passo 1, `A->BC` é dividida em `A->B` e `A->C`. No passo 2, `AB->C` vira `A->C`, pois o fecho de `A` já contém `C`. No passo 3, as duas cópias de `A->C` são redundantes, já que `A->B` e `B->C` implicam `A->C`, e sobram apenas `A->B` e `B->C`.


#### Complexidade

Seja $m'$ a quantidade de dependências após o passo 1 (limitada por `MAX_DEPENDENCIAS`). O passo 1 percorre cada linha e cada atributo, custando $O(m' \cdot n)$. No passo 2, para cada dependência são calculados fechos de atributos individuais, e o cálculo é repetido a cada atributo do lado esquerdo a partir do segundo, o que dá $O(n^2)$ fechos por dependência, ou seja, $O(m'^2 \cdot n^5)$ no total. No passo 3, é feita uma cópia da estrutura e um fecho por dependência, custando $O(m'^2 \cdot n^3)$.

O passo 2 domina o custo, mas o limite é polinomial e folgado: como $n \le 26$ e $m'$ é limitado, o tempo de execução na prática é desprezível. A remoção das linhas vazias é linear, $O(m' \cdot n)$.

## Chaves Candidatas

Uma chave candidata é um conjunto mínimo $K$ tal que $K^+ = U$. O algoritmo se baseia na seguinte observação: atributos que **nunca aparecem do lado direito** de nenhuma dependência não podem ser determinados por outros atributos, então fazem parte de todas as chaves. Estes são os atributos obrigatórios.

> **Limitação:** a implementação atual testa apenas o conjunto dos atributos obrigatórios e esse conjunto acrescido de **um único** atributo. Ela não faz todas as combinações possíveis dos atributos restantes. Por isso, chaves que precisam de **dois ou mais** atributos além dos obrigatórios **não são encontradas**.

O algoritmo segue estes passos:

1. Criamos um vetor `atrib_obrigatorios` copiando todos os atributos da relação e removemos dele os que aparecem do lado direito de alguma dependência.
2. Calculamos o fecho dos atributos obrigatórios. Se ele for igual a $U$, os obrigatórios já determinam todos os atributos. Como toda chave contém os obrigatórios, eles formam a única chave candidata e o algoritmo termina.
3. Caso contrário, para cada atributo que não é obrigatório, criamos um novo conjunto (`novo_conjunto`) com os obrigatórios mais esse atributo e calculamos o fecho. Se o fecho for igual a $U$, o conjunto é uma chave candidata e é guardado na estrutura `chaves_t`.

Não é necessário verificar a minimalidade, pois os obrigatórios sozinhos não são chave (passo 2), então nenhum subconjunto próprio de `novo_conjunto` determina $U$.

#### Exemplo

```txt
U = {A,B,C,D}
F = { AB->C, C->D, D->A }
```

```txt
$ ./fdtool keys --fds entrada.fds
AB
BC
BD
```

`B` é o único atributo que nunca aparece do lado direito, então é obrigatório, e seu fecho é apenas `B`. Testando um atributo por vez, o fecho de `AB`, `BC` e `BD` é igual a $U$, e `A`, `C` e `D` completam a chave. Neste caso o resultado está correto, pois todas as chaves da relação têm exatamente um atributo além do obrigatório.

#### Exemplo em que a limitação aparece

```txt
U = {A,B,C,D}
F = { A->B, B->A, C->D, D->C }
```

```txt
$ ./fdtool keys --fds entrada.fds

```

Todos os atributos aparecem do lado direito, então não há obrigatórios. O fecho do conjunto vazio não é igual a $U$ e o fecho de qualquer atributo sozinho também não (`A` determina apenas `AB`, por exemplo), logo nada é impresso. As chaves corretas são `AC`, `AD`, `BC` e `BD`, que têm dois atributos além dos obrigatórios (nenhum) e por isso não são testadas.

#### Complexidade

Seja $n$ a quantidade de atributos, $m$ a quantidade de dependências e $p$ a quantidade de atributos não obrigatórios. Descobrir os obrigatórios custa $O(m \cdot n)$. Depois são calculados um fecho dos obrigatórios e, no máximo, $p$ fechos dos conjuntos com um atributo a mais, cada um custando $O(m \cdot n^3)$. O custo total é $O(p \cdot m \cdot n^3)$, ou seja, $O(m \cdot n^4)$ no pior caso, polinomial. Apenas $p + 1$ conjuntos são testados.

O algoritmo completo precisa testar todas as combinações dos $p$ atributos não obrigatórios em ordem crescente de tamanho, o que dá $2^p$ conjuntos. Para cada um é calculado um fecho ($O(m \cdot n^3)$) e feita a comparação com as $q$ chaves já encontradas ($O(q \cdot n^2)$), resultando em $O(2^p \cdot (q \cdot n^2 + m \cdot n^3))$, exponencial em $p$. Com $p = 3$ seriam 8 conjuntos contra 4 da implementação atual, e com $p = 20$ seriam mais de um milhão contra 21.


## Formas Normais

A verificação das formas normais usa as chaves candidatas calculadas na seção anterior. Assumimos que a relação sempre está na 1FN, pois não há atributos multivalorados ou compostos.

1. **2FN:** Se há uma única chave candidata com apenas um atributo, não há dependências parciais. Caso contrário, para cada dependência cujo lado direito contém apenas atributos não-primos (que não pertencem a nenhuma chave candidata, `atributos_nao_primos`), verificamos se o lado esquerdo é um subconjunto próprio de alguma chave candidata. Se sim, há uma dependência parcial.
2. **3FN:** Para cada dependência cujo lado direito contém atributos não-primos, verificamos se o lado esquerdo é uma chave candidata. Se não for, há uma dependência transitiva e a violação é impressa.
3. **BCNF:** Para cada dependência, calculamos o fecho do lado esquerdo. Se ele não for igual a $U$, o lado esquerdo não é superchave e a violação é impressa.

Se nenhuma violação é encontrada, é impresso `3FN: OK` e/ou `BCNF: OK`. A impressão é feita pela função `imprime_forma_normal`, que recebe o tipo de mensagem (`FN3_OK`, `FN3_VIOLATION`, `BCNF_OK` ou `BCNF_VIOLATION`).

#### Exemplo

```txt
U = {A,B,C}
F = { AB->C, C->B }
```

```txt
$ ./fdtool normalform --fds entrada.fds
3FN: OK
VIOLATION BCNF: C->B (C not superkey)
```

As chaves candidatas são `AB` e `AC`, então todos os atributos são primos e a relação está na 3FN. Na BCNF, o fecho de `C` é `BC`, diferente de $U$, logo `C->B` viola a BCNF. A dependência `AB->C` não viola, pois o fecho de `AB` é $U$.

#### Complexidade

O custo é dominado pelo cálculo das chaves candidatas, $O(2^p \cdot (q \cdot n^2 + m \cdot n^3))$. As verificações de 2FN e 3FN percorrem as $m$ dependências e, em cada uma, recalculam os atributos não-primos ($O(q \cdot n^2)$) e comparam o lado esquerdo com as $q$ chaves ($O(q \cdot n)$), totalizando $O(m \cdot q \cdot n^2)$. A verificação de BCNF calcula um fecho por dependência, custando $O(m^2 \cdot n^3)$. Portanto, o custo total das formas normais é exponencial apenas por causa das chaves candidatas, sendo as demais etapas polinomiais.
