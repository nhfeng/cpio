#include <string.h>
#include <stdio.h>
#include <cpio.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include <stdlib.h>

/* Align 'n' up to the value 'align', which must be a power of two. */
static unsigned long align_up(unsigned long n, unsigned long align)
{
	return (n + align - 1) & (~(align - 1));
}

/* Parse an ASCII hex string into an integer. */
static unsigned long hex_to_str(char *s, unsigned int max_len)
{
	unsigned long r = 0;
	unsigned long i;

	for (i = 0; i < max_len; i++) {
		r *= 16;
		if (s[i] >= '0' && s[i] <= '9') {
			r += s[i] - '0';
		} else if (s[i] >= 'a' && s[i] <= 'f') {
			r += s[i] - 'a' + 10;
		} else if (s[i] >= 'A' && s[i] <= 'F') {
			r += s[i] - 'A' + 10;
		} else {
			return r;
		}
		continue;
	}
	return r;
}

static int cpio_loop_file(struct cpio_header *header, int len,
			  struct cpio_file *cfile, unsigned long *next)
{
	char *file_name;
	int name_len;

	if (!header || !next || !cfile || !cfile->name) {
		printf("error input param\n");
		goto error_out;
	}
	unsigned long filesize;
	/* ensure magic header exists. */
	if (strncmp(header->c_magic, CPIO_HEADER_MAGIC,
		    sizeof(header->c_magic)) != 0) {
		printf("%c%c%c%c%c%c\n", header->c_magic[0], header->c_magic[1], header->c_magic[2], header->c_magic[3], header->c_magic[4], header->c_magic[5]);
		printf("header magic error\n");
		goto error_out;
	}

	/* file size. */
	cfile->size =
		hex_to_str(header->c_filesize, sizeof(header->c_filesize));
	file_name = ((char *)header) + sizeof(struct cpio_header);

	/* Ensure filename is not the trailer indicating EOF. */
	if (!strncmp(file_name, CPIO_FOOTER_MAGIC, sizeof(CPIO_FOOTER_MAGIC)))
		goto error_out;

	/* Find offset to data. */
	name_len = hex_to_str(header->c_namesize, sizeof(header->c_namesize));

	memcpy(cfile->name, file_name, name_len);
	printf("name %s\n", cfile->name);

	cfile->data = (void *)align_up(
		((unsigned long)header) + sizeof(struct cpio_header) + name_len,
		CPIO_ALIGNMENT);
	*next = align_up(((unsigned long)cfile->data) + cfile->size,
			 CPIO_ALIGNMENT);

	return 0;

error_out:
	return -1;
}

struct cpio_file cpio_find_file(const void *archive, unsigned long len,
				const char *name)
{
	int ret;
	uintptr_t next;

	unsigned long start = (unsigned long)archive;
	unsigned long end = (unsigned long)archive + len;
	struct cpio_file cfile = { 0 };

	/* Find n'th entry. */
	while (start < end && start + sizeof(struct cpio_header) < end) {
		ret = cpio_loop_file(start, len, &cfile, &next);
		/* no this file */
		if (ret) {
			cfile.data = NULL;
			return cfile;
		}

		if (!strncmp(cfile.name, name, CPIO_MAX_NAME))
			return cfile;

		start = (struct cpio_header *)next;
	}

	return cfile;
}

int main()
{
	struct cpio_file cfile;
	int fd;
	char file_data[4096];
	system("echo hello > hello.txt");
	system("echo world > world.txt");
	system("echo test > test.txt");
	system("echo zzzzzzzzzzzzzzzzz > zzz.txt");
	system("find ./ -name '*.txt' |cpio -ov --format=newc > ./test.cpio");
	system("cpio -t -I ./test.cpio");

	fd=open("./test.cpio",O_RDWR|O_APPEND);
	read(fd, file_data, sizeof(file_data));

	cfile = cpio_find_file(file_data, sizeof(file_data), "zzz.txt");
	if (cfile.data) {
		printf("%s\n", cfile.data);
	}
	return 0;
}
