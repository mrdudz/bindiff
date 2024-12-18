
.SILENT:

all:
	$(CC) -O2 -Wall bindiff.c -o bindiff

install:
	cp ./bindiff ~/bin

clean:
	$(RM) *.o bindiff
