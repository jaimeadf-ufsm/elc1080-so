// so.c
// sistema operacional
// simulador de computador
// so24b

// INCLUDES {{{1
#include "so.h"
#include "dispositivos.h"
#include "irq.h"
#include "programa.h"
#include "instrucao.h"

#include "com.h"
#include "esc.h"
#include "tabela.h"

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

// CONSTANTES E TIPOS {{{1
// intervalo entre interrupções do relógio
#define INTERVALO_INTERRUPCAO 50   // em instruções executadas
#define INTERVALO_QUANTUM 5 // em interrupções de relógio

#define TAM_TABELA_PROC 16

#define ESC_MODO ESC_MODO_SIMPLES

typedef struct so_metricas_t so_metricas_t;

struct so_metricas_t {
  int t_exec;
  int t_ocioso;

  int n_preempcoes;

  int n_irqs[N_IRQ];
};

struct so_t {
  cpu_t *cpu;
  mem_t *mem;
  es_t *es;
  console_t *console;
  com_t *com;
  esc_t *esc;

  int proc_tam;
  int proc_qtd;
  int proc_id;

  proc_t *proc_corrente;
  proc_t **proc_tabela;

  int i_proc_quantum;
  int i_proc_restante;

  int t_relogio_atual;

  so_metricas_t metricas;

  bool erro_interno;
  // t1: tabela de processos, processo corrente, pendências, etc
};


// função de tratamento de interrupção (entrada no SO)
static int so_trata_interrupcao(void *argC, int reg_A);

// função de gerenciamento de processos
static proc_t *so_busca_proc(so_t *self, int pid);
static proc_t *so_gera_proc(so_t *self, char *nome_do_executavel);
static void so_mata_proc(so_t *self, proc_t *proc);
static void so_executa_proc(so_t *self, proc_t *proc);
static void so_bloqueia_proc(so_t *self, proc_t *proc, proc_bloq_motivo_t motivo, int arg);
static void so_desbloqueia_proc(so_t *self, proc_t *proc);
static void so_assegura_porta_proc(so_t *self, proc_t *proc);
static bool so_deve_escalonar(so_t *self);
static bool so_tem_trabalho(so_t *self);

// funções para contabilização de métricas
static void so_atualiza_metricas(so_t *self, int delta);
static void so_imprime_metricas(so_t *self);

// funções auxiliares
// carrega o programa contido no arquivo na memória do processador; retorna end. inicial
static int so_carrega_programa(so_t *self, char *nome_do_executavel);
// copia para str da memória do processador, até copiar um 0 (retorna true) ou tam bytes
static bool copia_str_da_mem(int tam, char str[tam], mem_t *mem, int ender);

// CRIAÇÃO {{{1

so_t *so_cria(cpu_t *cpu, mem_t *mem, es_t *es, console_t *console)
{
  so_t *self = malloc(sizeof(*self));
  if (self == NULL) return NULL;

  self->cpu = cpu;
  self->mem = mem;
  self->es = es;
  self->console = console;
  self->esc = esc_cria(ESC_MODO);
  self->com = com_cria(es);

  self->proc_tam = TAM_TABELA_PROC;
  self->proc_qtd = 0;
  self->proc_id = 1;

  self->proc_corrente = NULL;
  self->proc_tabela = malloc(self->proc_tam * sizeof(proc_t *));

  self->i_proc_quantum = INTERVALO_QUANTUM;
  self->i_proc_restante = 0;

  self->t_relogio_atual = -1;

  self->metricas.t_exec = 0;
  self->metricas.t_ocioso = 0;

  self->metricas.n_preempcoes = 0;

  for (int i = 0; i < N_IRQ; i++) {
    self->metricas.n_irqs[i] = 0;
  }

  self->erro_interno = false;

  com_registra_porta(self->com, D_TERM_A_TECLADO, D_TERM_A_TECLADO_OK, D_TERM_A_TELA, D_TERM_A_TELA_OK);
  com_registra_porta(self->com, D_TERM_B_TECLADO, D_TERM_B_TECLADO_OK, D_TERM_B_TELA, D_TERM_B_TELA_OK);
  com_registra_porta(self->com, D_TERM_C_TECLADO, D_TERM_C_TECLADO_OK, D_TERM_C_TELA, D_TERM_C_TELA_OK);
  com_registra_porta(self->com, D_TERM_D_TECLADO, D_TERM_D_TECLADO_OK, D_TERM_D_TELA, D_TERM_D_TELA_OK);

  // quando a CPU executar uma instrução CHAMAC, deve chamar a função
  //   so_trata_interrupcao, com primeiro argumento um ptr para o SO
  cpu_define_chamaC(self->cpu, so_trata_interrupcao, self);

  // coloca o tratador de interrupção na memória
  // quando a CPU aceita uma interrupção, passa para modo supervisor, 
  //   salva seu estado à partir do endereço 0, e desvia para o endereço
  //   IRQ_END_TRATADOR
  // colocamos no endereço IRQ_END_TRATADOR o programa de tratamento
  //   de interrupção (escrito em asm). esse programa deve conter a 
  //   instrução CHAMAC, que vai chamar so_trata_interrupcao (como
  //   foi definido acima)
  int ender = so_carrega_programa(self, "trata_int.maq");
  if (ender != IRQ_END_TRATADOR) {
    console_printf("SO: problema na carga do programa de tratamento de interrupção");
    self->erro_interno = true;
  }

  // programa o relógio para gerar uma interrupção após INTERVALO_INTERRUPCAO
  if (es_escreve(self->es, D_RELOGIO_TIMER, INTERVALO_INTERRUPCAO) != ERR_OK) {
    console_printf("SO: problema na programação do timer");
    self->erro_interno = true;
  }

  return self;
}

