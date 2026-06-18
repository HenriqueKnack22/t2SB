#include <stdio.h>
#include <stdlib.h>
#include "mymemory.h"

mymemory_t* mymemory_init(size_t size, strategy_t strategy)
{
    if (size == 0) return NULL;

    mymemory_t *mem = malloc(sizeof(mymemory_t));
    if (!mem) return NULL;

    mem->pool = malloc(size);
    if (!mem->pool) {
        free(mem);
        return NULL;
    }

    mem->total_size = size;
    mem->head       = NULL;
    mem->strategy   = strategy;

    return mem;
}

void* mymemory_alloc(mymemory_t *memory, size_t size)
{
    if (!memory || size == 0) return NULL;

    char *base   = (char *)memory->pool;
    char *limite = base + memory->total_size;

    char         *melhor     = NULL;
    size_t        melhor_sz  = 0;
    char         *fim_ant    = base;
    allocation_t *cursor     = memory->head;

    while (1) {
        char  *ini_lacuna = fim_ant;
        char  *fim_lacuna = (cursor != NULL) ? (char *)cursor->start : limite;
        size_t sz_lacuna  = (size_t)(fim_lacuna - ini_lacuna);

        if (sz_lacuna >= size) {
            if (memory->strategy == STRATEGY_FIRST_FIT) {
                melhor = ini_lacuna;
                break;
            } else if (memory->strategy == STRATEGY_BEST_FIT) {
                if (melhor == NULL || sz_lacuna < melhor_sz) {
                    melhor    = ini_lacuna;
                    melhor_sz = sz_lacuna;
                }
            } else {
                if (melhor == NULL || sz_lacuna > melhor_sz) {
                    melhor    = ini_lacuna;
                    melhor_sz = sz_lacuna;
                }
            }
        }

        if (cursor == NULL) break;
        fim_ant = (char *)cursor->start + cursor->size;
        cursor  = cursor->next;
    }

    if (!melhor) return NULL;

    allocation_t *novo = malloc(sizeof(allocation_t));
    if (!novo) return NULL;

    novo->start = melhor;
    novo->size  = size;
    novo->next  = NULL;

    if (memory->head == NULL || melhor < (char *)memory->head->start) {
        novo->next   = memory->head;
        memory->head = novo;
    } else {
        allocation_t *p = memory->head;
        while (p->next != NULL && (char *)p->next->start < melhor)
            p = p->next;
        novo->next = p->next;
        p->next    = novo;
    }

    return melhor;
}

void mymemory_free(mymemory_t *memory, void *ptr)
{
    if (!memory || !ptr) return;

    char *base   = (char *)memory->pool;
    char *limite = base + memory->total_size;
    if ((char *)ptr < base || (char *)ptr >= limite) return;

    if (memory->head == NULL) return;

    if (memory->head->start == ptr) {
        allocation_t *alvo = memory->head;
        memory->head = alvo->next;
        free(alvo);
        return;
    }

    allocation_t *p = memory->head;
    while (p->next != NULL) {
        if (p->next->start == ptr) {
            allocation_t *alvo = p->next;
            p->next = alvo->next;
            free(alvo);
            return;
        }
        p = p->next;
    }
}

void mymemory_display(mymemory_t *memory)
{
    if (!memory) return;

    printf("=== Alocacoes atuais ===\n");

    allocation_t *cursor = memory->head;
    int i = 0;
    while (cursor != NULL) {
        size_t offset = (size_t)((char *)cursor->start - (char *)memory->pool);
        printf("  [%d] offset=%-6zu  tamanho=%zu bytes\n", i, offset, cursor->size);
        cursor = cursor->next;
        i++;
    }

    if (i == 0)
        printf("  (nenhuma alocacao)\n");

    printf("========================\n");
}

void mymemory_stats(mymemory_t *memory)
{
    if (!memory) return;

    int    num_alocacoes  = 0;
    size_t bytes_alocados = 0;

    allocation_t *cursor = memory->head;
    while (cursor != NULL) {
        num_alocacoes++;
        bytes_alocados += cursor->size;
        cursor = cursor->next;
    }

    char *base   = (char *)memory->pool;
    char *limite = base + memory->total_size;

    size_t bytes_livres   = memory->total_size - bytes_alocados;
    size_t maior_livre    = 0;
    int    num_fragmentos = 0;

    char *fim_ant = base;
    cursor = memory->head;

    while (1) {
        char  *fim_lacuna = (cursor != NULL) ? (char *)cursor->start : limite;
        size_t sz_lacuna  = (size_t)(fim_lacuna - fim_ant);

        if (sz_lacuna > 0) {
            num_fragmentos++;
            if (sz_lacuna > maior_livre)
                maior_livre = sz_lacuna;
        }

        if (cursor == NULL) break;
        fim_ant = (char *)cursor->start + cursor->size;
        cursor  = cursor->next;
    }

    printf("=== Estatisticas ===\n");
    printf("  Alocacoes ativas:   %d\n",       num_alocacoes);
    printf("  Memoria alocada:    %zu bytes\n", bytes_alocados);
    printf("  Memoria livre:      %zu bytes\n", bytes_livres);
    printf("  Maior bloco livre:  %zu bytes\n", maior_livre);
    printf("  Fragmentos livres:  %d\n",        num_fragmentos);
    printf("====================\n");
}

void mymemory_cleanup(mymemory_t *memory)
{
    if (!memory) return;

    allocation_t *cursor = memory->head;
    while (cursor != NULL) {
        allocation_t *proximo = cursor->next;
        free(cursor);
        cursor = proximo;
    }

    free(memory->pool);
    free(memory);
}
