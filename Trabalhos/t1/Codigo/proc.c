#include "proc.h"

#include <stdlib.h>
#include <assert.h>

struct proc_t
{
  int id;
  double prioridade;
  proc_estado_t estado;

  int bloq_motivo;
  int bloq_arg;

  int porta;

  int reg_PC;
  int reg_A;
  int reg_X;

  proc_metricas_t metricas;
};

static void proc_muda_estado(proc_t *self, proc_estado_t estado);

proc_t *proc_cria(int id, int end)
{
  proc_t *self = malloc(sizeof(proc_t));
  assert(self != NULL);

  self->id = id;
  self->prioridade = 0.5;
  self->estado = PROC_ESTADO_PRONTO;

  self->bloq_motivo = 0;
  self->bloq_arg = 0;

  self->porta = -1;

  self->reg_PC = end;
  self->reg_A = 0;
  self->reg_X = 0;

  self->metricas.t_retorno = 0;
  self->metricas.n_preempcoes = 0;

  for (int i = 0; i < PROC_ESTADO_N; i++) {
    self->metricas.estados[i].n_vezes = 0;
    self->metricas.estados[i].t_total = 0;
  }

  self->metricas.estados[PROC_ESTADO_PRONTO].n_vezes = 1;

  return self;
}

void proc_destroi(proc_t *self)
{
  free(self);
}

int proc_id(proc_t *self)
{
  return self->id;
}

proc_estado_t proc_estado(proc_t *self)
{
  return self->estado;
}

void proc_executa(proc_t *self)
{
  assert(self->estado == PROC_ESTADO_PRONTO);
  proc_muda_estado(self, PROC_ESTADO_EXECUTANDO);
}

void proc_para(proc_t *self)
{
  assert(self->estado == PROC_ESTADO_EXECUTANDO);
  proc_muda_estado(self, PROC_ESTADO_PRONTO);
}

void proc_bloqueia(proc_t *self, proc_bloq_motivo_t motivo, int arg)
{
  assert(
    (self->estado == PROC_ESTADO_EXECUTANDO) ||
    (self->estado == PROC_ESTADO_PRONTO)
  );

  proc_muda_estado(self, PROC_ESTADO_BLOQUEADO);

  self->bloq_motivo = motivo;
  self->bloq_arg = arg;
}

void proc_desbloqueia(proc_t *self)
{
  assert(self->estado == PROC_ESTADO_BLOQUEADO);
  proc_muda_estado(self, PROC_ESTADO_PRONTO);
}

void proc_encerra(proc_t *self)
{
  proc_muda_estado(self, PROC_ESTADO_MORTO);
}

void proc_atualiza_metricas(proc_t *self, int delta)
{
  if (self->estado != PROC_ESTADO_MORTO) {
    self->metricas.t_retorno += delta;
  }

  self->metricas.estados[self->estado].t_total += delta;
  self->metricas.t_resposta = self->metricas.estados[PROC_ESTADO_PRONTO].t_total;
  self->metricas.t_resposta /= self->metricas.estados[PROC_ESTADO_PRONTO].n_vezes;
}

double proc_prioridade(proc_t *self)
{
  return self->prioridade;
}

void proc_define_prioridade(proc_t *self, double prioridade)
{
  self->prioridade = prioridade;
}

int proc_PC(proc_t *self)
{
  return self->reg_PC;
}

int proc_A(proc_t *self)
{
  return self->reg_A;
}

int proc_X(proc_t *self)
{
  return self->reg_X;
}

void proc_define_PC(proc_t *self, int valor)
{
  self->reg_PC = valor;
}

void proc_define_A(proc_t *self, int valor)
{
  self->reg_A = valor;
}

void proc_define_X(proc_t *self, int valor)
{
  self->reg_X = valor;
}

proc_bloq_motivo_t proc_bloq_motivo(proc_t *self)
{
  return self->bloq_motivo;
}

int proc_bloq_arg(proc_t *self)
{
  return self->bloq_arg;
}

int proc_porta(proc_t *self)
{
  return self->porta;
}

void proc_atribui_porta(proc_t *self, int porta)
{
  assert(porta != -1);
  self->porta = porta;
}

void proc_desatribui_porta(proc_t *self)
{
  self->porta = -1;
}

proc_metricas_t proc_metricas(proc_t *self)
{
  return self->metricas;
}

char *proc_estado_nome(proc_estado_t estado)
{
  switch (estado) {
    case PROC_ESTADO_PRONTO:
      return "Pronto";
    case PROC_ESTADO_EXECUTANDO:
      return "Executando";
    case PROC_ESTADO_BLOQUEADO:
      return "Bloqueado";
    case PROC_ESTADO_MORTO:
      return "Morto";
    default:
      return "Desconhecido";
  }
}

static void proc_muda_estado(proc_t *self, proc_estado_t estado)
{
  if (self->estado == PROC_ESTADO_EXECUTANDO && estado == PROC_ESTADO_PRONTO) {
    self->metricas.n_preempcoes++;
  }

  self->metricas.estados[estado].n_vezes++;
  self->estado = estado;
}
