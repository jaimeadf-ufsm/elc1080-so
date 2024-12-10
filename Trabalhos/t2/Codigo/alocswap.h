#ifndef ALOCSWAP_H
#define ALOCSWAP_H

#include <stdbool.h>

typedef struct alocswap_t alocswap_t;
typedef struct regswap_t regswap_t;

alocswap_t *alocswap_cria(int quadro_ini, int quadro_fim);
void alocswap_destroi(alocswap_t *self);

regswap_t *alocswap_aloca_regiao(alocswap_t *self, int qtd_quadros);
void alocswap_libera_regiao(alocswap_t *self, regswap_t *reg);

bool regswap_traduz(regswap_t *self, int pagina, int *pquadro);

#endif
