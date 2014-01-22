#define _GNU_SOURCE
#include <dlfcn.h>
#include <link.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <gelf.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

extern int fact(int n);
extern int fact_static(int n);

#define BRK  __asm__ volatile("int $0x03")

void *mmap_file(const char *path, size_t *size) {
	int fd;
    struct stat st;
    void *ptr;
	
	fd = open(path, O_RDONLY);
    assert(fd >= 0);
    assert(fstat(fd, &st) == 0);
    ptr = mmap(0, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    assert(ptr);
    close(fd);
    if (size)
    	*size = st.st_size;
	return ptr;
}


void memcpy_to_file(const char *path, void *ptr, size_t size)
{
    FILE *fp = fopen(path, "wb");
    fwrite(ptr, size, 1, fp);
    fclose(fp);
}

void dump(const char *path) {
	Elf *e;
	Elf_Scn *scn = NULL;
	size_t shstrndx;
	GElf_Shdr shdr;
	char *name;
	size_t sz;
	void *elf;
	Elf_Data *data;
	GElf_Sym *sym;

	assert(elf = mmap_file(path, &sz));
	assert(elf_version(EV_CURRENT) != EV_NONE);
	assert(e = elf_memory(elf, sz));
	assert(!elf_getshdrstrndx(e, &shstrndx));

	while ((scn = elf_nextscn(e, scn)) != NULL) {
		assert(gelf_getshdr(scn, &shdr));
		assert(name = elf_strptr(e, shstrndx, shdr.sh_name));
		if (!strcmp(name, ".symtab"))
			continue;
		data = NULL;
		while ((data = elf_getdata(scn, data)) != NULL) {

		} 
	}
	return;
}

#if 1

int main(void) {
	printf("fact = %p\n", fact);
	printf("fact(5) = %d\n", fact(5));
	printf("fact_static = %p\n", fact_static);
	printf("fact_static(5) = %d\n", fact_static(5));

	void *handle = dlopen("libdl.so", RTLD_LAZY | RTLD_NOW);
	assert(handle);
	struct link_map *map;
	int ret =  dlinfo(handle, RTLD_DI_LINKMAP, &map);
	assert(!ret);
	assert(map);

	// back up to find the first real entry
	while (map->l_prev && strlen(map->l_prev->l_name) > 0)
		map = map->l_prev;

	while (map) {
		printf("addr: 0x%lx\n", map->l_addr);
		if (strlen(map->l_name) > 0)
			printf("*addr: 0x%lx\n", *(uint64_t *)map->l_addr);
		printf("name: %s\n", map->l_name ? map->l_name : "(null)");
		printf("ld: %p\n", map->l_ld);
		dump(map->l_name);
		map = map->l_next;
	}

	return 0;
}

#else

int main(void) {
	size_t size;
	void *elf = mmap_file("/home/jevin/code/substrate/libtest-dl-lib.so", &size);
	printf("size %zu\n", size);
	dump(elf, size);
	return 0;
}

#endif
