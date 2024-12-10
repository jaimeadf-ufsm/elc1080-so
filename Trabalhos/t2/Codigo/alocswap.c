#include "alocswap.h"

#include <assert.h>
#include <stdlib.h>

#define TAM_REGS_INICIAL 4

struct regswap_t {
  int quadro_inicial;
  int qtd_quadros;
};

struct alocswap_t {
  int quadro_ini;

  int qtd_quadros_total;
  int qtd_quadros_ocupados;
};

alocswap_t *alocswap_cria(int quadro_ini, int quadro_fim)
{
  alocswap_t *self = (alocswap_t *)malloc(sizeof(alocswap_t));
  assert(self != NULL);

  self->quadro_ini = quadro_ini;
  self->qtd_quadros_total = quadro_fim - quadro_ini + 1;
  self->qtd_quadros_ocupados = 0;

  return self;
}

void alocswap_destroi(alocswap_t *self)
{
  free(self);
}

regswap_t *alocswap_aloca_regiao(alocswap_t *self, int qtd_quadros)
{
  if (self->qtd_quadros_ocupados + qtd_quadros > self->qtd_quadros_total) {
    return NULL;
  }

  regswap_t *reg = (regswap_t *)malloc(sizeof(regswap_t));
  assert(reg != NULL);

  reg->quadro_inicial = self->quadro_ini + self->qtd_quadros_ocupados;
  reg->qtd_quadros = qtd_quadros;

  self->qtd_quadros_ocupados += qtd_quadros;

  return reg;
}

void alocswap_libera_regiao(alocswap_t *self, regswap_t *reg)
{
  self->qtd_quadros_ocupados -= reg->qtd_quadros;
  free(reg);
}

bool regswap_traduz(regswap_t *self, int pagina, int *pquadro)
{
  if (pagina < 0 || pagina >= self->qtd_quadros) {
    return false;
  }

  *pquadro = self->quadro_inicial + pagina;

  return true;
}
