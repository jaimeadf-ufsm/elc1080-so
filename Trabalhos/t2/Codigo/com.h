#ifndef COM_H
#define COM_H

#include "es.h"

typedef struct com_t com_t;

com_t *com_cria(es_t *es);
void com_destroi(com_t *self);

int com_registra_porta(com_t *self, dispositivo_id_t leitura, dispositivo_id_t leitura_ok, dispositivo_id_t escrita, dispositivo_id_t escrita_ok);

bool com_le_porta(com_t *self, int id, int *pvalor);
bool com_escreve_porta(com_t *self, int id, int valor);

void com_reserva_porta(com_t *self, int id);
void com_libera_porta(com_t *self, int id);

int com_porta_disponivel(com_t *self);

#endif