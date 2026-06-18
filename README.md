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