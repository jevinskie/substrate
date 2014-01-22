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
#include <string>
#include <map>
#include <iostream>
#include <boost/format.hpp>
#include <cstdint>
#include <limits>
#include <boost/algorithm/string/predicate.hpp>

extern "C" int fact(int n);
extern "C" int fact_static(int n);

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

std::map<int, std::string> sym_types {
	{0, "STT_NOTYPE"},
	{1, "STT_OBJECT"},
	{2, "STT_FUNC"},
	{3, "STT_SECTION"},
	{4, "STT_FILE"},
	{5, "STT_COMMON"},
	{10, "STT_LOOS"},
	{12, "STT_HIOS"},
	{13, "STT_LOPROC"},
	{15, "STT_HIPROC"}
};

std::string sym_type(int type) {
	try {
		return sym_types.at(type);
	} catch (...) {
		return "STT_" + std::to_string(type);
	}
}

void dump(std::string path, std::uint64_t base) {
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

	assert(elf = (char *)mmap_file(path, &sz));
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

	std::cout << "symtab " << symtab_idx << " strtab " << strtab_idx << " shstrtab " << \
		shstrtab_idx << " dynstr " << dynstr_idx << " dynsym " << dynsym_idx << "\n";

	if (symtab_idx != 0) {
		assert(scn = elf_getscn(e, symtab_idx));
		assert(gelf_getshdr(scn, &shdr));
		assert(data = elf_getdata(scn, NULL));
		for (unsigned int i = 0; i < shdr.sh_size / shdr.sh_entsize; i++) {
			assert(gelf_getsym(data, i, &sym));
			std::string sym_name {elf_strptr(e, strtab_idx, sym.st_name)};
			if (!(boost::contains(sym_name, "fact") || boost::contains(sym_name, "number")))
				continue;
			std::cout << "symtab st_name: " << sym_name << "\n";
			std::cout << boost::format("\tst_value: 0x%016x\n") % sym.st_value;
			std::cout << boost::format("\taddr: 0x%016x\n") % (sym.st_value + base);
			std::cout << "\tst_info (type): " << sym_type(GELF_ST_TYPE(sym.st_info)) << "\n";
		}
	}

	if (dynsym_idx != 0) {
		assert(scn = elf_getscn(e, dynsym_idx));
		assert(gelf_getshdr(scn, &shdr));
		assert(data = elf_getdata(scn, NULL));
		for (unsigned int i = 0; i < shdr.sh_size / shdr.sh_entsize; i++) {
			assert(gelf_getsym(data, i, &sym));
			std::string sym_name {elf_strptr(e, dynstr_idx, sym.st_name)};
			if (!(boost::contains(sym_name, "fact") || boost::contains(sym_name, "number")))
				continue;
			std::cout << "dynsym st_name: " << sym_name << "\n";
			std::cout << boost::format("\tst_value: 0x%016x\n") % sym.st_value;
			std::cout << boost::format("\taddr: 0x%016x\n") % (sym.st_value + base);
			std::cout << "\tst_info (type): " << sym_type(GELF_ST_TYPE(sym.st_info)) << "\n";
		}
	}

	munmap((void *)elf, sz);

	return;
}

int main(void) {
	std::cout << boost::format("fact = %1%\n") % (void *)fact;
	std::cout << "fact(5) = " << fact(5) << "\n";
	std::cout << boost::format("fact_static = %1%\n") % (void *)fact_static;
	std::cout << "fact_static(5) = " << fact_static(5) << "\n";

	void *handle = dlopen("libdl.so", RTLD_LAZY | RTLD_NOW);
	assert(handle);
	struct link_map *map;
	int ret = dlinfo(handle, RTLD_DI_LINKMAP, &map);
	dlclose(handle);
	assert(!ret);
	assert(map);

	// back up to find the first real entry
	while (map->l_prev && strlen(map->l_prev->l_name) > 0)
		map = map->l_prev;

	while (map) {
		std::string name {map->l_name};
		std::cout << "name: " << name << "\n";
		if (!boost::contains(name, "libtest-dl-lib")) {
			map = map->l_next;
			continue;
		}
		dump(map->l_name, map->l_addr);
		map = map->l_next;
	}

	return 0;
}
