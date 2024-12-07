#ifndef ESC_H
#define ESC_H

#include <stdbool.h>
#include "proc.h"

typedef enum {
  ESC_MODO_SIMPLES,
  ESC_MODO_CIRCULAR,
  ESC_MODO_PRIORITARIO,
  N_ESC_MODO
} esc_modo_t;

typedef struct esc_t esc_t;

esc_t *esc_cria(esc_modo_t modo);
void esc_destroi(esc_t *self);

void esc_insere_proc(esc_t *self, proc_t *proc);
void esc_remove_proc(esc_t *self, proc_t *proc);

proc_t *esc_proximo(esc_t *self);

#endif