void so_destroi(so_t *self)
{
  cpu_define_chamaC(self->cpu, NULL, NULL);
  com_destroi(self->com);
  esc_destroi(self->esc);

  for (int i = 0; i < self->proc_qtd; i++) {
    proc_destroi(self->proc_tabela[i]);
  }

  free(self->proc_tabela);
  free(self);
}


// TRATAMENTO DE INTERRUPÇÃO {{{1

// funções auxiliares para o tratamento de interrupção
static void so_salva_estado_da_cpu(so_t *self);
static void so_sincroniza(so_t *self);
static void so_trata_irq(so_t *self, int irq);
static void so_trata_bloq(so_t *self, proc_t *proc);
static void so_trata_pendencias(so_t *self);
static void so_escalona(so_t *self);
static int so_despacha(so_t *self);
static int so_desliga(so_t *self);

// função a ser chamada pela CPU quando executa a instrução CHAMAC, no tratador de
//   interrupção em assembly
// essa é a única forma de entrada no SO depois da inicialização
// na inicialização do SO, a CPU foi programada para chamar esta função para executar
//   a instrução CHAMAC
// a instrução CHAMAC só deve ser executada pelo tratador de interrupção
//
// o primeiro argumento é um ponteiro para o SO, o segundo é a identificação
//   da interrupção
// o valor retornado por esta função é colocado no registrador A, e pode ser
//   testado pelo código que está após o CHAMAC. No tratador de interrupção em
//   assembly esse valor é usado para decidir se a CPU deve retornar da interrupção
//   (e executar o código de usuário) ou executar PARA e ficar suspensa até receber
//   outra interrupção
static int so_trata_interrupcao(void *argC, int reg_A)
{
  so_t *self = argC;
  irq_t irq = reg_A;

  self->metricas.n_irqs[irq]++;

  // esse print polui bastante, recomendo tirar quando estiver com mais confiança
  // console_printf("SO: recebi IRQ %d (%s)", irq, irq_nome(irq));
  // salva o estado da cpu no descritor do processo que foi interrompido
  so_salva_estado_da_cpu(self);
  // sincroniza o tempo do sistema
  so_sincroniza(self);
  // faz o atendimento da interrupção
  so_trata_irq(self, irq);
  // faz o processamento independente da interrupção
  so_trata_pendencias(self);
  // escolhe o próximo processo a executar
  so_escalona(self);

  if (so_tem_trabalho(self)) {
    // recupera o estado do processo escolhido
    return so_despacha(self);
  } else {
    // para de executar o SO, desabilitando as interrupções de relógio
    return so_desliga(self);
  }
}

static void so_salva_estado_da_cpu(so_t *self)
{
  // t1: salva os registradores que compõem o estado da cpu no descritor do
  //   processo corrente. os valores dos registradores foram colocados pela
  //   CPU na memória, nos endereços IRQ_END_*
  // se não houver processo corrente, não faz nada

  proc_t *proc = self->proc_corrente;

  if (proc == NULL) {
    return;
  }
  
  int PC, A, X;
  mem_le(self->mem, IRQ_END_PC, &PC);
  mem_le(self->mem, IRQ_END_A, &A);
  mem_le(self->mem, IRQ_END_X, &X);

  proc_define_PC(proc, PC);
  proc_define_A(proc, A);
  proc_define_X(proc, X);
}

