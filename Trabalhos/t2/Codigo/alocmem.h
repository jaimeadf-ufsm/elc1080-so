#ifndef ALOCMEM_H
#define ALOCMEM_H

#include <stdbool.h>

typedef struct alocmem_t alocmem_t;

alocmem_t *alocmem_cria(int quadro_ini, int quadro_fim);
void alocmem_destroi(alocmem_t *self);

bool alocmem_aloca_quadro(alocmem_t *self, int *pquadro);
void alocmem_libera_quadro(alocmem_t *self, int quadro);

#endif

