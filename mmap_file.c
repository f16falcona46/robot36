/*
robot36 - encode and decode images using SSTV in Robot 36 mode
Written in 2011 by <Ahmet Inan> <xdsopl@googlemail.com>
To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.
You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mmap_file.h"

struct fake_mmap_file {
	size_t write_on_close;
	size_t size;
	char* fname;
};

#define DATA_OFFSET (sizeof(size_t) + sizeof(size_t) + sizeof(char*))

int mmap_file_ro(void **p, const char *name, size_t *size)
{
	*size = 0;
	FILE *file = fopen(name, "rb");
	if (!file) return 0;
	fseek(file, 0, SEEK_END);
	long fsize = ftell(file);
	*size = fsize;
	fseek(file, 0, SEEK_SET);
	
	struct fake_mmap_file *f = malloc(sizeof(struct fake_mmap_file) + fsize);
	if (!f) {
		fclose(file);
		return 0;
	}
	*p = ((char*) f) + sizeof(struct fake_mmap_file);
	f->write_on_close = 0;
	f->size = fsize;
	f->fname = NULL;
	fread(*p, 1, fsize, file);
	fclose(file);
	return 1;
}

int mmap_file_rw(void **p, const char *name, size_t size)
{
	struct fake_mmap_file *f = calloc(sizeof(struct fake_mmap_file) + size, 1);
	if (!f) return 0;
	*p = ((char*) f) + sizeof(struct fake_mmap_file);
	f->write_on_close = 1;
	f->size = size;
	f->fname = malloc(strlen(name) + 1);
	if (!f->fname) {
		free(f);
		return 0;
	}
	strcpy(f->fname, name);
	return 1;
}
int munmap_file(void *p, size_t size)
{
	struct fake_mmap_file *f = (struct fake_mmap_file *)(((char*) p) - DATA_OFFSET);
	int success = 1;
	if (f->write_on_close) {
		FILE* file = fopen(f->fname, "wb");
		if (!file) {
			success = 0;
		}
		else {
			size_t written = fwrite(p, 1, f->size, file);
			if (written != f->size) {
				success = 0;
			}
			fclose(file);
		}
	}
	free(f->fname);
	free(f);
	return success;
}

