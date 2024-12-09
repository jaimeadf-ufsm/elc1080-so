#include "tabela.h"
#include "console.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

struct tabela_t {
  int n_linhas;
  int n_colunas;
  int tam_celula;

  char *celulas;
  int *larguras;
};

static int tabela_tamanho(tabela_t *self);
static char *tabela_celula(tabela_t *self, int i, int j);

tabela_t *tabela_cria(int n_linhas, int n_colunas, int tam_celula)
{
  tabela_t *self = malloc(sizeof(*self));
  assert(self != NULL);

  self->n_linhas = n_linhas;
  self->n_colunas = n_colunas;
  self->tam_celula = tam_celula;

  self->celulas = calloc(n_linhas * n_colunas, tam_celula * sizeof(char));
  assert(self->celulas != NULL);

  self->larguras = calloc(n_colunas, sizeof(int));
  assert(self->larguras != NULL);

  return self;
}

void tabela_destroi(tabela_t *self)
{
  free(self->celulas);
  free(self->larguras);
  free(self);
}

int tabela_linhas(tabela_t *self)
{
  return self->n_linhas;
}

int tabela_colunas(tabela_t *self)
{
  return self->n_colunas;
}

void tabela_preenche(tabela_t *self, int i, int j, const char *formato, ...)
{
  char *celula = tabela_celula(self, i, j);

  va_list arg;
  va_start(arg, formato);
  vsnprintf(celula, self->tam_celula, formato, arg);
  va_end(arg);

  int largura = strlen(celula);

  if (largura > self->larguras[j]) {
    self->larguras[j] = largura;
  }
}

void tabela_mostra(tabela_t *self)
{
  char *buffer = (char *)malloc(tabela_tamanho(self) * sizeof(char));
  char *ptr = buffer;

  for (int j = 0; j < self->n_colunas; j++) {
    ptr += sprintf(ptr, "| %-*s ", self->larguras[j], tabela_celula(self, 0, j));
  }

  ptr += sprintf(ptr, "| \n");

  for (int j = 0; j < self->n_colunas; j++) {
    ptr += sprintf(ptr, "| ");

    for (int k = 0; k < self->larguras[j]; k++) {
      *ptr++ = '-';
    }

    *ptr++ = ' ';
  }

  ptr += sprintf(ptr, "|\n");

  for (int i = 1; i < self->n_linhas; i++) {
    for (int j = 0; j < self->n_colunas; j++) {
      char *celula = tabela_celula(self, i, j);
      int largura = self->larguras[j];

      ptr += sprintf(ptr, "| %-*s ", largura, celula);
    }

    ptr += sprintf(ptr, "|\n");
  }

  console_printf("%s\n", buffer);

  free(buffer);
}

static int tabela_tamanho(tabela_t *self)
{
  int tamanho = 0;

  for (int j = 0; j < self->n_colunas; j++) {
    tamanho += self->larguras[j] + 2;
  }

  tamanho += self->n_colunas + 2;
  tamanho *= self->n_linhas + 1;

  return tamanho + 1;
}

static char *tabela_celula(tabela_t *self, int i, int j)
{
  return self->celulas + (i * self->n_colunas + j) * self->tam_celula;
}