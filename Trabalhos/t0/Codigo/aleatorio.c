// aleatorio.h
// dispositivo de E/S para gerar números aleatórios
// simulador de computador
// so24b

#include <stdlib.h>
#include <assert.h>

#include "aleatorio.h"

struct aleatorio_t {
    unsigned int seed;
};

aleatorio_t *aleatorio_cria(unsigned int seed) {
    aleatorio_t *self;
    self = malloc(sizeof(aleatorio_t));
    assert(self != NULL);

    self->seed = seed;
    srand(seed);

    return self;
}

void aleatorio_destroi(aleatorio_t *self) {
    free(self);
}

err_t aleatorio_leitura(void *disp, int id, int *pvalor)
{
    aleatorio_t *self = disp;

    switch (id) {
        case 0:
            *pvalor = self->seed;
            break;
        case 1:
            *pvalor = rand();
            break;
        default:
            return ERR_END_INV;
    }

    return ERR_OK;
}