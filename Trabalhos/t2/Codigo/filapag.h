#ifndef FILAPAG_H
#define FILAPAG_H

#include "proc.h"

typedef struct filapag_t filapag_t;

typedef enum {
    FILAPAG_MODO_FIFO,
    FILAPAG_MODO_SEGUNDA_CHANCE,
    N_FILAPAG_MODO
} filapag_modo_t;

filapag_t *filapag_cria(filapag_modo_t modo, int quadro_ini, int quadro_fim);
void filapag_destroi(filapag_t *self);

void filapag_enfilera_pagina(filapag_t *self, proc_t *proc, int pagina);
void filapag_desenfilera_pagina(filapag_t *self, proc_t *proc, int pagina);

bool filapag_escolhe_vitima(filapag_t *self, proc_t **pproc , int *ppagina);

#endif