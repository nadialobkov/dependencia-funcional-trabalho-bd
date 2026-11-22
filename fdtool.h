#ifndef FDTOOL_H
#define FDTOOL_H

#include "dpf.h"


char *fecho(dpf_t *dpf, char *atributos);

dpf_t *cobertura_min(dpf_t *dpf);

chaves_t *chaves_candidatas(dpf_t *dpf);

void forma_normal(dpf_t *dpf);

#endif