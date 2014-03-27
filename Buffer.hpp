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

#ifndef SUBSTRATE_BUFFER_HPP
#define SUBSTRATE_BUFFER_HPP

#include <string.h>
#include "Debug.hpp"
#include <typeinfo>
#include <string>
#include <boost/format.hpp>
#include <cxxabi.h>

template <typename Type_>
_disused static _finline void MSWrite(uint8_t *&buffer, Type_ value) {
    *reinterpret_cast<Type_ *>(buffer) = value;
    uint8_t *old_buf = buffer;
    buffer += sizeof(Type_);
    if (MSDebug) {
    	char name_buf[4096];
    	int status;
    	const char *mangled = typeid(Type_).name();
    	size_t size = sizeof(name_buf);
    	char *demangled = __cxxabiv1::__cxa_demangle(mangled, name_buf, &size, &status);
    	boost::format f("MSWrite<%1%> 0x%2$08x");
    	f % demangled % (uint64_t)old_buf;
    	MSLogHex(old_buf, sizeof(Type_), f.str().c_str());
    }
}
_disused static _finline void MSWrite(uint8_t *&buffer, uint8_t *data, size_t size) {
    memcpy(buffer, data, size);
    uint8_t *old_buf = buffer;
    buffer += size;
    if (MSDebug) {
    	boost::format f("MSWrite<%1%> 0x%2$08x");
    	f % size % (uint64_t)old_buf;
    	MSLogHex(old_buf, size, f.str().c_str());
    }
}

#endif//SUBSTRATE_BUFFER_HPP
