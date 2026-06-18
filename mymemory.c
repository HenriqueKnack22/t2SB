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

/*
 * Varredura de lacunas livres no pool.
 * A lista head é mantida ordenada por endereço; as lacunas são o espaço
 * antes do primeiro bloco, entre blocos consecutivos, e após o último.
 *
 * Retorna o ponteiro de início da lacuna escolhida, ou NULL se não houver
 * espaço suficiente.
 */
static char *find_gap(mymemory_t *memory, size_t size)
{
    char *pool_start = (char *)memory->pool;
    char *pool_end   = pool_start + memory->total_size;

    char *chosen      = NULL;
    size_t chosen_sz  = 0;

    char *prev_end = pool_start;
    allocation_t *cur = memory->head;

    for (;;) {
        char *gap_start = prev_end;
        char *gap_end   = (cur != NULL) ? (char *)cur->start : pool_end;
        size_t gap_sz   = (size_t)(gap_end - gap_start);

        if (gap_sz >= size) {
            switch (memory->strategy) {
            case STRATEGY_FIRST_FIT:
                return gap_start;

            case STRATEGY_BEST_FIT:
                if (chosen == NULL || gap_sz < chosen_sz) {
                    chosen    = gap_start;
                    chosen_sz = gap_sz;
                }
                break;

            case STRATEGY_WORST_FIT:
                if (chosen == NULL || gap_sz > chosen_sz) {
                    chosen    = gap_start;
                    chosen_sz = gap_sz;
                }
                break;
            }
        }

        if (cur == NULL) break;

        prev_end = (char *)cur->start + cur->size;
        cur = cur->next;
    }

    return chosen;
}

void* mymemory_alloc(mymemory_t *memory, size_t size)
{
    if (!memory || size == 0) return NULL;

    char *chosen = find_gap(memory, size);
    if (!chosen) return NULL;

    allocation_t *node = malloc(sizeof(allocation_t));
    if (!node) return NULL;

    node->start = chosen;
    node->size  = size;
    node->next  = NULL;

    /* Inserção na lista ordenada por endereço */
    if (memory->head == NULL || chosen < (char *)memory->head->start) {
        node->next   = memory->head;
        memory->head = node;
    } else {
        allocation_t *p = memory->head;
        while (p->next != NULL && (char *)p->next->start < chosen)
            p = p->next;
        node->next = p->next;
        p->next    = node;
    }

    return chosen;
}

void mymemory_free(mymemory_t *memory, void *ptr)
{
    if (!memory || !ptr) return;

    /* Rejeita ponteiros fora do pool */
    char *pool_start = (char *)memory->pool;
    char *pool_end   = pool_start + memory->total_size;
    if ((char *)ptr < pool_start || (char *)ptr >= pool_end) return;

    /* Remove da lista encadeada */
    if (memory->head == NULL) return;

    if (memory->head->start == ptr) {
        allocation_t *to_free = memory->head;
        memory->head = memory->head->next;
        free(to_free);
        return;
    }

    allocation_t *p = memory->head;
    while (p->next != NULL) {
        if (p->next->start == ptr) {
            allocation_t *to_free = p->next;
            p->next = to_free->next;
            free(to_free);
            return;
        }
        p = p->next;
    }
    /* ptr não corresponde a nenhuma alocação: no-op */
}

void mymemory_display(mymemory_t *memory)
{
    if (!memory) return;

    printf("=== Alocacoes atuais ===\n");

    allocation_t *cur = memory->head;
    int i = 0;
    while (cur != NULL) {
        size_t offset = (size_t)((char *)cur->start - (char *)memory->pool);
        printf("  [%d] offset=%-6zu  tamanho=%zu bytes\n", i, offset, cur->size);
        cur = cur->next;
        i++;
    }

    if (i == 0)
        printf("  (nenhuma alocacao)\n");

    printf("========================\n");
}

void mymemory_stats(mymemory_t *memory)
{
    if (!memory) return;

    char *pool_start = (char *)memory->pool;
    char *pool_end   = pool_start + memory->total_size;

    int    num_alloc       = 0;
    size_t total_allocated = 0;

    allocation_t *cur = memory->head;
    while (cur != NULL) {
        num_alloc++;
        total_allocated += cur->size;
        cur = cur->next;
    }

    size_t total_free   = memory->total_size - total_allocated;
    size_t largest_free = 0;
    int    num_frags    = 0;

    /* Reutiliza o mesmo padrão de varredura de lacunas */
    char *prev_end = pool_start;
    cur = memory->head;

    for (;;) {
        char *gap_end = (cur != NULL) ? (char *)cur->start : pool_end;
        size_t gap_sz = (size_t)(gap_end - prev_end);

        if (gap_sz > 0) {
            num_frags++;
            if (gap_sz > largest_free)
                largest_free = gap_sz;
        }

        if (cur == NULL) break;

        prev_end = (char *)cur->start + cur->size;
        cur = cur->next;
    }

    printf("=== Estatisticas ===\n");
    printf("  Alocacoes ativas:   %d\n",     num_alloc);
    printf("  Memoria alocada:    %zu bytes\n", total_allocated);
    printf("  Memoria livre:      %zu bytes\n", total_free);
    printf("  Maior bloco livre:  %zu bytes\n", largest_free);
    printf("  Fragmentos livres:  %d\n",     num_frags);
    printf("====================\n");
}

void mymemory_cleanup(mymemory_t *memory)
{
    if (!memory) return;

    allocation_t *cur = memory->head;
    while (cur != NULL) {
        allocation_t *next = cur->next;
        free(cur);
        cur = next;
    }

    free(memory->pool);
    free(memory);
}