static void so_sincroniza(so_t *self)
{
  int t_relogio_anterior = self->t_relogio_atual;

  if (es_le(self->es, D_RELOGIO_INSTRUCOES, &self->t_relogio_atual) != ERR_OK) {
    console_printf("SO: problema na leitura do relógio");
    return;
  }

  if (t_relogio_anterior == -1) {
    return;
  }

  int delta = self->t_relogio_atual - t_relogio_anterior;

  so_atualiza_metricas(self, delta);
}

static void so_trata_pendencias(so_t *self)
{
  // t1: realiza ações que não são diretamente ligadas com a interrupção que
  //   está sendo atendida:
  // - E/S pendente
  // - desbloqueio de processos
  // - contabilidades

  for (int i = 0; i < self->proc_qtd; i++) {
    proc_t *proc = self->proc_tabela[i];

    if (proc_estado(proc) == PROC_ESTADO_BLOQUEADO) {
      so_trata_bloq(self, proc);
    }
  }
}

static void so_escalona(so_t *self)
{
  // escolhe o próximo processo a executar, que passa a ser o processo
  //   corrente; pode continuar sendo o mesmo de antes ou não
  // t1: na primeira versão, escolhe um processo caso o processo corrente não possa continuar
  //   executando. depois, implementar escalonador melhor
  if (!so_deve_escalonar(self)) {
    return;
  }

  if (self->proc_corrente != NULL) {
    int t_exec = self->i_proc_quantum - self->i_proc_restante;

    double prioridade = proc_prioridade(self->proc_corrente);
    prioridade += (double)t_exec / self->i_proc_quantum;
    prioridade /= 2.0;

    console_printf(
      "SO: processo %d - atualiza prioridade %lf -> %lf",
      proc_id(self->proc_corrente),
      proc_prioridade(self->proc_corrente),
      prioridade
    );

    proc_define_prioridade(self->proc_corrente, prioridade);
  }

  if (
    self->proc_corrente != NULL &&
    proc_estado(self->proc_corrente) == PROC_ESTADO_EXECUTANDO
  ) {
    esc_insere_proc(self->esc, self->proc_corrente);
  }

  proc_t *proc = esc_proximo(self->esc);

  if (proc != NULL) {
    console_printf("SO: escalona processo %d", proc_id(proc));
  } else {
    console_printf("SO: nenhum processo para escalonar");
  }

  so_executa_proc(self, proc);
}

static int so_despacha(so_t *self)
{
  // t1: se houver processo corrente, coloca o estado desse processo onde ele
  //   será recuperado pela CPU (em IRQ_END_*) e retorna 0, senão retorna 1
  // o valor retornado será o valor de retorno de CHAMAC
  if (self->erro_interno) return 1;

  proc_t *proc = self->proc_corrente;

  if (proc == NULL) {
    return 1;
  }
  
  int PC = proc_PC(proc);
  int A = proc_A(proc);
  int X = proc_X(proc);

  mem_escreve(self->mem, IRQ_END_PC, PC);
  mem_escreve(self->mem, IRQ_END_A, A);
  mem_escreve(self->mem, IRQ_END_X, X);

  return 0;
}

static int so_desliga(so_t *self)
{
  err_t e1, e2;
  e1 = es_escreve(self->es, D_RELOGIO_INTERRUPCAO, 0);
  e2 = es_escreve(self->es, D_RELOGIO_TIMER, 0);

  if (e1 != ERR_OK || e2 != ERR_OK) {
    console_printf("SO: problema de desarme do timer");
    self->erro_interno = true;
  }

  so_imprime_metricas(self);

  return 1;
}

// TRATAMENTO DE UM BLOQUEIO {{{1
static void so_trata_bloq_leitura(so_t *self, proc_t *proc);
static void so_trata_bloq_escrita(so_t *self, proc_t *proc);
static void so_trata_bloq_espera_proc(so_t *self, proc_t *proc);
static void so_trata_bloq_desconhecido(so_t *self, proc_t *proc);

static void so_trata_bloq(so_t *self, proc_t *proc)
{
  proc_bloq_motivo_t motivo = proc_bloq_motivo(proc);

  switch (motivo)
  {
  case PROC_BLOQ_LEITURA:
    so_trata_bloq_leitura(self, proc);
    break;
  case PROC_BLOQ_ESCRITA:
    so_trata_bloq_escrita(self, proc);
    break;
  case PROC_BLOQ_ESPERA_PROC:
    so_trata_bloq_espera_proc(self, proc);
    break;
  default:
    so_trata_bloq_desconhecido(self, proc);
    break;
  }
}

