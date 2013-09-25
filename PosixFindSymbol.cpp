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
#include <elf.h>

_extern MSImageRef MSGetImageByName(const char *file) {
	if (!file)
		return NULL;

	void *handle = dlopen("libdl.so", RTLD_LAZY | RTLD_NOW);
	assert(handle);
	struct link_map *map;
	int ret =  dlinfo(handle, RTLD_DI_LINKMAP, &map);
	assert(!ret);
	assert(map);

	while (map) {
		if (!strcmp(map->l_name, file))
			return (MSImageRef)map;
		map = map->l_next;
	}
	return NULL;
}

static void *MSFindSymbolSub(MSImageRef image, const char *name) {
	return 0;
}

_extern void *MSFindSymbol(MSImageRef image, const char *name) {
	if (image)
		return MSFindSymbolSub(image, name);
	void *handle = dlopen("libdl.so", RTLD_LAZY | RTLD_NOW);
	assert(handle);
	struct link_map *map;
	int ret =  dlinfo(handle, RTLD_DI_LINKMAP, &map);
	assert(!ret);
	assert(map);

	void *sym;

	while (map) {
		sym = MSFindSymbolSub((MSImageRef)map, name);
		if (sym)
			return sym;
		map = map->l_next;
	}
	return NULL;
}
