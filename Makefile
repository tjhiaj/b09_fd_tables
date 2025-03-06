CC = gcc
CFLAGS = -Wall -g -Werror -std=c99 -D_GNU_SOURCE
TARGET = showFDtables
SRC = showFDtables.c
OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

%.o: %.c showFDtables.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)