static void so_trata_bloq_leitura(so_t *self, proc_t *proc)
{
  so_assegura_porta_proc(self, proc);

  int porta = proc_porta(proc);
  int dado;

  if (porta != -1 && com_le_porta(self->com, porta, &dado)) {
    proc_define_A(proc, dado);
    so_desbloqueia_proc(self, proc);

    console_printf("SO: processo %d - desbloqueia de leitura", proc_id(proc));
  }
}

static void so_trata_bloq_escrita(so_t *self, proc_t *proc)
{
  so_assegura_porta_proc(self, proc);

  int porta = proc_porta(proc);
  int dado = proc_bloq_arg(proc);

  if (porta != -1 && com_escreve_porta(self->com, porta, dado)) {
    proc_define_A(proc, 0);
    so_desbloqueia_proc(self, proc);

    console_printf("SO: processo %d - desbloqueia de escrita", proc_id(proc));
  }
}

static void so_trata_bloq_espera_proc(so_t *self, proc_t *proc)
{
  int pid_alvo = proc_bloq_arg(proc);
  proc_t *proc_alvo = so_busca_proc(self, pid_alvo);

  if (proc_alvo == NULL || proc_estado(proc_alvo) == PROC_ESTADO_MORTO) {
    proc_define_A(proc, 0);
    so_desbloqueia_proc(self, proc);

    console_printf(
      "SO: processo %d - desbloqueia de espera de processo",
      proc_id(proc)
    );
  }
}

static void so_trata_bloq_desconhecido(so_t *self, proc_t *proc)
{
  console_printf("SO: não sei tratar bloqueio por motivo %d", proc_bloq_motivo(proc));
  self->erro_interno = true;
}

// TRATAMENTO DE UMA IRQ {{{1

// funções auxiliares para tratar cada tipo de interrupção
static void so_trata_irq_reset(so_t *self);
static void so_trata_irq_chamada_sistema(so_t *self);
static void so_trata_irq_err_cpu(so_t *self);
static void so_trata_irq_relogio(so_t *self);
static void so_trata_irq_desconhecida(so_t *self, int irq);

static void so_trata_irq(so_t *self, int irq)
{
  // verifica o tipo de interrupção que está acontecendo, e atende de acordo
  switch (irq) {
    case IRQ_RESET:
      so_trata_irq_reset(self);
      break;
    case IRQ_SISTEMA:
      so_trata_irq_chamada_sistema(self);
      break;
    case IRQ_ERR_CPU:
      so_trata_irq_err_cpu(self);
      break;
    case IRQ_RELOGIO:
      so_trata_irq_relogio(self);
      break;
    default:
      so_trata_irq_desconhecida(self, irq);
  }
}

// interrupção gerada uma única vez, quando a CPU inicializa
static void so_trata_irq_reset(so_t *self)
{
  // t1: deveria criar um processo para o init, e inicializar o estado do
  //   processador para esse processo com os registradores zerados, exceto
  //   o PC e o modo.
  // como não tem suporte a processos, está carregando os valores dos
  //   registradores diretamente para a memória, de onde a CPU vai carregar
  //   para os seus registradores quando executar a instrução RETI

  // coloca o programa init na memória
  proc_t *proc = so_gera_proc(self, "init.maq");

  if (proc_PC(proc) != 100) {
    console_printf("SO: problema na carga do programa inicial");
    self->erro_interno = true;
    return;
  }

  // altera o PC para o endereço de carga
  // mem_escreve(self->mem, IRQ_END_PC, ender);
  // passa o processador para modo usuário
  mem_escreve(self->mem, IRQ_END_modo, usuario);
}

// interrupção gerada quando a CPU identifica um erro
static void so_trata_irq_err_cpu(so_t *self)
{
  // Ocorreu um erro interno na CPU
  // O erro está codificado em IRQ_END_erro
  // Em geral, causa a morte do processo que causou o erro
  // Ainda não temos processos, causa a parada da CPU
  int err_int;
  // t1: com suporte a processos, deveria pegar o valor do registrador erro
  //   no descritor do processo corrente, e reagir de acordo com esse erro
  //   (em geral, matando o processo)
  mem_le(self->mem, IRQ_END_erro, &err_int);
  err_t err = err_int;
  console_printf("SO: IRQ não tratada -- erro na CPU: %s", err_nome(err));
  self->erro_interno = true;
}

