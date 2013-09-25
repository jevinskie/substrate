#define _GNU_SOURCE
#include <dlfcn.h>
#include <link.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <gelf.h>
#include <sys/stat.h>

void dump(void *elf, size_t sz) {
	Elf *e = elf_memory((char *)elf, sz);
	return;
}

int main(void) {
	void *handle = dlopen("libdl.so", RTLD_LAZY | RTLD_NOW);
	assert(handle);
	struct link_map *map;
	int ret =  dlinfo(handle, RTLD_DI_LINKMAP, &map);
	assert(!ret);
	assert(map);

	while (map) {
		printf("addr: 0x%lx\n", map->l_addr);
		if (strlen(map->l_name) > 0)
			printf("*addr: 0x%lx\n", *(uint64_t *)map->l_addr);
		printf("name: %s\n", map->l_name ? map->l_name : "(null)");
		printf("ld: %p\n", map->l_ld);
		struct stat st;
		stat(map->l_name, &st);
		dump((void *)map->l_addr, st.st_size);
		map = map->l_next;
	}
	return 0;
}