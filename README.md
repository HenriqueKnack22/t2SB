Feito por: Henrique Knack

Como rodar:

    make all

    ./test1_first_fit
    ./test1_best_fit
    ./test1_worst_fit
    ./test2_stress

    make check




O que é mymemory.h

  É o arquivo de cabeçalho (header) de um alocador de memória customizado — basicamente um malloc/free simplificado que
  usa um "pool" fixo de memória e implementa três estratégias clássicas de alocação.

  ---
  Estruturas de Dados

  strategy_t (enum)

  typedef enum { FIRST_FIT, BEST_FIT, WORST_FIT } strategy_t;
  Define qual algoritmo será usado para encontrar espaço livre no pool:
  - FIRST_FIT — usa o primeiro espaço livre suficientemente grande
  - BEST_FIT — usa o menor espaço livre que caiba (minimiza desperdício)
  - WORST_FIT — usa o maior espaço livre disponível (evita fragmentação de pequenos buracos)

  ---
  allocation_t (struct)

  Representa um único bloco alocado:
  [ start | size | *next ]
  - start — offset dentro do pool onde o bloco começa
  - size — tamanho do bloco em bytes
  - next — ponteiro para o próximo bloco (lista ligada, ordenada por endereço)

  ---
  mymemory_t (struct)

  O gerenciador do pool — a estrutura principal:
  [ *pool | total_size | *alloc_list | strategy ]
  - pool — ponteiro para o buffer de memória bruta (o "heap" simulado)
  - total_size — tamanho total do pool em bytes
  - alloc_list — cabeça da lista ligada de blocos ativos (em ordem crescente de endereço)
  - strategy — qual das três estratégias usar

  ---
  API Pública (funções declaradas)

  ┌───────────────────────────────┬─────────────────────────────────────────────────────────────────┐
  │            Função             │                            O que faz                            │
  ├───────────────────────────────┼─────────────────────────────────────────────────────────────────┤
  │ mymemory_init(size, strategy) │ Cria e retorna um pool de size bytes com a estratégia escolhida │
  ├───────────────────────────────┼─────────────────────────────────────────────────────────────────┤
  │ mymemory_alloc(mem, size)     │ Aloca size bytes no pool, retorna ponteiro para o espaço        │
  ├───────────────────────────────┼──────────────────────────────────────────────────────────────────────────┤
  │ mymemory_free(mem, ptr)       │ Libera o bloco apontado por ptr                                          │
  ├───────────────────────────────┼──────────────────────────────────────────────────────────────────────────┤
  │ mymemory_display(mem)         │ Imprime todos os blocos ativos (offset + tamanho)                        │
  ├───────────────────────────────┼──────────────────────────────────────────────────────────────────────────┤
  │ mymemory_stats(mem)           │ Mostra estatísticas: total alocado, livre, fragmentos, maior bloco livre │
  ├───────────────────────────────┼──────────────────────────────────────────────────────────────────────────┤
  │ mymemory_cleanup(mem)         │ Libera todos os blocos e o próprio pool da memória                       │
  └───────────────────────────────┴──────────────────────────────────────────────────────────────────────────┘

  ---
  Como tudo se conecta

  O fluxo de uso típico é:

  mymemory_init()  →  mymemory_alloc() × N  →  mymemory_free() × M  →  mymemory_cleanup()

  Internamente (em mymemory.c), a função find_gap() percorre a lista ligada de alocações e identifica os espaços vazios
  entre blocos (gaps). Dependendo da estratégia, ela escolhe o gap correto e o mymemory_alloc() insere o novo nó na
  posição correta da lista (ordenada por endereço).

  Importante: não há coalescência — blocos livres adjacentes não são fundidos, então a fragmentação pode crescer ao longo
  do tempo.
  O gerenciador do pool — a estrutura principal:
  [ *pool | total_size | *alloc_list | strategy ]
  - pool — ponteiro para o buffer de memória bruta (o "heap" simulado)
  - total_size — tamanho total do pool em bytes
  - alloc_list — cabeça da lista ligada de blocos ativos (em ordem crescente de endereço)
  - strategy — qual das três estratégias usar