// interrupção gerada quando o timer expira
static void so_trata_irq_relogio(so_t *self)
{
  // rearma o interruptor do relógio e reinicializa o timer para a próxima interrupção
  err_t e1, e2;
  e1 = es_escreve(self->es, D_RELOGIO_INTERRUPCAO, 0); // desliga o sinalizador de interrupção
  e2 = es_escreve(self->es, D_RELOGIO_TIMER, INTERVALO_INTERRUPCAO);
  if (e1 != ERR_OK || e2 != ERR_OK) {
    console_printf("SO: problema da reinicialização do timer");
    self->erro_interno = true;
  }
  // t1: deveria tratar a interrupção
  //   por exemplo, decrementa o quantum do processo corrente, quando se tem
  //   um escalonador com quantum

  console_printf("SO: interrupção do relógio");

  if (self->i_proc_restante > 0) {
    self->i_proc_restante--;
  }
}

// foi gerada uma interrupção para a qual o SO não está preparado
static void so_trata_irq_desconhecida(so_t *self, int irq)
{
  console_printf("SO: não sei tratar IRQ %d (%s)", irq, irq_nome(irq));
  self->erro_interno = true;
}

// CHAMADAS DE SISTEMA {{{1

// funções auxiliares para cada chamada de sistema
static void so_chamada_le(so_t *self);
static void so_chamada_escr(so_t *self);
static void so_chamada_cria_proc(so_t *self);
static void so_chamada_mata_proc(so_t *self);
static void so_chamada_espera_proc(so_t *self);

static void so_trata_irq_chamada_sistema(so_t *self)
{
  // a identificação da chamada está no registrador A
  // t1: com processos, o reg A tá no descritor do processo corrente
  int id_chamada;
  if (mem_le(self->mem, IRQ_END_A, &id_chamada) != ERR_OK) {
    console_printf("SO: erro no acesso ao id da chamada de sistema");
    self->erro_interno = true;
    return;
  }
  // console_printf("SO: chamada de sistema %d", id_chamada);
  switch (id_chamada) {
    case SO_LE:
      so_chamada_le(self);
      break;
    case SO_ESCR:
      so_chamada_escr(self);
      break;
    case SO_CRIA_PROC:
      so_chamada_cria_proc(self);
      break;
    case SO_MATA_PROC:
      so_chamada_mata_proc(self);
      break;
    case SO_ESPERA_PROC:
      so_chamada_espera_proc(self);
      break;
    default:
      console_printf("SO: chamada de sistema desconhecida (%d)", id_chamada);
      // t1: deveria matar o processo
      self->erro_interno = true;
  }
}

// implementação da chamada se sistema SO_LE
// faz a leitura de um dado da entrada corrente do processo, coloca o dado no reg A
static void so_chamada_le(so_t *self)
{
  // implementação com espera ocupada
  //   T1: deveria realizar a leitura somente se a entrada estiver disponível,
  //     senão, deveria bloquear o processo.
  //   no caso de bloqueio do processo, a leitura (e desbloqueio) deverá
  //     ser feita mais tarde, em tratamentos pendentes em outra interrupção,
  //     ou diretamente em uma interrupção específica do dispositivo, se for
  //     o caso
  // implementação lendo direto do terminal A
  //   T1: deveria usar dispositivo de entrada corrente do processo
  proc_t *proc = self->proc_corrente;

  if (proc == NULL) {
    return;
  }
  
  // escreve no reg A do processador
  // (na verdade, na posição onde o processador vai pegar o A quando retornar da int)
  // T1: se houvesse processo, deveria escrever no reg A do processo
  // T1: o acesso só deve ser feito nesse momento se for possível; se não, o processo
  //   é bloqueado, e o acesso só deve ser feito mais tarde (e o processo desbloqueado)
  so_assegura_porta_proc(self, proc);

  int porta = proc_porta(proc);
  int dado;

  if (porta != -1 && com_le_porta(self->com, porta, &dado)) {
    proc_define_A(proc, dado);
  } else {
    console_printf("SO: processo %d - bloqueia para leitura", proc_id(proc));
    so_bloqueia_proc(self, proc, PROC_BLOQ_LEITURA, 0);
  }
}

// implementação da chamada se sistema SO_ESCR
// escreve o valor do reg X na saída corrente do processo
static void so_chamada_escr(so_t *self)
{
  // implementação com espera ocupada
  //   T1: deveria bloquear o processo se dispositivo ocupado
  // implementação escrevendo direto do terminal A
  //   T1: deveria usar o dispositivo de saída corrente do processo
  proc_t *proc = self->proc_corrente;

  if (proc == NULL) {
    return;
  }

  // está lendo o valor de X e escrevendo o de A direto onde o processador colocou/vai pegar
  // T1: deveria usar os registradores do processo que está realizando a E/S
  // T1: caso o processo tenha sido bloqueado, esse acesso deve ser realizado em outra execução
  //   do SO, quando ele verificar que esse acesso já pode ser feito.
  so_assegura_porta_proc(self, proc);

  int porta = proc_porta(proc);
  int dado = proc_X(proc);

  if (porta != -1 && com_escreve_porta(self->com, porta, dado)) {
    proc_define_A(proc, 0);
  } else {
    console_printf("SO: processo %d - bloqueia para escrita", proc_id(proc));
    so_bloqueia_proc(self, proc, PROC_BLOQ_ESCRITA, dado);
  }
}

