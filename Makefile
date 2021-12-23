CC = @gcc
CFLAGS = -O2 -g

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
	$(CC) $^ -o $@
	@printf "Linking complete\n"