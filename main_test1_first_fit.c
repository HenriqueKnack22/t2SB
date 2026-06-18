/*
 * main_test1_first_fit.c — TESTE 1: Funcionalidades Básicas (FIRST FIT)
 *
 * Objetivo: verificar mymemory_alloc, mymemory_free, mymemory_display,
 *           mymemory_stats e mymemory_cleanup com a estratégia First Fit.
 *
 * Compile:
 *   gcc -o test1_first_fit  main_test1_first_fit.c  mymemory.c
 */

#include <stdio.h>
#include <stdlib.h>
#include "mymemory.h"

#define CHECK(cond, msg) \
    do { \
        if (!(cond)) { fprintf(stderr, "  [FALHA] %s\n", msg); falhas++; } \
        else          { printf(        "  [OK]    %s\n", msg);           } \
    } while(0)

int main(void)
{
    int falhas = 0;

    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║     TESTE 1 — Funcionalidades Básicas  [FIRST FIT]      ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n\n");

    /* ── 1. Inicialização ─────────────────────────────────────── */
    printf("[ 1 ] Inicialização do pool (2048 bytes)\n");
    mymemory_t *mem = mymemory_init(2048, STRATEGY_FIRST_FIT);
    CHECK(mem != NULL,                              "mymemory_init retornou ponteiro válido");
    CHECK(mem->pool != NULL,                        "mem->pool não é NULL");
    CHECK(mem->total_size == 2048,                  "total_size == 2048");
    CHECK(mem->head == NULL,                        "lista inicialmente vazia");
    CHECK(mem->strategy == STRATEGY_FIRST_FIT,      "estratégia == FIRST FIT");

    /* ── 2. Alocações simples ──────────────────────────────────── */
    printf("\n[ 2 ] Alocações simples\n");
    void *p10  = mymemory_alloc(mem, 10);
    void *p25  = mymemory_alloc(mem, 25);
    void *p50  = mymemory_alloc(mem, 50);
    void *p100 = mymemory_alloc(mem, 100);

    CHECK(p10  != NULL, "alloc(10)  retornou ponteiro válido");
    CHECK(p25  != NULL, "alloc(25)  retornou ponteiro válido");
    CHECK(p50  != NULL, "alloc(50)  retornou ponteiro válido");
    CHECK(p100 != NULL, "alloc(100) retornou ponteiro válido");

    char *pool_start = (char *)mem->pool;
    char *pool_end   = pool_start + mem->total_size;
    CHECK((char *)p10  >= pool_start && (char *)p10  < pool_end, "p10  dentro do pool");
    CHECK((char *)p25  >= pool_start && (char *)p25  < pool_end, "p25  dentro do pool");
    CHECK((char *)p50  >= pool_start && (char *)p50  < pool_end, "p50  dentro do pool");
    CHECK((char *)p100 >= pool_start && (char *)p100 < pool_end, "p100 dentro do pool");
    CHECK(p10 != p25 && p10 != p50 && p25 != p50,                "endereços distintos");

    /*
     * FIRST FIT: os blocos devem estar contíguos desde o offset 0,
     * pois a primeira lacuna encontrada é sempre a do início do pool.
     */
    CHECK((char *)p10  - pool_start == 0,  "FIRST FIT: p10  começa no offset 0");
    CHECK((char *)p25  - pool_start == 10, "FIRST FIT: p25  começa no offset 10");
    CHECK((char *)p50  - pool_start == 35, "FIRST FIT: p50  começa no offset 35");
    CHECK((char *)p100 - pool_start == 85, "FIRST FIT: p100 começa no offset 85");

    /* ── 3. Display e Stats ────────────────────────────────────── */
    printf("\n[ 3 ] Display e Stats (após 4 alocações)\n");
    mymemory_display(mem);
    mymemory_stats(mem);

    /* ── 4. Libertação e reutilização ────────────────────────── */
    printf("[ 4 ] Libertação e reutilização\n");
    mymemory_free(mem, p25);
    CHECK(1, "mymemory_free(p25) sem crash");

    /* First Fit: ao alocar 20 bytes, deve usar o buraco de 25 que
       começa no offset 10 (primeira lacuna suficiente). */
    void *p20 = mymemory_alloc(mem, 20);
    CHECK(p20 != NULL,                                    "alloc(20) após free(p25) OK");
    CHECK((char *)p20 - pool_start == 10, "FIRST FIT: p20 reutiliza o buraco (offset 10)");

    mymemory_display(mem);
    mymemory_stats(mem);

    /* ── 5. Ponteiro inválido no free ─────────────────────────── */
    printf("[ 5 ] Free com ponteiro inválido\n");
    int dummy = 0;
    mymemory_free(mem, &dummy);
    mymemory_free(mem, NULL);
    CHECK(1, "free com ponteiro inválido não crashou");

    /* ── 6. Alocação maior que o pool ────────────────────────── */
    printf("\n[ 6 ] Alocação maior que o espaço disponível\n");
    void *pbig = mymemory_alloc(mem, 99999);
    CHECK(pbig == NULL, "alloc(99999) retornou NULL");

    /* ── 7. Alocação de tamanho zero ─────────────────────────── */
    printf("\n[ 7 ] Alocação de tamanho zero\n");
    void *pzero = mymemory_alloc(mem, 0);
    CHECK(pzero == NULL, "alloc(0) retornou NULL");

    /* ── 8. Liberar tudo ─────────────────────────────────────── */
    printf("\n[ 8 ] Liberar todas as alocações\n");
    mymemory_free(mem, p10);
    mymemory_free(mem, p20);
    mymemory_free(mem, p50);
    mymemory_free(mem, p100);
    CHECK(mem->head == NULL, "lista vazia após liberar tudo");
    mymemory_display(mem);
    mymemory_stats(mem);

    /* ── 9. Alocar pool inteiro ───────────────────────────────── */
    printf("[ 9 ] Alocar o pool inteiro\n");
    void *pfull = mymemory_alloc(mem, 2048);
    CHECK(pfull != NULL,  "alloc(2048) no pool vazio OK");
    void *pover = mymemory_alloc(mem, 1);
    CHECK(pover == NULL,  "alloc(1) com pool cheio retornou NULL");
    mymemory_stats(mem);

    /* ── 10. Cleanup ─────────────────────────────────────────── */
    printf("\n[ 10 ] Cleanup\n");
    mymemory_cleanup(mem);
    CHECK(1, "mymemory_cleanup sem crash");

    printf("\n══════════════════════════════════════════\n");
    if (falhas == 0) printf("  RESULTADO: TODOS OS TESTES PASSARAM\n");
    else             printf("  RESULTADO: %d TESTE(S) FALHARAM\n", falhas);
    printf("══════════════════════════════════════════\n\n");

    return falhas;
}
