# Cydia Substrate - Powerful Code Insertion Platform
# Copyright (C) 2008-2011  Jay Freeman (saurik)

# GNU Lesser General Public License, Version 3 {{{
#
# Substrate is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the
# Free Software Foundation, either version 3 of the License, or (at your
# option) any later version.
#
# Substrate is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
# License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with Substrate.  If not, see <http://www.gnu.org/licenses/>.
# }}}

flags :=
flags += -O3 -g

flags += -isystem extra
flags += -fvisibility=hidden
flags += -Wall -Wextra
flags += -Wno-bitwise-op-parentheses
flags += -Wno-unused-parameter
flags += -fPIC

flags_Hooker := -Ihde64c/include
flags_PosixMessage := -Xarch_armv6 -marm

hde64c := hde64c/src/hde64.c
lsubstrate := Debug.o Hooker.o PosixMemory.o PosixFindSymbol.o hde64c/src/hde64.c

cyxx = clang++ -std=c++11 -o$@ $(flags) $(filter %.o,$^) $(filter %.so,$^)
cycc = clang -o$@ $(flags) $(filter %.o,$^) $(filter %.so,$^)


all: linux

linux: libsubstrate.so

PosixProtect.c: PosixProtect.defs PosixInterface.sh
	./PosixInterface.sh $@ PosixProtect.h $<

PosixMemory.o: PosixMemory.cpp

PosixFindSymbol.o: PosixFindSymbol.cpp

%.o: %.cpp
	$(cyxx) $(flags_$*) -c -Iinclude $<

%.o: %.mm
	$(cycc) $(flags_$*) -c -Iinclude $<

libsubstrate.so: $(lsubstrate)
	$(cycc) -lelf -shared $(hde64c)

%: %.o
	$(cycc)

clean:
	rm -f PosixProtect.h PosixProtect.c *.o libsubstrate.so

.PHONY: all clean linux
