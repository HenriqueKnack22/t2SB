Feito por: Henrique Knack

Como rodar:

  gcc -o test1_ff  main_test1_first_fit.c  mymemory.c && ./test1_ff
  gcc -o test1_bf  main_test1_best_fit.c   mymemory.c && ./test1_bf
  gcc -o test1_wf  main_test1_worst_fit.c  mymemory.c && ./test1_wf
  gcc -o test2     main_test2_stress.c     mymemory.c && ./test2

  ./menu
  
  ├───────┼───────────────────────────────────────────┤
  │ 1     │ Aloca 30b → offset 0                      │
  ├───────┼───────────────────────────────────────────┤
  │ 2     │ Aloca 10b → offset 30                     │
  ├───────┼───────────────────────────────────────────┤
  │ 3     │ Aloca 50b → offset 40                     │
  ├───────┼───────────────────────────────────────────┤
  │ 4     │ Libera o de 30b → gap de 30b no offset 0  │
  ├───────┼───────────────────────────────────────────┤
  │ 5     │ Libera o de 10b → gap de 10b no offset 30 │
  ├───────┼───────────────────────────────────────────┤
  │ 6     │ Aloca 8b → aqui as estratégias divergem   │
  └───────┴───────────────────────────────────────────┘














  │   Conceito   │                                   O que é                                   │
  ├──────────────┼─────────────────────────────────────────────────────────────────────────────┤
  │ malloc       │ Reserva memória no heap do SO                                               │
  ├──────────────┼─────────────────────────────────────────────────────────────────────────────┤
  │ free         │ Libera memória alocada com malloc                                           │
  ├──────────────┼─────────────────────────────────────────────────────────────────────────────┤
  │ Memory Pool  │ Bloco único pré-alocado; alocações internas não chamam malloc de novo       │
  ├──────────────┼─────────────────────────────────────────────────────────────────────────────┤
  │ heap         │ Região de memória do processo gerenciada manualmente                        │
  ├──────────────┼─────────────────────────────────────────────────────────────────────────────┤
  │ offset       │ Distância em bytes do início do pool até um ponteiro                        │
  ├──────────────┼─────────────────────────────────────────────────────────────────────────────┤
  │ allocation_t │ Nó de lista encadeada representando um bloco em uso                         │
  ├──────────────┼─────────────────────────────────────────────────────────────────────────────┤
  │ Linked list  │ Estrutura onde cada nó aponta para o próximo                                │
  ├──────────────┼─────────────────────────────────────────────────────────────────────────────┤
  │ Fragmentação │ Espaços livres espalhados que individualmente não cabem uma alocação grande │
  ├──────────────┼─────────────────────────────────────────────────────────────────────────────┤
  │ First Fit    │ Usa o primeiro buraco livre que couber o pedido                             │
  ├──────────────┼─────────────────────────────────────────────────────────────────────────────┤
  │ Best Fit     │ Usa o menor buraco que couber exatamente                                    │
  ├──────────────┼─────────────────────────────────────────────────────────────────────────────┤
  │ Worst Fit    │ Usa o maior buraco disponível                                               │
  ├──────────────┼─────────────────────────────────────────────────────────────────────────────┤
  │ strategy_t   │ Enum que define qual estratégia de alocação o pool usa                      │
  ├──────────────┼─────────────────────────────────────────────────────────────────────────────┤
  │ size_t       │ Tipo inteiro sem sinal para representar tamanhos de memória                 │
  ├──────────────┼─────────────────────────────────────────────────────────────────────────────┤
  │ void*        │ Ponteiro genérico sem tipo definido                                         │
  ├──────────────┼─────────────────────────────────────────────────────────────────────────────┤
  │ Coalescência │ Junção de dois blocos livres adjacentes em um bloco maior                   │
  ├──────────────┼─────────────────────────────────────────────────────────────────────────────┤
  │ mymemory_t   │ Struct principal que representa o pool inteiro                              │
  └──────────────┴─────────────────────────────────────────────────────────────────────────────┘