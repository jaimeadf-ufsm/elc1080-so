#ifndef PROC_H
#define PROC_H

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

typedef struct proc_t proc_t;
typedef struct proc_estado_metricas_t proc_estado_metricas_t;
typedef struct proc_metricas_t proc_metricas_t;

struct proc_estado_metricas_t
{
    int n_vezes;
    int t_total;
};

struct proc_metricas_t
{
    int n_preempcoes;
    
    int t_retorno;
    int t_resposta;

    proc_estado_metricas_t estados[PROC_ESTADO_N];
};

proc_t *proc_cria(int id, int end);
void proc_destroi(proc_t *self);

int proc_id(proc_t *self);
proc_estado_t proc_estado(proc_t *self);

void proc_executa(proc_t *self);
void proc_para(proc_t *self);
void proc_bloqueia(proc_t *self, proc_bloq_motivo_t motivo, int arg);
void proc_desbloqueia(proc_t *self);
void proc_encerra(proc_t *self);

void proc_atualiza_metricas(proc_t *self, int delta);

double proc_prioridade(proc_t *self);
void proc_define_prioridade(proc_t *self, double prioridade);

proc_bloq_motivo_t proc_bloq_motivo(proc_t *self);
int proc_bloq_arg(proc_t *self);

int proc_porta(proc_t *self);
void proc_atribui_porta(proc_t *self, int porta);
void proc_desatribui_porta(proc_t *self);

int proc_PC(proc_t *self);
int proc_A(proc_t *self);
int proc_X(proc_t *self);

void proc_define_PC(proc_t *self, int valor);
void proc_define_A(proc_t *self, int valor);
void proc_define_X(proc_t *self, int valor);

proc_metricas_t proc_metricas(proc_t *self);

char *proc_estado_nome(proc_estado_t estado);

#endif