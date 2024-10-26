#ifndef ESC_H
#define ESC_H

#include <stdbool.h>

typedef struct esc_t esc_t;
typedef struct proc_t proc_t;

typedef enum {
    ESC_MODO_SIMPLES,
    ESC_MODO_CIRCULAR,
    ESC_MODO_PRIORITARIO,
    ESC_MODO_N
} esc_modo_t;

typedef enum {
    PROC_ESTADO_EXECUTANDO,
    PROC_ESTADO_PRONTO,
    PROC_ESTADO_BLOQUEADO,
    PROC_ESTADO_MORTO,
    PROC_ESTADO_N
} proc_estado_t;

typedef enum {
    PROC_BLOQ_LEITURA,
    PROC_BLOQ_ESCRITA,
    PROC_BLOQ_ESPERA_PROC,
    PROC_BLOQ_N
} proc_bloq_motivo_t;

esc_t *esc_cria();
void esc_destroi(esc_t *self);

int esc_proc_qtd(esc_t *self);

proc_t **esc_proc_tabela(esc_t *self);
proc_t *esc_proc_corrente(esc_t *self);

proc_t *esc_busca_proc(esc_t *self, int id);

proc_t *esc_inicia_proc(esc_t *self, int end);
bool esc_executa_proc(esc_t *self, int id);
bool esc_bloqueia_proc(esc_t *self, int id, proc_bloq_motivo_t motivo, int arg);
bool esc_desbloqueia_proc(esc_t *self, int id);
bool esc_encerra_proc(esc_t *self, int id);

void esc_escalona_proc(esc_t *self);

void esc_tictac(esc_t *self);

int proc_id(proc_t *self);
proc_estado_t proc_estado(proc_t *self);

int proc_PC(proc_t *self);
int proc_A(proc_t *self);
int proc_X(proc_t *self);

void proc_define_PC(proc_t *self, int valor);
void proc_define_A(proc_t *self, int valor);
void proc_define_X(proc_t *self, int valor);

proc_bloq_motivo_t proc_bloq_motivo(proc_t *self);
int proc_bloq_arg(proc_t *self);

int proc_porta(proc_t *self);

void proc_atribui_porta(proc_t *self, int porta);
void proc_desatribui_porta(proc_t *self);

#endif