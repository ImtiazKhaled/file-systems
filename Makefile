all:
	gcc -std=c99 filesystems.c -o file

clean:
	rm -f $(LIBRARIES) $(TESTS)