// implementação da chamada se sistema SO_CRIA_PROC
// cria um processo
static void so_chamada_cria_proc(so_t *self)
{
  // ainda sem suporte a processos, carrega programa e passa a executar ele
  // quem chamou o sistema não vai mais ser executado, coitado!
  // T1: deveria criar um novo processo
  proc_t *proc = self->proc_corrente;

  if (proc == NULL) {
    return;
  }

  // em X está o endereço onde está o nome do arquivo
  // t1: deveria ler o X do descritor do processo criador
  int ender_proc = proc_X(proc);
  char nome[100];

  if (copia_str_da_mem(100, nome, self->mem, ender_proc)) {
    console_printf(
      "SO: processo %d - cria processo (nome: %s)",
      proc_id(self->proc_corrente),
      nome
    );

    proc_t *proc_alvo = so_gera_proc(self, nome);

    if (proc_alvo != NULL) {
      // t1: deveria escrever no PC do descritor do processo criado
      proc_define_A(proc, proc_id(proc_alvo));
      return;
    } // else?
  }
  // deveria escrever -1 (se erro) ou o PID do processo criado (se OK) no reg A
  //   do processo que pediu a criação
  proc_define_A(proc, -1);
}

// implementação da chamada se sistema SO_MATA_PROC
// mata o processo com pid X (ou o processo corrente se X é 0)
static void so_chamada_mata_proc(so_t *self)
{
  proc_t *proc = self->proc_corrente;

  if (proc == NULL) {
    return;
  }


  // T1: deveria matar um processo
  // ainda sem suporte a processos, retorna erro -1
  int pid_alvo = proc_X(proc);
  proc_t *proc_alvo = so_busca_proc(self, pid_alvo);

  console_printf(
    "SO: processo %d - mata processo (PID: %d)",
    proc_id(self->proc_corrente),
    pid_alvo
  );

  if (pid_alvo == 0) {
    proc_alvo = self->proc_corrente;
  }
  
  if (proc_alvo != NULL) {
    so_mata_proc(self, proc_alvo);
    proc_define_A(proc, 0);
  } else {
    proc_define_A(proc, -1);
  }
}

// implementação da chamada se sistema SO_ESPERA_PROC
// espera o fim do processo com pid X
static void so_chamada_espera_proc(so_t *self)
{
  proc_t *proc = self->proc_corrente;

  if (proc == NULL) {
    return;
  }

  // T1: deveria bloquear o processo se for o caso (e desbloquear na morte do esperado)
  // ainda sem suporte a processos, retorna erro -1
  int pid_alvo = proc_X(proc);
  proc_t *proc_alvo = so_busca_proc(self, pid_alvo);

  console_printf(
    "SO: processo %d - espera processo (PID: %d)",
    proc_id(self->proc_corrente),
    pid_alvo
  );

  if (proc_alvo == NULL || proc_alvo == proc) {
    proc_define_A(proc, -1);
    return;
  }

  if (proc_estado(proc_alvo) == PROC_ESTADO_MORTO) {
    proc_define_A(proc, 0);
  } else {
    console_printf(
    "SO: processo %d - bloqueia para espera de processo",
     proc_id(proc)
    );

    so_bloqueia_proc(self, proc, PROC_BLOQ_ESPERA_PROC, pid_alvo);
  }
}

// GERENCIAMENTO DE PROCESSOS{{{1
static proc_t *so_busca_proc(so_t *self, int pid)
{
  for (int i = 0; i < self->proc_qtd; i++) {
    if (proc_id(self->proc_tabela[i]) == pid) {
      return self->proc_tabela[i];
    }
  }

  return NULL;
}

static proc_t *so_gera_proc(so_t *self, char *nome_do_executavel)
{
  int end = so_carrega_programa(self, nome_do_executavel);

  if (end <= 0) {
    return NULL;
  }

  if (self->proc_qtd == self->proc_tam) {
    self->proc_tam = self->proc_tam * 2;
    self->proc_tabela = realloc(self->proc_tabela, self->proc_tam * sizeof(*self->proc_tabela));
    assert(self->proc_tabela != NULL);
  }

  proc_t *proc = proc_cria(self->proc_id++, end);

  self->proc_tabela[self->proc_qtd++] = proc;
  esc_insere_proc(self->esc, proc);

  return proc;
}

