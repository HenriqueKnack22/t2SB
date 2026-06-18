CC     = gcc
CFLAGS = -Wall -Wextra -g

OBJ = mymemory.o

all: test1_first_fit test1_best_fit test1_worst_fit test2_stress

mymemory.o: mymemory.c mymemory.h
	$(CC) $(CFLAGS) -c mymemory.c -o mymemory.o

test1_first_fit: main_test1_first_fit.c $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

test1_best_fit: main_test1_best_fit.c $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

test1_worst_fit: main_test1_worst_fit.c $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

test2_stress: main_test2_stress.c $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

ASAN_FLAGS = -Wall -Wextra -g -fsanitize=address,leak

check: mymemory.c mymemory.h
	$(CC) $(ASAN_FLAGS) main_test1_first_fit.c  mymemory.c -o _asan_ff  && ./_asan_ff
	$(CC) $(ASAN_FLAGS) main_test1_best_fit.c   mymemory.c -o _asan_bf  && ./_asan_bf
	$(CC) $(ASAN_FLAGS) main_test1_worst_fit.c  mymemory.c -o _asan_wf  && ./_asan_wf
	$(CC) $(ASAN_FLAGS) main_test2_stress.c     mymemory.c -o _asan_str && ./_asan_str
	@rm -f _asan_ff _asan_bf _asan_wf _asan_str
	@echo "=== SEM LEAKS — TODOS OS TESTES PASSARAM ==="

clean:
	rm -f *.o test1_first_fit test1_best_fit test1_worst_fit test2_stress _asan_ff _asan_bf _asan_wf _asan_str
