#include <stdio.h>
#include <stdlib.h>
#include "mymemory.h"

#define MAX_ALOCACOES 256

static mymemory_t *mem = NULL;
static void       *alocados[MAX_ALOCACOES];
static int         num_alocados = 0;

/* Descarta os caracteres restantes no buffer de entrada ate encontrar '\n' ou EOF. */
static void limpar_buffer(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/* Solicita ao usuario o tamanho e a estrategia de alocacao, inicializa e retorna o pool de memoria. */
static mymemory_t *configurar_pool(void)
{
    size_t tamanho;
    int    escolha;

    printf("\n╔══════════════════════════════════════╗\n");
    printf("║     CONFIGURACAO DO POOL             ║\n");
    printf("╚══════════════════════════════════════╝\n");

    printf("Tamanho do pool (bytes): ");
    if (scanf("%zu", &tamanho) != 1 || tamanho == 0) {
        printf("Tamanho invalido. Usando 2048.\n");
        tamanho = 2048;
    }
    limpar_buffer();

    printf("\nEstrategia de alocacao:\n");
    printf("  [1] First Fit\n");
    printf("  [2] Best Fit\n");
    printf("  [3] Worst Fit\n");
    printf("Escolha: ");
    if (scanf("%d", &escolha) != 1) escolha = 1;
    limpar_buffer();

    strategy_t estrategia;
    const char *nome;
    switch (escolha) {
    case 2:  estrategia = STRATEGY_BEST_FIT;  nome = "Best Fit";  break;
    case 3:  estrategia = STRATEGY_WORST_FIT; nome = "Worst Fit"; break;
    default: estrategia = STRATEGY_FIRST_FIT; nome = "First Fit"; break;
    }

    mymemory_t *novo = mymemory_init(tamanho, estrategia);
    if (!novo) {
        printf("Erro ao criar pool.\n");
        return NULL;
    }

    printf("\nPool criado: %zu bytes | estrategia: %s\n", tamanho, nome);
    return novo;
}

/* Le o tamanho desejado, aloca no pool e registra o ponteiro no vetor de alocacoes ativas. */
static void alocar(void)
{
    if (num_alocados >= MAX_ALOCACOES) {
        printf("Limite de rastreamento (%d) atingido.\n", MAX_ALOCACOES);
        return;
    }

    size_t tamanho;
    printf("Tamanho a alocar (bytes): ");
    if (scanf("%zu", &tamanho) != 1 || tamanho == 0) {
        printf("Valor invalido.\n");
        limpar_buffer();
        return;
    }
    limpar_buffer();

    void *ptr = mymemory_alloc(mem, tamanho);
    if (!ptr) {
        printf("  [FALHA] Sem espaco disponivel para %zu bytes.\n", tamanho);
        return;
    }

    alocados[num_alocados++] = ptr;
    size_t offset = (size_t)((char *)ptr - (char *)mem->pool);
    printf("  [OK] Alocado %zu bytes no offset %zu (indice %d)\n",
           tamanho, offset, num_alocados - 1);
}

/* Lista as alocacoes ativas, solicita o indice a liberar e devolve o bloco ao pool. */
static void liberar(void)
{
    if (num_alocados == 0) {
        printf("Nenhuma alocacao para liberar.\n");
        return;
    }

    printf("Alocacoes disponiveis:\n");
    for (int i = 0; i < num_alocados; i++) {
        if (!alocados[i]) continue;
        size_t offset = (size_t)((char *)alocados[i] - (char *)mem->pool);

        allocation_t *cur = mem->head;
        size_t sz = 0;
        while (cur) {
            if (cur->start == alocados[i]) { sz = cur->size; break; }
            cur = cur->next;
        }
        printf("  [%d] offset=%-6zu  tamanho=%zu bytes\n", i, offset, sz);
    }

    int idx;
    printf("Indice a liberar: ");
    if (scanf("%d", &idx) != 1 || idx < 0 || idx >= num_alocados || !alocados[idx]) {
        printf("Indice invalido.\n");
        limpar_buffer();
        return;
    }
    limpar_buffer();

    mymemory_free(mem, alocados[idx]);
    printf("  [OK] Indice %d liberado.\n", idx);
    alocados[idx] = NULL;
}

/* Destroi o pool atual, zera o vetor de alocacoes e reconfigura um novo pool do zero. */
static void reiniciar(void)
{
    mymemory_cleanup(mem);
    mem = NULL;
    num_alocados = 0;
    for (int i = 0; i < MAX_ALOCACOES; i++) alocados[i] = NULL;

    printf("\nPool destruido. Reconfigurando...\n");
    mem = configurar_pool();
}

/* Ponto de entrada: inicializa o pool e executa o loop de menu ate o usuario sair. */
int main(void)
{
    mem = configurar_pool();
    if (!mem) return 1;

    int opcao;
    do {
        printf("\n╔══════════════════════════════════════╗\n");
        printf("║     GERENCIADOR DE MEMORIA           ║\n");
        printf("╠══════════════════════════════════════╣\n");
        printf("║  [1] Alocar memoria                  ║\n");
        printf("║  [2] Liberar memoria                 ║\n");
        printf("║  [3] Exibir alocacoes                ║\n");
        printf("║  [4] Exibir estatisticas             ║\n");
        printf("║  [5] Reiniciar pool                  ║\n");
        printf("║  [0] Sair                            ║\n");
        printf("╚══════════════════════════════════════╝\n");
        printf("Opcao: ");

        if (scanf("%d", &opcao) != 1) { limpar_buffer(); opcao = -1; }
        limpar_buffer();

        switch (opcao) {
        case 1: alocar();                  break;
        case 2: liberar();                 break;
        case 3: mymemory_display(mem);     break;
        case 4: mymemory_stats(mem);       break;
        case 5: reiniciar();               break;
        case 0: printf("Saindo...\n");     break;
        default: printf("Opcao invalida.\n"); break;
        }
    } while (opcao != 0);

    mymemory_cleanup(mem);
    return 0;
}
