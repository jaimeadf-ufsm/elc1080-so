#include "ctlmem.h"

#include <stdlib.h>

typedef struct ctlmem_no_t ctlmem_no_t;

struct ctlmem_no_t {
    proc_t *proc;
    int pagina;

    ctlmem_no_t *ant;
    ctlmem_no_t *prox;
};

struct ctlmem_t {
    int modo;

    int quadro_ini;
    int quadro_fim;

    ctlmem_no_t *mapa;
    ctlmem_no_t *pilha;
};

static bool ctlmem_extrair_candidato_fifo(ctlmem_t *self, proc_t **pproc, int *ppagina);
static bool ctlmem_extrair_candidato_second_chance(ctlmem_t *self, proc_t **pproc, int *ppagina);

ctlmem_t *ctlmem_cria(ctlmem_modo_t modo, int quadro_ini, int quadro_fim)
{
    ctlmem_t *self = (ctlmem_t *)malloc(sizeof(ctlmem_t));
    assert(self != NULL);

    self->modo = modo;

    self->quadro_ini = quadro_ini;
    self->quadro_fim = quadro_fim;

    self->mapa = (ctlmem_no_t *)calloc(quadro_fim - quadro_ini + 1, sizeof(ctlmem_no_t));
    assert(self->mapa != NULL);

    self->pilha = NULL;

    int qtd_quadros = quadro_fim - quadro_ini + 1;

    for (int i = 0; i < qtd_quadros; i++) {
        ctlmem_no_t *no = &self->mapa[i];

        no->proc = NULL;
        no->pagina = -1;
        no->ant = NULL;
        no->prox = NULL;
    }

    return self;
}

void ctlmem_destroi(ctlmem_t *self)
{
    free(self->mapa);
    free(self);
}

void ctlmem_adicionar_candidato(ctlmem_t *self, proc_t *proc, int pagina)
{
  int quadro;
  assert(tabpag_traduz(proc_tabpag(proc), pagina, &quadro) == ERR_OK);
  assert(quadro >= self->quadro_ini && quadro <= self->quadro_fim);

  ctlmem_no_t *no = &self->mapa[quadro - self->quadro_ini];
  assert(no->proc == NULL && no->pagina == -1);

  no->proc = proc;
  no->pagina = pagina;

  if (self->pilha == NULL) {
    self->pilha = no;
    no->ant = no;
    no->prox = no;
  } else {
    ctlmem_no_t *topo = self->pilha;
    ctlmem_no_t *fundo = topo->ant;

    no->ant = fundo;
    no->prox = topo;

    fundo->prox = no;
    topo->ant = no;

    self->pilha = no;
  }
}

void ctlmem_remover_candidato(ctlmem_t *self, proc_t *proc, int pagina)
{
  int quadro;
  assert(tabpag_traduz(proc_tabpag(proc), pagina, &quadro) == ERR_OK);
  assert(quadro >= self->quadro_ini && quadro <= self->quadro_fim);

  ctlmem_no_t *no = &self->mapa[quadro - self->quadro_ini];
  assert(no->proc == proc && no->pagina == pagina);

  if (no->prox == no) {
    self->pilha = NULL;
  } else {
    no->ant->prox = no->prox;
    no->prox->ant = no->ant;

    if (self->pilha == no) {
      self->pilha = no->prox;
    }
  }

  no->proc = NULL;
  no->pagina = -1;
  no->ant = NULL;
  no->prox = NULL;
}

bool ctlmem_extrair_candidato(ctlmem_t *self, proc_t **pproc, int *ppagina)
{
  switch (self->modo) {
    case CTLMEM_MODO_FIFO:
      ctlmem_extrair_candidato_fifo(self, pproc, ppagina);
      break;
    case CTLMEM_MODO_SECOND_CHANCE:
      ctlmem_extrair_candidato_second_chance(self, pproc, ppagina);
      break;
  }
}

static bool ctlmem_extrair_candidato_fifo(ctlmem_t *self, proc_t **pproc, int *ppagina)
{
  if (self->pilha == NULL) {
    return false;
  }

  ctlmem_no_t *no = self->pilha;

  *pproc = no->proc;
  *ppagina = no->pagina;

  ctlmem_remover_candidato(self, no->proc, no->pagina);
}

static bool ctlmem_extrair_candidato_second_chance(ctlmem_t *self, proc_t **pproc, int *ppagina)
{
  if (self->pilha == NULL) {
    return false;
  }

  ctlmem_no_t *no = self->pilha;

  while (tabpag_bit_acesso(proc_tabpag(no->proc), no->pagina)) {
    tabpag_zera_bit_acesso(proc_tabpag(no->proc), no->pagina);
    no = no->prox;
  }

  *pproc = no->proc;
  *ppagina = no->pagina;

  return true;
}
