#include <stdio.h>

void bin(unsigned n);

int main(int argc, char *argv[])
{
	 (void)argc; (void)argv;

	// a) 0...01
	int mask1= 1 << 0;
	printf("a) mask1: ");
	bin(mask1);
	printf("\n");

	// b) 0...10
	int mask2= 1 << 1;
	printf("b) mask2: ");
	bin(mask2);
	printf("\n");

	// c) 0...10101
	int mask3= 1 << 0; //0...00001
	mask3|= 1 << 2;		 //0...00100
	mask3|= 1 << 4;		 //0...10000
	printf("c) mask3: ");
	bin(mask3);
	printf("\n");

	return 0;
}

void bin(unsigned n)
{
    unsigned i;
    for (i = 1 << 31; i > 0; i = i / 2)
        (n & i)? printf("1"): printf("0");
}
