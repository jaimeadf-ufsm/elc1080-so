#include "agenda.h"

#include <stdlib.h>
#include <assert.h>

struct agenda_t
{
    int latencia;
    int atraso;
};

agenda_t *agenda_cria(int latencia)
{
    agenda_t *self = (agenda_t *)malloc(sizeof(agenda_t));
    assert(self != NULL);

    self->latencia = latencia;
    self->atraso = 0;

    return self;
}

void agenda_destroi(agenda_t *self)
{
    free(self);
}

void agenda_sincroniza(agenda_t *self, int tempo)
{
    if (tempo > self->atraso)
    {
      self->atraso = tempo;
    }
}

int agenda_acessa(agenda_t *self)
{
    self->atraso += self->latencia;
    return self->atraso;
}