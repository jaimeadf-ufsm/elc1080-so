#ifndef TABELA_H
#define TABELA_H

typedef struct tabela_t tabela_t;

tabela_t *tabela_cria(int n_linhas, int n_colunas, int tam_celula);
void tabela_destroi(tabela_t *self);

int tabela_linhas(tabela_t *self);
int tabela_colunas(tabela_t *self);

void tabela_preenche(tabela_t *self, int i, int j, const char *formato, ...);

void tabela_mostra(tabela_t *self);

#endif