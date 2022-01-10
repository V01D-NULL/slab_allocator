CC = @gcc
CFLAGS = -O2 -g -fsanitize=undefined

SRC = $(shell find . -type f -name '*.c')
OBJ = $(SRC:.c=.c.o)
FINAL = test_slab

all: $(FINAL)
	@printf "Built: $<\n\n"
	./$<

%.c.o: %.c
	@printf "CC $<\n"
	$(CC) -c $(CFLAGS) $< -o $@

clean: $(OBJ)
	rm $^
	

$(FINAL): $(OBJ)
	@printf "Linking...\n"
	$(CC) $^ -lubsan -o $@
	@printf "Linking complete\n"
