#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFFER_SIZE 512
#define HUGEPAGE_SIZE (2UL * 1024 * 1024)
#define HUGEPAGE_OFFSET (1UL * 1024 * 1024)
#define PAGES (HUGEPAGE_SIZE / 4096)

int main(int argc, char **argv)
{
	long num_buffers;
	int f;
	char *src;
	char filename[512];
	struct stat statbuf;

	if (argc < 2) {
		printf("Usage: madv <filename>\n");
		exit(1);
	}

	strcpy(filename, argv[1]);

	/* Size of input file */
	if (lstat(filename, &statbuf) < 0)
	{
		perror("lstat");
		goto Exit;
	}
	num_buffers = statbuf.st_size / BUFFER_SIZE;
	printf("file_size = %lld MB, num_buffers = %ld\n", statbuf.st_size / 1024 / 1024, num_buffers);

	/* Open file */
	f = open(filename, O_RDONLY);
	if (!f) {
		printf("Error cannot open junk file\n");
		abort();
	}

	/* mmap file */
	if ((src = mmap(0, statbuf.st_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE, f, 0)) == (caddr_t)-1) {
		perror("mmap");
		goto Exit;
	}

	/* Fault every 1024 bytes */
	long long start = 24;
	long reads = 0;
	unsigned int sum = 0;

	for (long i = start; i < statbuf.st_size; i += BUFFER_SIZE) {
		((volatile char *)src)[i];
		sum += src[i];
		reads++;
	}
	printf("sum = %ld, reads = %ld\n", sum, reads);

	/* Request hugepages */
	long num_hugepages = statbuf.st_size / HUGEPAGE_SIZE;
	printf("num_hugepages = %ld\n", num_hugepages);

	start = 4096;
	for (int i = 0; i < 512; i++) {
		int error = madvise(src + start, HUGEPAGE_SIZE, MADV_HUGEPAGE);
		if (error < 0) {
			perror("madvise");
		} else {
			printf("madvise: %d, offset = %ld, error = %d\n", i, start, error);
			sum += src[start];
		}

		sleep(1);
		start += 2 * HUGEPAGE_SIZE;
	}


	/* unmap file */
	munmap(src, statbuf.st_size);
	close(f);

	/* Open file */
	f = open(filename, O_RDWR);
	if (!f) {
		printf("Error cannot open junk file\n");
		abort();
	}

	start += 8096;
	while (start >= 0) {
		int error = ftruncate(f, start);
		if (error < 0)
			perror("ftruncate");
		printf("ftruncate(%ld) = %d\n", start, error);

		start -= HUGEPAGE_OFFSET;
	}

Exit:
	close(f);
}
