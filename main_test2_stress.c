/*
 * main_test2_stress.c — TESTE 2: Estresse com ~50 Blocos (Seção 2.3)
 *
 * Objetivo: verificar as três estratégias com um conjunto de 50 blocos
 *           de tamanho variado e variação de ordem das alocações,
 *           conforme exigido pela especificação.
 *
 * Compile:
 *   gcc -o test2_stress  main_test2_stress.c  mymemory.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "mymemory.h"

#define CHECK(cond, msg) \
    do { \
        if (!(cond)) { fprintf(stderr, "  [FALHA] %s\n", msg); falhas++; } \
        else          { printf(        "  [OK]    %s\n", msg);           } \
    } while(0)

#define N 50
#define POOL_SIZE 8192

/* 50 tamanhos variados (3–500 bytes); soma ≈ 3900 bytes < POOL_SIZE */
static const size_t sizes[N] = {
    10,  25,   8,  50,  15, 100,   3, 250,   7, 200,
    12,  20,  30,  40,  60,  80,  90, 110, 130, 150,
     5,  70,  45,  35,  55, 180,  75, 120, 160,  95,
    22,  18,  65,  85, 140, 170, 190,  33,  44,  66,
    77,  88,  99, 111, 122,   9,  11, 150,  50,  30
};

