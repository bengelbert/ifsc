CC	:= gcc
INCDIRS	:=	-I/usr/include/glib-2.0 \
		-I/usr/include/gnet-2.0 \
		-I/usr/lib/glib-2.0/include \
		-I/usr/lib/gnet-2.0/include
CFLAGS	:= -W -Wall $(INCDIRS)
EXEC	:= manager
SRC	:= $(wildcard *.c)
OBJ	:= $(SRC:.c=.o)
LDFLAGS	:= -lglib-2.0 -lgnet-2.0 -lgio-2.0

all: $(EXEC)
	@size $(EXEC)

$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

.PHONY: clean mrproper

clean:
	@rm -rf *.o *~

mrproper: clean
	@rm -rf $(EXEC)
