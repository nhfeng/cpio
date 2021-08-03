#include <stddef.h>

/* Magic identifiers for the "cpio" file format. */
#define CPIO_HEADER_MAGIC "070701"
#define CPIO_FOOTER_MAGIC "TRAILER!!!"
#define CPIO_ALIGNMENT 4
#define CPIO_MAX_NAME 64

struct cpio_header {
	char c_magic[6]; /* Magic header '070701'. */
	char c_ino[8]; /* "i-node" number. */
	char c_mode[8]; /* Permisions. */
	char c_uid[8]; /* User ID. */
	char c_gid[8]; /* Group ID. */
	char c_nlink[8]; /* Number of hard links. */
	char c_mtime[8]; /* Modification time. */
	char c_filesize[8]; /* File size. */
	char c_devmajor[8]; /* Major dev number. */
	char c_devminor[8]; /* Minor dev number. */
	char c_rdevmajor[8];
	char c_rdevminor[8];
	char c_namesize[8]; /* Length of filename in bytes. */
	char c_check[8]; /* Checksum. */
};

struct cpio_file {
	char name[CPIO_MAX_NAME];
	void *data;
	unsigned long size;
};

struct cpio_file cpio_find_file(const void *archive, unsigned long len,
				const char *name);
