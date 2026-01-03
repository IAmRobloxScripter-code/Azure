main:
	 gcc -g -Wall -Wextra azure_0.c azure_1.c azure_2.c -o azure

out:
	nasm -f elf64 out.s -o cache/out.o
	nasm -f elf64 cache/entry.s -o cache/entry.o
	nasm -f elf64 cache/helper.s -o cache/helper.o
	ld cache/out.o cache/entry.o cache/helper.o -o program