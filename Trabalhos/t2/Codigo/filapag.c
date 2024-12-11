#include "filapag.h"

#include <stdlib.h>
#include <assert.h>

typedef struct filapag_no_t filapag_no_t;

struct filapag_no_t {
    proc_t *proc;
    int pagina;

    filapag_no_t *ant;
    filapag_no_t *prox;
};

struct filapag_t {
    int modo;

    int quadro_ini;
    int quadro_fim;

    filapag_no_t *mapa;
    filapag_no_t *fila;
};

static bool filapag_escolhe_vitima_fifo(filapag_t *self, proc_t **pproc, int *ppagina);
static bool filapag_escolhe_vitima_segunda_chance(filapag_t *self, proc_t **pproc, int *ppagina);

filapag_t *filapag_cria(filapag_modo_t modo, int quadro_ini, int quadro_fim)
{
    filapag_t *self = (filapag_t *)malloc(sizeof(filapag_t));
    assert(self != NULL);

    self->modo = modo;

    self->quadro_ini = quadro_ini;
    self->quadro_fim = quadro_fim;

    self->mapa = (filapag_no_t *)calloc(quadro_fim - quadro_ini + 1, sizeof(filapag_no_t));
    assert(self->mapa != NULL);

    self->fila = NULL;

    int qtd_quadros = quadro_fim - quadro_ini + 1;

    for (int i = 0; i < qtd_quadros; i++) {
        filapag_no_t *no = &self->mapa[i];

        no->proc = NULL;
        no->pagina = -1;
        no->ant = NULL;
        no->prox = NULL;
    }

    return self;
}

void filapag_destroi(filapag_t *self)
{
    free(self->mapa);
    free(self);
}

void filapag_enfilera_pagina(filapag_t *self, proc_t *proc, int pagina)
{
  int quadro;
  assert(tabpag_traduz(proc_tabpag(proc), pagina, &quadro) == ERR_OK);
  assert(quadro >= self->quadro_ini && quadro <= self->quadro_fim);

  filapag_no_t *no = &self->mapa[quadro - self->quadro_ini];
  assert(no->proc == NULL && no->pagina == -1);

  no->proc = proc;
  no->pagina = pagina;

  if (self->fila == NULL) {
    self->fila = no;
    no->ant = no;
    no->prox = no;
  } else {
    no->ant = self->fila->ant;
    no->prox = self->fila;

    self->fila->ant->prox = no;
    self->fila->ant = no;
  }
}

void filapag_desenfilera_pagina(filapag_t *self, proc_t *proc, int pagina)
{
  int quadro;
  assert(tabpag_traduz(proc_tabpag(proc), pagina, &quadro) == ERR_OK);
  assert(quadro >= self->quadro_ini && quadro <= self->quadro_fim);

  filapag_no_t *no = &self->mapa[quadro - self->quadro_ini];
  assert(no->proc == proc && no->pagina == pagina);

  if (no->ant == no) {
    self->fila = NULL;
  } else {
    no->ant->prox = no->prox;
    no->prox->ant = no->ant;

    if (self->fila == no) {
      self->fila = no->prox;
    }
  }

  no->proc = NULL;
  no->pagina = -1;
  no->ant = NULL;
  no->prox = NULL;
}

bool filapag_escolhe_vitima(filapag_t *self, proc_t **pproc, int *ppagina)
{
  switch (self->modo) {
    case FILAPAG_MODO_FIFO:
      return filapag_escolhe_vitima_fifo(self, pproc, ppagina);
      break;
    case FILAPAG_MODO_SEGUNDA_CHANCE:
      return filapag_escolhe_vitima_segunda_chance(self, pproc, ppagina);
      break;
    default:
      return false;
  }
}

static bool filapag_escolhe_vitima_fifo(filapag_t *self, proc_t **pproc, int *ppagina)
{
  if (self->fila == NULL) {
    return false;
  }

  filapag_no_t *no = self->fila;

  *pproc = no->proc;
  *ppagina = no->pagina;

  return true;
}

static bool filapag_escolhe_vitima_segunda_chance(filapag_t *self, proc_t **pproc, int *ppagina)
{
  if (self->fila == NULL) {
    return false;
  }

  while (tabpag_bit_acesso(proc_tabpag(self->fila->proc), self->fila->pagina)) {
    tabpag_zera_bit_acesso(proc_tabpag(self->fila->proc), self->fila->pagina);
    self->fila = self->fila->prox;
  }

  *pproc = self->fila->proc;
  *ppagina = self->fila->pagina;

  return true;
}
