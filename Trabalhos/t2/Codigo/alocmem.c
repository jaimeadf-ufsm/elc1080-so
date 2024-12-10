#include "alocmem.h"

#include <stdlib.h>
#include <assert.h>

struct alocmem_t {
  int quadro_ini;
  int quadro_fim;

  bool *quadros;
};

alocmem_t *alocmem_cria(int quadro_ini, int quadro_fim)
{
  alocmem_t *self = (alocmem_t *)malloc(sizeof(alocmem_t));
  assert(self != NULL);

  int qtd_quadros = quadro_fim - quadro_ini + 1;

  self->quadro_ini = quadro_ini;
  self->quadro_fim = quadro_fim;

  self->quadros = (bool *)calloc(qtd_quadros, sizeof(bool));

  return self;
}

void alocmem_destroi(alocmem_t *self)
{
  free(self->quadros);
  free(self);
}

bool alocmem_aloca_quadro(alocmem_t *self, int *pquadro)
{
  int qtd_quadros = self->quadro_fim - self->quadro_ini + 1;

  for (int i = 0; i < qtd_quadros; i++) {
    if (!self->quadros[i]) {
      self->quadros[i] = true;
      *pquadro = self->quadro_ini + i;

      return true;
    }
  }

  return false;
}

void alocmem_libera_quadro(alocmem_t *self, int quadro)
{
  self->quadros[quadro - self->quadro_ini] = false;
}