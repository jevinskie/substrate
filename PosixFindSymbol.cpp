/* Cydia Substrate - Powerful Code Insertion Platform
 * Copyright (C) 2008-2011  Jay Freeman (saurik)
*/

/* GNU Lesser General Public License, Version 3 {{{ */
/*
 * Substrate is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * Substrate is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Substrate.  If not, see <http://www.gnu.org/licenses/>.
**/
/* }}} */

#include "CydiaSubstrate.h"

#include <dlfcn.h>
#include <link.h>
#include <assert.h>
#include <string.h>
#include <gelf.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string>
#include <set>
#include <map>
#include <iterator>
#include <vector>
#include <boost/algorithm/string/predicate.hpp>

void *mmap_file(std::string path, size_t *size) {
	int fd;
    struct stat st;
    void *ptr;
	
	fd = open(path.c_str(), O_RDONLY);
    assert(fd >= 0);
    assert(fstat(fd, &st) == 0);
    ptr = mmap(0, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    assert(ptr);
    close(fd);
    if (size)
    	*size = st.st_size;
	return ptr;
}

_extern MSImageRef MSGetImageByName(const char *file) {
	if (!file)
		return NULL;

	void *handle = dlopen("libdl.so", RTLD_LAZY | RTLD_NOW);
	assert(handle);
	struct link_map *map;
	int ret =  dlinfo(handle, RTLD_DI_LINKMAP, &map);
	dlclose(handle);
	assert(!ret);
	assert(map);

	// back up to find the first real entry
	while (map->l_prev && strlen(map->l_prev->l_name) > 0)
		map = map->l_prev;

	while (map) {
		//if (!strcmp(map->l_name, file))
		//	return (MSImageRef)map;
		if (boost::algorithm::ends_with(map->l_name, file))
			return (MSImageRef)map;
		map = map->l_next;
	}
	return NULL;
}

static void MSFindSymbolSub(MSImageRef image, std::set<std::string> &worklist, std::map<std::string, void *> &resolved_symbols) {
	Elf *e;
	Elf_Scn *scn;
	size_t shstrtab_idx;
	size_t strtab_idx = 0;
	size_t symtab_idx = 0;
	size_t dynstr_idx = 0;
	size_t dynsym_idx = 0;
	GElf_Shdr shdr;
	size_t sz;
	size_t i;
	char *elf;
	Elf_Data *data;
	GElf_Sym sym;
	struct link_map *map = (struct link_map *)image;

	if (worklist.empty())
		return;

	assert(elf = (char *)mmap_file(map->l_name, &sz));
	assert(elf_version(EV_CURRENT) != EV_NONE);
	assert(e = elf_memory(elf, sz));
	assert(!elf_getshdrstrndx(e, &shstrtab_idx));

	i = 1;
	scn = NULL;
	while ((scn = elf_nextscn(e, scn)) != NULL) {
		assert(gelf_getshdr(scn, &shdr));
		std::string name {elf_strptr(e, shstrtab_idx, shdr.sh_name)};
		if (name == ".symtab")
			symtab_idx = i;
		else if (name == ".strtab")
			strtab_idx = i;
		else if (name == ".dynstr")
			dynstr_idx = i;
		else if (name == ".dynsym")
			dynsym_idx = i;
		i++;
	}

	if (symtab_idx != 0) {
		assert(scn = elf_getscn(e, symtab_idx));
		assert(gelf_getshdr(scn, &shdr));
		assert(data = elf_getdata(scn, NULL));
		for (unsigned int i = 0; i < shdr.sh_size / shdr.sh_entsize; i++) {
			assert(gelf_getsym(data, i, &sym));
			std::string sym_name {elf_strptr(e, strtab_idx, sym.st_name)};
			if (worklist.find(sym_name) != worklist.end()) {
				resolved_symbols[sym_name] = (void *)(map->l_addr + sym.st_value);
				worklist.erase(sym_name);
				if (worklist.empty())
					goto done;
			}
		}
	}

	if (dynsym_idx != 0) {
		assert(scn = elf_getscn(e, dynsym_idx));
		assert(gelf_getshdr(scn, &shdr));
		assert(data = elf_getdata(scn, NULL));
		for (unsigned int i = 0; i < shdr.sh_size / shdr.sh_entsize; i++) {
			assert(gelf_getsym(data, i, &sym));
			std::string sym_name {elf_strptr(e, dynstr_idx, sym.st_name)};
			if (worklist.find(sym_name) != worklist.end()) {
				resolved_symbols[sym_name] = (void *)(map->l_addr + sym.st_value);
				worklist.erase(sym_name);
				if (worklist.empty())
					goto done;
			}
		}
	}

done:
	munmap((void *)elf, sz);
}

static void MSFindSymbols(MSImageRef image, size_t count, const char *names[], void *values[]) {
	std::set<std::string> worklist {names, names + count};
	std::map<std::string, void *> resolved_symbols;

	if (image) {
		MSFindSymbolSub(image, worklist, resolved_symbols);
	} else {
		void *handle = dlopen("libdl.so", RTLD_LAZY | RTLD_NOW);
		assert(handle);
		struct link_map *map;
		int ret =  dlinfo(handle, RTLD_DI_LINKMAP, &map);
		dlclose(handle);
		assert(!ret);
		assert(map);

		// back up to find the first real entry
		while (map->l_prev && strlen(map->l_prev->l_name) > 0)
			map = map->l_prev;

		while (map && !worklist.empty()) {
			MSFindSymbolSub((MSImageRef)map, worklist, resolved_symbols);
			map = map->l_next;
		}
	}

	for (size_t i = 0; i < count; i++) {
		try {
			void *res = resolved_symbols.at(names[i]);
			values[i] = res;
		} catch (std::out_of_range) {
			values[i] = NULL;
		}
	}

}

_extern void *MSFindSymbol(MSImageRef image, const char *name) {
    void *value;
    MSFindSymbols(image, 1, &name, &value);
    return value;
}