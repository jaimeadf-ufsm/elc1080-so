#include "esc.h"

#include <stdlib.h>
#include <assert.h>

typedef struct esc_no_t esc_no_t;

struct esc_no_t {
  proc_t *proc;

  esc_no_t *prox;
  esc_no_t *ant;
};

struct esc_t
{
  esc_modo_t modo;
  esc_no_t *fila;
};

static proc_t *esc_proximo_modo_simples(esc_t *self);
static proc_t *esc_proximo_modo_circular(esc_t *self);
static proc_t *esc_proximo_modo_prioritario(esc_t *self);

esc_t *esc_cria(esc_modo_t modo)
{
  esc_t *self = (esc_t *)malloc(sizeof(esc_t));
  assert(self != NULL);

  self->modo = modo;
  self->fila = NULL;

  return self;
}

void esc_destroi(esc_t *self)
{
  if (self->fila != NULL) {
    esc_no_t *no = self->fila;

    do
    {
      esc_no_t *prox = no->prox;
      free(no);
      no = prox;
    } while (no != self->fila);
  }

  free(self);
}

void esc_insere_proc(esc_t *self, proc_t *proc)
{
  esc_no_t *no = (esc_no_t *)malloc(sizeof(esc_no_t));
  assert(no != NULL);

  no->proc = proc;

  if (self->fila == NULL) {
    no->prox = no;
    no->ant = no;
    self->fila = no;
  } else {
    no->prox = self->fila;
    no->ant = self->fila->ant;

    self->fila->ant->prox = no;
    self->fila->ant = no;
  }
}

void esc_remove_proc(esc_t *self, proc_t *proc)
{
  if (self->fila == NULL) {
    return;
  }

  esc_no_t *no = self->fila;

  do
  {
    if (no->proc == proc)
    {
      if (no->prox == no)
      {
        self->fila = NULL;
      }
      else
      {
        no->prox->ant = no->ant;
        no->ant->prox = no->prox;

        if (self->fila == no)
        {
          self->fila = no->prox;
        }
      }

      free(no);
      return;
    }

    no = no->prox;
  } while (no != self->fila);
}

proc_t *esc_proximo(esc_t *self)
{
  switch (self->modo)
  {
  case ESC_MODO_SIMPLES:
    return esc_proximo_modo_simples(self);
  case ESC_MODO_CIRCULAR:
    return esc_proximo_modo_circular(self);
  case ESC_MODO_PRIORITARIO:
    return esc_proximo_modo_prioritario(self);
  default:
    return NULL;
  }
}

static proc_t *esc_proximo_modo_simples(esc_t *self)
{
  if (self->fila == NULL) {
    return NULL;
  }

  return self->fila->proc;
}

static proc_t *esc_proximo_modo_circular(esc_t *self)
{
  if (self->fila == NULL) {
    return NULL;
  }

  esc_no_t *no = self->fila;

  self->fila = no->prox;

  return no->proc;
}

static proc_t *esc_proximo_modo_prioritario(esc_t *self)
{
  if (self->fila == NULL) {
    return NULL;
  }

  proc_t *melhor = self->fila->proc;

  for (esc_no_t *no = self->fila->prox; no != self->fila; no = no->prox) {
    if (proc_prioridade(no->proc) < proc_prioridade(melhor)) {
      melhor = no->proc;
    }
  }

  return melhor;
}