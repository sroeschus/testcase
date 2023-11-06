#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 512

int main(int argc, char **argv)
{
	long long  file_size;
	long  num_buffers;
	FILE *f;

	if (argc < 2) {
		printf("usage: create_file size\n");
		abort();
	}
	file_size = atoll(argv[1]);
	num_buffers = file_size / BUFFER_SIZE + 1;

	printf("file_size = %ld, num_buffers = %ld\n", file_size, num_buffers);
	f = fopen("junk.bin", "wb");
	if (f) {
		for (int i = 0; i < num_buffers; i++) { 
			char buffer[BUFFER_SIZE];

			for (int j = 0; j < sizeof(buffer); j++)
				buffer[j] = rand() % 256;

			fwrite(buffer, sizeof(char), sizeof(buffer), f);
		}
	}

	fclose(f);
}
