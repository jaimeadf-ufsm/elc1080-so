#include "esc.h"

#include <stdlib.h>
#include <assert.h>

typedef struct esc_no_t esc_no_t;

struct esc_no_t {
  proc_t *proc;

  esc_no_t *ant;
  esc_no_t *prox;
};

struct esc_t
{
  esc_modo_t modo;

  esc_no_t *inicio;
  esc_no_t *fim;
};

static proc_t *esc_proximo_modo_simples(esc_t *self);
static proc_t *esc_proximo_modo_circular(esc_t *self);
static proc_t *esc_proximo_modo_prioritario(esc_t *self);

esc_t *esc_cria(esc_modo_t modo)
{
  esc_t *self = (esc_t *)malloc(sizeof(esc_t));
  assert(self != NULL);

  self->modo = modo;
  self->inicio = NULL;
  self->fim = NULL;

  return self;
}

void esc_destroi(esc_t *self)
{
  while (self->inicio != NULL) {
    esc_no_t *prox = self->inicio->prox;
    free(self->inicio);
    self->inicio = prox;
  }

  free(self);
}

void esc_insere_proc(esc_t *self, proc_t *proc)
{
  esc_no_t *no = (esc_no_t *)malloc(sizeof(esc_no_t));
  assert(no != NULL);

  no->proc = proc;
  no->ant = self->fim;
  no->prox = NULL;

  if (self->fim == NULL) {
    self->inicio = no;
  } else {
    self->fim->prox = no;
  }

  self->fim = no;
}

void esc_remove_proc(esc_t *self, proc_t *proc)
{
  for (esc_no_t *no = self->inicio; no != NULL; no = no->prox) {
    if (no->proc != proc) {
      continue;
    }

    if (no->ant != NULL) {
      no->ant->prox = no->prox;
    } else {
      self->inicio = no->prox;
    }

    if (no->prox != NULL) {
      no->prox->ant = no->ant;
    } else {
      self->fim = no->ant;
    }

    free(no);

    break;
  }
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
  if (self->inicio == NULL) {
    return NULL;
  }

  for (esc_no_t *no = self->inicio; no != NULL; no = no->prox) {
    if (proc_estado(no->proc) == PROC_ESTADO_EXECUTANDO) {
      return no->proc;
    }
  }

  return self->inicio->proc;
}

static proc_t *esc_proximo_modo_circular(esc_t *self)
{
  if (self->inicio == NULL) {
    return NULL;
  }

  return self->inicio->proc;
}

static proc_t *esc_proximo_modo_prioritario(esc_t *self)
{
  if (self->inicio == NULL) {
    return NULL;
  }

  proc_t *candidato = self->inicio->proc;

  for (esc_no_t *no = self->inicio->prox; no != NULL; no = no->prox) {
    if (proc_prioridade(no->proc) < proc_prioridade(candidato)) {
      candidato = no->proc;
    }
  }

  return candidato;
}