#include "esc.h"

#include <stdlib.h>
#include <assert.h>

#include "console.h"

#define TAM_TABELA_INICIAL 4

struct esc_t
{
  int tam;
  int qtd;

  proc_t **proc_tabela;
  proc_t *proc_executando;
};

struct proc_t
{
  int id;
  proc_estado_t estado;

  int reg_PC;
  int reg_A;
  int reg_X;

  int bloq_motivo;
  int bloq_arg;

  int porta;

  proc_t *proximo;
  proc_t *anterior;
};

proc_t *proc_cria(int id, int end);
void proc_destroi(proc_t *self);

void proc_executa(proc_t *self);
void proc_para(proc_t *self);
void proc_bloqueia(proc_t *self, proc_bloq_motivo_t motivo, int arg);
void proc_desbloqueia(proc_t *self);
void proc_encerra(proc_t *self);

esc_t *esc_cria()
{
  esc_t *self = malloc(sizeof(esc_t));
  assert(self != NULL);

  self->tam = TAM_TABELA_INICIAL;
  self->qtd = 0;

  self->proc_tabela = malloc(self->tam * sizeof(proc_t *));
  assert(self->proc_tabela != NULL);

  self->proc_executando = NULL;

  return self;
}

void esc_destroi(esc_t *self)
{
  for (int i = 0; i < self->qtd; i++)
  {
    proc_destroi(self->proc_tabela[i]);
  }

  free(self->proc_tabela);
  free(self->proc_executando);
}

int esc_proc_qtd(esc_t *self)
{
  return self->qtd;
}

proc_t **esc_proc_tabela(esc_t *self)
{
  return self->proc_tabela;
}

proc_t *esc_proc_executando(esc_t *self)
{
  return self->proc_executando;
}

proc_t *esc_busca_proc(esc_t *self, int id)
{
  if (id <= 0 || id > self->qtd)
    return NULL;

  return self->proc_tabela[id - 1];
}

proc_t *esc_inicia_proc(esc_t *self, int end)
{
  if (self->qtd == self->tam)
  {
    self->tam *= 2;
    self->proc_tabela = realloc(self->proc_tabela, self->tam * sizeof(proc_t));
    assert(self->proc_tabela != NULL);
  }

  proc_t *proc = proc_cria(self->qtd + 1, end);
  self->proc_tabela[self->qtd++] = proc;

  return proc;
}

bool esc_executa_proc(esc_t *self, int id)
{
  proc_t *proc = esc_busca_proc(self, id);

  if (proc == NULL)
    return false;
  
  if (proc_estado(proc) != PROC_ESTADO_EXECUTANDO)
  {
    assert(proc_estado(proc) == PROC_ESTADO_PRONTO);

    if (self->proc_executando != NULL)
      proc_para(self->proc_executando);

    proc_executa(proc);
    self->proc_executando = proc;
  }

  return true;
}

bool esc_bloqueia_proc(esc_t *self, int id, proc_bloq_motivo_t motivo, int arg)
{
  proc_t *proc = esc_busca_proc(self, id);

  if (proc == NULL)
    return false;

  if (proc_estado(proc) == PROC_ESTADO_EXECUTANDO)
    self->proc_executando = NULL;

  proc_bloqueia(proc, motivo, arg);

  return true;
}

bool esc_desbloqueia_proc(esc_t *self, int id)
{
  proc_t *proc = esc_busca_proc(self, id);

  if (proc == NULL)
    return false;

  proc_desbloqueia(proc);

  return true;
}

bool esc_encerra_proc(esc_t *self, int id)
{
  proc_t *proc = esc_busca_proc(self, id);

  if (proc == NULL)
    return false;

  if (proc_estado(proc) == PROC_ESTADO_EXECUTANDO)
    self->proc_executando = NULL;

  proc_encerra(proc);

  return true;
}

void esc_proximo_proc(esc_t *self)
{
  if (self->proc_executando != NULL)
    return;

  for (int i = 0; i < self->qtd; i++) {
    proc_t *proc = self->proc_tabela[i];

    if (proc_estado(proc) == PROC_ESTADO_PRONTO) {
      esc_executa_proc(self, proc_id(proc));
      break;
    }
  }
}

proc_t *proc_cria(int id, int end)
{
  proc_t *self = malloc(sizeof(proc_t));
  assert(self != NULL);

  self->id = id;
  self->estado = PROC_ESTADO_PRONTO;

  self->reg_PC = end;
  self->reg_A = 0;
  self->reg_X = 0;

  self->bloq_motivo = 0;
  self->bloq_arg = 0;

  self->porta = -1;

  self->proximo = NULL;
  self->anterior = NULL;

  return self;
}

proc_estado_t proc_estado(proc_t *self)
{
  return self->estado;
}

void proc_destroi(proc_t *self)
{
  free(self);
}

int proc_id(proc_t *self)
{
  return self->id;
}

void proc_executa(proc_t *self)
{
  self->estado = PROC_ESTADO_EXECUTANDO;
}

void proc_para(proc_t *self)
{
  self->estado = PROC_ESTADO_PRONTO;
}

void proc_bloqueia(proc_t *self, proc_bloq_motivo_t motivo, int arg)
{
  self->estado = PROC_ESTADO_BLOQUEADO;
  self->bloq_motivo = motivo;
  self->bloq_arg = arg;
}

void proc_desbloqueia(proc_t *self)
{
  self->estado = PROC_ESTADO_PRONTO;
}

void proc_encerra(proc_t *self)
{
  self->estado = PROC_ESTADO_MORTO;
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