static void so_mata_proc(so_t *self, proc_t *proc)
{
  int porta = proc_porta(proc);

  if (porta != -1) {
    proc_desatribui_porta(proc);
    com_libera_porta(self->com, porta);
  }

  esc_remove_proc(self->esc, proc);
  proc_encerra(proc);
}

static void so_executa_proc(so_t *self, proc_t *proc)
{
  if (
    self->proc_corrente != NULL &&
    self->proc_corrente != proc &&
    proc_estado(self->proc_corrente) == PROC_ESTADO_EXECUTANDO
  ) {
    proc_para(self->proc_corrente);
    self->metricas.n_preempcoes++;
  }

  if (proc != NULL && proc_estado(proc) != PROC_ESTADO_EXECUTANDO) {
    proc_executa(proc);
  }

  if (proc != NULL) {
    esc_remove_proc(self->esc, proc);
  }

  self->proc_corrente = proc;
  self->i_proc_restante = self->i_proc_quantum;
}

static void so_bloqueia_proc(so_t *self, proc_t *proc, proc_bloq_motivo_t motivo, int arg)
{
  esc_remove_proc(self->esc, proc);
  proc_bloqueia(proc, motivo, arg);
}

static void so_desbloqueia_proc(so_t *self, proc_t *proc)
{
  proc_desbloqueia(proc);
  esc_insere_proc(self->esc, proc);
}

static void so_assegura_porta_proc(so_t *self, proc_t *proc)
{
  if (proc_porta(proc) != -1) {
    return;
  }

  int porta = com_porta_disponivel(self->com);
  proc_atribui_porta(proc, porta);
  com_reserva_porta(self->com, porta);
}

static bool so_deve_escalonar(so_t *self)
{
  if (self->proc_corrente == NULL) {
    return true;
  }

  if (proc_estado(self->proc_corrente) != PROC_ESTADO_EXECUTANDO) {
    return true;
  }

  if (self->i_proc_restante <= 0) {
    return true;
  }

  return false;
}

static bool so_tem_trabalho(so_t *self)
{
  for (int i = 0; i < self->proc_qtd; i++) {
    if (proc_estado(self->proc_tabela[i]) != PROC_ESTADO_MORTO) {
      return true;
    }
  }

  return false;
}

// MÉTRICAS {{1

static void so_atualiza_metricas(so_t *self, int delta)
{
  self->metricas.t_exec += delta;
  
  if (self->proc_corrente == NULL) {
    self->metricas.t_ocioso += delta;
  }

  for (int i = 0; i < self->proc_qtd; i++) {
    proc_atualiza_metricas(self->proc_tabela[i], delta);
  }
}