static int run_strategy(strategy_t strat, const char *name)
{
    int falhas = 0;
    void *ptrs[N];

    printf("\n╔══════════════════════════════════════════════════════════╗\n");
    printf("║  TESTE 2 — Estresse 50 blocos  [%-24s]║\n", name);
    printf("╚══════════════════════════════════════════════════════════╝\n\n");

    /* ── 1. Init ──────────────────────────────────────────────── */
    printf("[ 1 ] Inicialização do pool (%d bytes)\n", POOL_SIZE);
    mymemory_t *mem = mymemory_init(POOL_SIZE, strat);
    CHECK(mem != NULL,            "mymemory_init retornou ponteiro válido");
    CHECK(mem->pool != NULL,      "mem->pool não é NULL");
    CHECK(mem->total_size == POOL_SIZE, "total_size correto");
    CHECK(mem->head == NULL,      "lista inicialmente vazia");

    char *pool_start = (char *)mem->pool;
    char *pool_end   = pool_start + mem->total_size;

    /* ── 2. Alocar 50 blocos ──────────────────────────────────── */
    printf("\n[ 2 ] Alocação de 50 blocos de tamanhos variados\n");
    int ok_count = 0;
    for (int i = 0; i < N; i++) {
        ptrs[i] = mymemory_alloc(mem, sizes[i]);
        if (ptrs[i] != NULL &&
            (char *)ptrs[i] >= pool_start &&
            (char *)ptrs[i] <  pool_end)
            ok_count++;
    }
    CHECK(ok_count == N, "todos os 50 blocos alocados dentro do pool");

    /* Endereços distintos entre si */
    int distintos = 1;
    for (int i = 0; i < N && distintos; i++)
        for (int j = i + 1; j < N && distintos; j++)
            if (ptrs[i] == ptrs[j]) distintos = 0;
    CHECK(distintos, "todos os 50 ponteiros são distintos");

    mymemory_stats(mem);

    /* ── 3. Liberar blocos de índice par (25 frees) ───────────── */
    printf("[ 3 ] Liberação dos blocos de índice par (25 liberações)\n");
    for (int i = 0; i < N; i += 2)
        mymemory_free(mem, ptrs[i]);

    /* Verificar que blocos ímpares ainda estão na lista */
    allocation_t *cur = mem->head;
    int count_impares = 0;
    while (cur != NULL) { count_impares++; cur = cur->next; }
    CHECK(count_impares == N / 2, "25 alocações restantes na lista após frees");

    mymemory_stats(mem);

    /* ── 4. Realocar 10 blocos — verificar escolha de lacuna ──── */
    printf("[ 4 ] Realocação de 10 blocos (verifica escolha de lacuna)\n");

    /*
     * Tamanhos pequenos garantem que sempre caibam em alguma lacuna
     * criada pelas liberações acima.
     */
    size_t re_sizes[10] = {5, 8, 3, 10, 7, 12, 6, 9, 4, 11};
    void  *re_ptrs[10];
    int re_ok = 0;

    for (int i = 0; i < 10; i++) {
        re_ptrs[i] = mymemory_alloc(mem, re_sizes[i]);
        if (re_ptrs[i] != NULL &&
            (char *)re_ptrs[i] >= pool_start &&
            (char *)re_ptrs[i] <  pool_end)
            re_ok++;
    }
    CHECK(re_ok == 10, "10 realocações bem-sucedidas após fragmentação");

    /* Precompute block offsets */
    size_t offsets[N];
    offsets[0] = 0;
    for (int i = 1; i < N; i++)
        offsets[i] = offsets[i-1] + sizes[i-1];
    size_t total_used = offsets[N-1] + sizes[N-1];
    size_t trailing   = POOL_SIZE - total_used;

    if (strat == STRATEGY_FIRST_FIT) {
        /* First Fit: primeira lacuna suficiente é no offset 0 (sizes[0] foi liberado) */
        CHECK((char *)re_ptrs[0] - pool_start == 0,
              "FIRST FIT: primeira realocação no offset mais baixo (0)");
    } else if (strat == STRATEGY_BEST_FIT) {
        /*
         * Best Fit com re_sizes[0]=5: menor lacuna >= 5 entre as lacunas
         * criadas pelos frees (índices pares) e a lacuna final do pool.
         */
        size_t best_gap = SIZE_MAX, best_off = 0;
        for (int i = 0; i < N; i += 2) {
            if (sizes[i] >= re_sizes[0] && sizes[i] < best_gap) {
                best_gap = sizes[i];
                best_off = offsets[i];
            }
        }
        if (trailing >= re_sizes[0] && trailing < best_gap)
            best_off = total_used;
        CHECK((char *)re_ptrs[0] - pool_start == (ptrdiff_t)best_off,
              "BEST FIT: alloc(5) escolheu a menor lacuna suficiente");
    } else {
        /*
         * Worst Fit com re_sizes[0]=5: maior lacuna >= 5 entre lacunas
         * dos índices pares liberados e a lacuna final do pool.
         */
        size_t worst_gap = 0, worst_off = 0;
        for (int i = 0; i < N; i += 2) {
            if (sizes[i] >= re_sizes[0] && sizes[i] > worst_gap) {
                worst_gap = sizes[i];
                worst_off = offsets[i];
            }
        }
        if (trailing >= re_sizes[0] && trailing > worst_gap)
            worst_off = total_used;
        CHECK((char *)re_ptrs[0] - pool_start == (ptrdiff_t)worst_off,
              "WORST FIT: alloc(5) escolheu a maior lacuna disponível");
    }

    mymemory_stats(mem);

    /* ── 5. Liberar tudo e verificar pool vazio ───────────────── */
    printf("[ 5 ] Liberar todos os blocos restantes\n");
    /* ímpares ainda alocados */
    for (int i = 1; i < N; i += 2)
        mymemory_free(mem, ptrs[i]);
    /* realocações */
    for (int i = 0; i < 10; i++)
        mymemory_free(mem, re_ptrs[i]);

    CHECK(mem->head == NULL, "lista vazia após liberar todos os blocos");
    mymemory_stats(mem);

    /* ── 6. Cleanup ──────────────────────────────────────────── */
    printf("[ 6 ] Cleanup\n");
    mymemory_cleanup(mem);
    CHECK(1, "mymemory_cleanup sem crash");

    printf("\n══════════════════════════════════════════\n");
    if (falhas == 0)
        printf("  [%s] RESULTADO: TODOS OS TESTES PASSARAM\n", name);
    else
        printf("  [%s] RESULTADO: %d TESTE(S) FALHARAM\n", name, falhas);
    printf("══════════════════════════════════════════\n");

    return falhas;
}

int main(void)
{
    int total = 0;

    total += run_strategy(STRATEGY_FIRST_FIT, "FIRST FIT");
    total += run_strategy(STRATEGY_BEST_FIT,  "BEST FIT");
    total += run_strategy(STRATEGY_WORST_FIT, "WORST FIT");

    printf("\n══════════════════════════════════════════\n");
    if (total == 0)
        printf("  RESULTADO GLOBAL: TODOS OS TESTES PASSARAM\n");
    else
        printf("  RESULTADO GLOBAL: %d TESTE(S) FALHARAM\n", total);
    printf("══════════════════════════════════════════\n\n");

    return total;
}
