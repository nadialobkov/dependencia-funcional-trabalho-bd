#include <stdio.h>
#include <stdlib.h>
#include "dpf.h"

int main() {
    dpf_t *dpf = leitura_dpf();
    if (!dpf) {
        fprintf(stderr, "Erro na leitura do DPF.\n");
        return EXIT_FAILURE;
    }

    imprime_dpf(dpf);
    libera_dpf(dpf);

    return 0;
}