static void so_imprime_metricas(so_t *self)
{
  const int TAM_CELULA = 256;

  tabela_t *tabela_so = tabela_cria(2, 4, TAM_CELULA);
  
  tabela_preenche(tabela_so, 0, 0, "$N_{\\text{Processos}}$");
  tabela_preenche(tabela_so, 0, 1, "$T_{\\text{Execução}}$");
  tabela_preenche(tabela_so, 0, 2, "$T_{\\text{Ocioso}}$");
  tabela_preenche(tabela_so, 0, 3, "$N_{\\text{Preempções}}$");
  tabela_preenche(tabela_so, 1, 0, "%d", self->proc_qtd);
  tabela_preenche(tabela_so, 1, 1, "%d", self->metricas.t_exec);
  tabela_preenche(tabela_so, 1, 2, "%d", self->metricas.t_ocioso);
  tabela_preenche(tabela_so, 1, 3, "%d", self->metricas.n_preempcoes);

  tabela_t *tabela_irqs = tabela_cria(N_IRQ + 1, 2, TAM_CELULA);

  tabela_preenche(tabela_irqs, 0, 0, "IRQ");
  tabela_preenche(tabela_irqs, 0, 1, "$N_{\\text{Vezes}}$");

  for (int i = 0; i < N_IRQ; i++) {
    tabela_preenche(tabela_irqs, i + 1, 0, irq_nome(i));
    tabela_preenche(tabela_irqs, i + 1, 1, "%d", self->metricas.n_irqs[i]);
  }

  tabela_t *tabela_proc_geral = tabela_cria(self->proc_qtd + 1, 4, TAM_CELULA);
  tabela_t *tabela_proc_est_vezes = tabela_cria(self->proc_qtd + 1, N_PROC_ESTADO + 1, TAM_CELULA);
  tabela_t *tabela_proc_est_tempo = tabela_cria(self->proc_qtd + 1, N_PROC_ESTADO + 1, TAM_CELULA);

  tabela_preenche(tabela_proc_geral, 0, 0, "PID");
  tabela_preenche(tabela_proc_geral, 0, 1, "$N_{\\text{Preempções}}$");
  tabela_preenche(tabela_proc_geral, 0, 2, "$T_{\\text{Retorno}}$");
  tabela_preenche(tabela_proc_geral, 0, 3, "$T_{\\text{Resposta}}$");

  tabela_preenche(tabela_proc_est_vezes, 0, 0, "PID");
  tabela_preenche(tabela_proc_est_tempo, 0, 0, "PID");

  for (int i = 0; i < N_PROC_ESTADO; i++) {
    tabela_preenche(tabela_proc_est_vezes, 0, i + 1, "$N_{\\text{%s}}$", proc_estado_nome(i));
    tabela_preenche(tabela_proc_est_tempo, 0, i + 1, "$T_{\\text{%s}}$", proc_estado_nome(i));
  }

  for (int i = 0; i < self->proc_qtd; i++) {
    proc_t *proc = self->proc_tabela[i];

    proc_metricas_t proc_metricas_atual = proc_metricas(self->proc_tabela[i]);

    tabela_preenche(tabela_proc_geral, i + 1, 0, "%d", proc_id(proc));
    tabela_preenche(tabela_proc_geral, i + 1, 1, "%d", proc_metricas_atual.n_preempcoes);
    tabela_preenche(tabela_proc_geral, i + 1, 2, "%d", proc_metricas_atual.t_retorno);
    tabela_preenche(tabela_proc_geral, i + 1, 3, "%d", proc_metricas_atual.t_resposta);

    tabela_preenche(tabela_proc_est_vezes, i + 1, 0, "%d", proc_id(proc));
    tabela_preenche(tabela_proc_est_tempo, i + 1, 0, "%d", proc_id(proc));

    for (int j = 0; j < N_PROC_ESTADO; j++) {
      tabela_preenche(tabela_proc_est_vezes, i + 1, j + 1, "%d", proc_metricas_atual.estados[j].n_vezes);
      tabela_preenche(tabela_proc_est_tempo, i + 1, j + 1, "%d", proc_metricas_atual.estados[j].t_total);
    }
  }

  tabela_mostra(tabela_so);
  tabela_mostra(tabela_irqs);
  tabela_mostra(tabela_proc_geral);
  tabela_mostra(tabela_proc_est_vezes);
  tabela_mostra(tabela_proc_est_tempo);

  tabela_destroi(tabela_so);
  tabela_destroi(tabela_proc_geral);
  tabela_destroi(tabela_proc_est_vezes);
  tabela_destroi(tabela_proc_est_tempo);
}

// CARGA DE PROGRAMA {{{1

// carrega o programa na memória
// retorna o endereço de carga ou -1
static int so_carrega_programa(so_t *self, char *nome_do_executavel)
{
  // programa para executar na nossa CPU
  programa_t *prog = prog_cria(nome_do_executavel);
  if (prog == NULL) {
    console_printf("Erro na leitura do programa '%s'\n", nome_do_executavel);
    return -1;
  }

  int end_ini = prog_end_carga(prog);
  int end_fim = end_ini + prog_tamanho(prog);

  for (int end = end_ini; end < end_fim; end++) {
    if (mem_escreve(self->mem, end, prog_dado(prog, end)) != ERR_OK) {
      console_printf("Erro na carga da memória, endereco %d\n", end);
      return -1;
    }
  }

  prog_destroi(prog);
  console_printf("SO: carga de '%s' em %d-%d", nome_do_executavel, end_ini, end_fim);
  return end_ini;
}

// ACESSO À MEMÓRIA DOS PROCESSOS {{{1

// copia uma string da memória do simulador para o vetor str.
// retorna false se erro (string maior que vetor, valor não char na memória,
//   erro de acesso à memória)
// T1: deveria verificar se a memória pertence ao processo
static bool copia_str_da_mem(int tam, char str[tam], mem_t *mem, int ender)
{
  for (int indice_str = 0; indice_str < tam; indice_str++) {
    int caractere;
    if (mem_le(mem, ender + indice_str, &caractere) != ERR_OK) {
      return false;
    }
    if (caractere < 0 || caractere > 255) {
      return false;
    }
    str[indice_str] = caractere;
    if (caractere == 0) {
      return true;
    }
  }
  // estourou o tamanho de str
  return false;
}

// vim: foldmethod=marker
