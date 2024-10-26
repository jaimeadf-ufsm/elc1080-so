#include "esc.h"

#include <stdlib.h>
#include <assert.h>

#define TAM_TABELA_INICIAL 16

#define MODO ESC_MODO_SIMPLES
#define QUANTUM 5

struct esc_t
{
  esc_modo_t modo;

  int tam;
  int qtd;

  proc_t **proc_tabela;
  proc_t *proc_fila;

  proc_t *proc_corrente;

  int t_quantum;
  int t_restante;
};

struct proc_t
{
  int id;
  double prioridade;
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

double proc_prioridade(proc_t *self);
void proc_reprioriza(proc_t *self, double t_exec, double t_quantum);

void proc_executa(proc_t *self);
void proc_para(proc_t *self);
void proc_bloqueia(proc_t *self, proc_bloq_motivo_t motivo, int arg);
void proc_desbloqueia(proc_t *self);
void proc_encerra(proc_t *self);

void proc_insere_fila(proc_t *self, proc_t **fila);
void proc_remove_fila(proc_t *self, proc_t **fila);
void proc_avanca_fila(proc_t **fila);

void esc_escalona_proc_simples(esc_t *self);
void esc_escalona_proc_circular(esc_t *self);
void esc_escalona_proc_prioritario(esc_t *self);

esc_t *esc_cria()
{
  esc_t *self = malloc(sizeof(esc_t));
  assert(self != NULL);

  self->modo = MODO;

  self->tam = TAM_TABELA_INICIAL;
  self->qtd = 0;

  self->proc_tabela = malloc(self->tam * sizeof(proc_t *));
  assert(self->proc_tabela != NULL);

  self->proc_corrente = NULL;

  self->t_quantum = QUANTUM;
  self->t_restante = 0;

  return self;
}

void esc_destroi(esc_t *self)
{
  for (int i = 0; i < self->qtd; i++)
  {
    proc_destroi(self->proc_tabela[i]);
  }

  free(self->proc_tabela);
  free(self->proc_corrente);
}

int esc_proc_qtd(esc_t *self)
{
  return self->qtd;
}

proc_t **esc_proc_tabela(esc_t *self)
{
  return self->proc_tabela;
}

proc_t *esc_proc_corrente(esc_t *self)
{
  return self->proc_corrente;
}

proc_t *esc_busca_proc(esc_t *self, int id)
{
  if (id <= 0 || id > self->qtd) {
    return NULL;
  }

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
  proc_insere_fila(proc, &self->proc_fila);

  return proc;
}

bool esc_executa_proc(esc_t *self, int id)
{
  proc_t *proc = esc_busca_proc(self, id);

  if (proc == NULL) {
    return false;
  }
  
  if (self->proc_corrente != proc) {
    assert(proc_estado(proc) == PROC_ESTADO_PRONTO);

    if (
      (self->proc_corrente != NULL) &&
      (proc_estado(self->proc_corrente) == PROC_ESTADO_EXECUTANDO)
    ) {
      proc_para(self->proc_corrente);
    }

    proc_executa(proc);
  }

  self->proc_corrente = proc;
  self->t_restante = self->t_quantum;

  return true;
}

bool esc_bloqueia_proc(esc_t *self, int id, proc_bloq_motivo_t motivo, int arg)
{
  proc_t *proc = esc_busca_proc(self, id);

  if (proc == NULL) {
    return false;
  }
  
  proc_remove_fila(proc, &self->proc_fila);
  proc_bloqueia(proc, motivo, arg);

  return true;
}

bool esc_desbloqueia_proc(esc_t *self, int id)
{
  proc_t *proc = esc_busca_proc(self, id);

  if (proc == NULL) {
    return false;
  }

  proc_desbloqueia(proc);
  proc_insere_fila(proc, &self->proc_fila);

  return true;
}

bool esc_encerra_proc(esc_t *self, int id)
{
  proc_t *proc = esc_busca_proc(self, id);

  if (proc == NULL) {
    return false;
  }

  proc_remove_fila(proc, &self->proc_fila);
  proc_encerra(proc);

  return true;
}

void esc_escalona_proc(esc_t *self)
{
  if (
    (self->t_restante > 0) &&
    (self->proc_corrente != NULL) &&
    (proc_estado(self->proc_corrente) == PROC_ESTADO_EXECUTANDO)
  ) {
    return;
  }

  switch (self->modo) {
    case ESC_MODO_SIMPLES:
      esc_escalona_proc_simples(self);
      break;
    case ESC_MODO_CIRCULAR:
      esc_escalona_proc_circular(self);
      break;
    case ESC_MODO_PRIORITARIO:
      esc_escalona_proc_prioritario(self);
      break;
    default:
      assert(false);
  }

  if (
    (self->proc_corrente != NULL) &&
    (proc_estado(self->proc_corrente) != PROC_ESTADO_EXECUTANDO)
  ) {
    self->proc_corrente = NULL;
  }
}

void esc_escalona_proc_simples(esc_t *self)
{
  if (
    (self->proc_corrente != NULL) &&
    (proc_estado(self->proc_corrente) == PROC_ESTADO_EXECUTANDO)
  ) {
    return;
  }

  self->proc_corrente = NULL;

  for (int i = 0; i < self->qtd; i++) {
    proc_t *proc = self->proc_tabela[i];

    if (proc_estado(proc) == PROC_ESTADO_PRONTO) {
      esc_executa_proc(self, proc_id(proc));
      break;
    }
  }
}

void esc_escalona_proc_circular(esc_t *self)
{
  if (self->proc_fila == NULL) {
    return;
  }

  proc_t *proc = self->proc_fila;

  esc_executa_proc(self, proc_id(proc));
  proc_avanca_fila(&self->proc_fila);
}

void esc_escalona_proc_prioritario(esc_t *self)
{
  if (self->proc_corrente != NULL) {
    double t_exec = self->t_quantum - self->t_restante;
    proc_reprioriza(self->proc_corrente, t_exec, self->t_quantum);
  }

  proc_t *proc_prioritario = NULL;

  for (int i = 0; i < self->qtd; i++) {
    proc_t *proc = self->proc_tabela[i];

    if (
      (proc_estado(proc) != PROC_ESTADO_PRONTO) &&
      (proc_estado(proc) != PROC_ESTADO_EXECUTANDO)
    ) {
      continue;
    }

    if (
      (proc_prioritario == NULL) ||
      (proc_prioridade(proc) < proc_prioridade(proc_prioritario))
    ) {
      proc_prioritario = proc;
    }
  }

  if (proc_prioritario != NULL) {
    esc_executa_proc(self, proc_id(proc_prioritario));
  }
}

void esc_tictac(esc_t *self)
{
  self->t_restante--;
}

proc_t *proc_cria(int id, int end)
{
  proc_t *self = malloc(sizeof(proc_t));
  assert(self != NULL);

  self->id = id;
  self->prioridade = 0.5;
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

double proc_prioridade(proc_t *self)
{
  return self->prioridade;
}

void proc_reprioriza(proc_t *self, double t_exec, double t_quantum)
{
  self->prioridade = (self->prioridade + t_exec / t_quantum) / 2.0;
}

void proc_executa(proc_t *self)
{
  assert(self->estado == PROC_ESTADO_PRONTO);
  self->estado = PROC_ESTADO_EXECUTANDO;
}

void proc_para(proc_t *self)
{
  assert(self->estado == PROC_ESTADO_EXECUTANDO);
  self->estado = PROC_ESTADO_PRONTO;
}

void proc_bloqueia(proc_t *self, proc_bloq_motivo_t motivo, int arg)
{
  assert(
    (self->estado == PROC_ESTADO_EXECUTANDO) ||
    (self->estado == PROC_ESTADO_PRONTO)
  );

  self->estado = PROC_ESTADO_BLOQUEADO;
  self->bloq_motivo = motivo;
  self->bloq_arg = arg;
}

void proc_desbloqueia(proc_t *self)
{
  assert(self->estado == PROC_ESTADO_BLOQUEADO);
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

void proc_insere_fila(proc_t *self, proc_t **fila)
{
  if (*fila == NULL) {
    self->anterior = self;
    self->proximo = self;
    *fila = self;
  } else {
    proc_t *primeiro = *fila;
    proc_t *ultimo = primeiro->anterior;

    self->anterior = ultimo;
    self->proximo = primeiro;
    ultimo->proximo = self;
    primeiro->anterior = self;
  }
}

void proc_remove_fila(proc_t *self, proc_t **fila)
{
  if (*fila == NULL)
    return;
  
  if (self->proximo == NULL && self->anterior == NULL)
    return;

  if (self->proximo == self) {
    *fila = NULL;
  } else {
    self->proximo->anterior = self->anterior;
    self->anterior->proximo = self->proximo;

    if (*fila == self)
      *fila = self->proximo;
  }

  self->proximo = NULL;
  self->anterior = NULL;
}

void proc_avanca_fila(proc_t **fila)
{
  if (*fila == NULL)
    return;

  *fila = (*fila)->proximo;
}