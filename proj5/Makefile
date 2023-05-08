
block.o: block.c
	gcc -Wall -Wextra -c $<

free.o: free.c
	gcc -Wall -Wextra -c $<

image.o: image.c
	gcc -Wall -Wextra -c $<

inode.o: inode.c
	gcc -Wall -Wextra -c $<

mkfs.o: mkfs.c
	gcc -Wall -Wextra -c $<

mylib.a: block.o free.o image.o inode.o mkfs.o
	ar rcs $@ $^

testlib.a: image.o free.o mkfs.o block.o inode.o
	ar rcs $@ $^

simfs_test: simfs_test.c testlib.a
	gcc -Wall -Wextra -DCTEST_ENABLE -o $@ $^

.PHONY: test

test: simfs_test
	./simfs_test


