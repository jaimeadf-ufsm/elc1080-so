// aleatorio.c
// dispositivo de E/S para gerar números aleatórios
// simulador de computador
// so24b

#ifndef ALEATORIO_H
#define ALEATORIO_H

#include "es.h"

typedef struct aleatorio_t aleatorio_t;

// cria e inicializa um gerador de número aleatórios
aleatorio_t *aleatorio_cria(unsigned int seed);

// destroi um gerador de números aleatórios
void aleatorio_destroi(aleatorio_t *self);

// Funções para acessar o gerador de números aleatórios como dispositivo de E/S, com id:
//     '0' para ler a seed
//     '1' para gerar um número aleatório
// Devem seguir o protocolo f_leitura_t e f_escrita_t declarados em es.h
err_t aleatorio_leitura(void *disp, int id, int *pvalor);

#endif