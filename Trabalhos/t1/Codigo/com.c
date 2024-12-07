#include "com.h"

#include <stdlib.h>
#include <assert.h>

typedef struct porta_t porta_t;

#define TAM_PORTAS_INICIAL 4

struct porta_t
{
  dispositivo_id_t leitura;
  dispositivo_id_t leitura_ok;
  dispositivo_id_t escrita;
  dispositivo_id_t escrita_ok;

  bool reservada;
};

struct com_t
{
  es_t *es;

  int tam;
  int qtd;
  int ini;

  porta_t *portas;
};

com_t *com_cria(es_t *es)
{
  com_t *self = (com_t *)malloc(sizeof(com_t));
  assert(self != NULL);

  self->es = es;

  self->tam = TAM_PORTAS_INICIAL;
  self->qtd = 0;
  self->ini = 0;

  self->portas = (porta_t *)malloc(sizeof(porta_t) * self->tam);
  assert(self->portas != NULL);

  return self;
}

void com_destroi(com_t *self)
{
  free(self->portas);
  free(self);
}

int com_registra_porta(com_t *self, dispositivo_id_t leitura, dispositivo_id_t leitura_ok, dispositivo_id_t escrita, dispositivo_id_t escrita_ok)
{
  if (self->qtd == self->tam) {
    self->tam *= 2;
    self->portas = (porta_t *)realloc(self->portas, sizeof(porta_t) * self->tam);
    assert(self->portas != NULL);
  }

  self->portas[self->qtd].leitura = leitura;
  self->portas[self->qtd].leitura_ok = leitura_ok;
  self->portas[self->qtd].escrita = escrita;
  self->portas[self->qtd].escrita_ok = escrita_ok;

  self->portas[self->qtd].reservada = false;

  return self->qtd++;
}

bool com_le_porta(com_t *self, int id, int *pvalor)
{
  int ok;

  assert(id < self->qtd);

  if (es_le(self->es, self->portas[id].leitura_ok, &ok) != ERR_OK) {
    return false;
  }

  if (!ok) {
    return false;
  }

  return es_le(self->es, self->portas[id].leitura, pvalor) == ERR_OK;
}

bool com_escreve_porta(com_t *self, int id, int valor)
{
  int ok;

  assert(id < self->qtd);

  if (es_le(self->es, self->portas[id].escrita_ok, &ok) != ERR_OK) {
    return false;
  }

  if (!ok) {
    return false;
  }

  return es_escreve(self->es, self->portas[id].escrita, valor) == ERR_OK;
}

void com_reserva_porta(com_t *self, int id)
{
  assert(id < self->qtd);
  self->portas[id].reservada = true;
}

void com_libera_porta(com_t *self, int id)
{
  assert(id < self->qtd);
  self->portas[id].reservada = false;
}

int com_porta_disponivel(com_t *self)
{
  for (int j = 0; j < self->qtd; j++) {
    int i = (self->ini + j) % self->qtd;

    if (!self->portas[i].reservada) {
      self->ini = (i + 1) % self->qtd;
      return i;
    }
  }

  return -1;
}