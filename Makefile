all:
	gcc filesystems.c -o file

clean:
	rm -f $(LIBRARIES) $(TESTS)
