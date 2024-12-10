#ifndef CTLMEM_H
#define CTLMEM_H

#include "proc.h"

typedef struct ctlmem_t ctlmem_t;

typedef enum {
    CTLMEM_MODO_FIFO,
    CTLMEM_MODO_SECOND_CHANCE,
    N_CTLMEM_MODO
} ctlmem_modo_t;

ctlmem_t *ctlmem_cria(ctlmem_modo_t modo, int quadro_ini, int quadro_fim);
void ctlmem_destroi(ctlmem_t *self);

void ctlmem_adicionar_candidato(ctlmem_t *self, proc_t *proc, int pagina);
void ctlmem_remover_candidato(ctlmem_t *self, proc_t *proc, int pagina);

bool ctlmem_extrair_candidato(ctlmem_t *self, proc_t **pproc , int *ppagina);

#endif