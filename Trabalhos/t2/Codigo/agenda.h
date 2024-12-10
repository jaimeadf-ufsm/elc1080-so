#ifndef AGENDA_H
#define AGENDA_H

typedef struct agenda_t agenda_t;

agenda_t *agenda_cria(int latencia);
void agenda_destroi(agenda_t *self);

void agenda_sincroniza(agenda_t *self, int tempo);

int agenda_acessa(agenda_t *self);

#endif