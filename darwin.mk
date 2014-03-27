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

mac := -m10.8

flags :=
flags += -O0 -g -frtti -std=c++11 -stdlib=libc++

flags += -isystem extra
#flags += -fno-exceptions
flags += -fvisibility=hidden

flags_Hooker := -Ihde64c/include
flags_MachMessage := -Xarch_armv6 -marm

hde64c := hde64c/src/hde64.c
lsubstrate := Debug.o Hooker.o MachMemory.o MachMessage.o hde64c/src/hde64.c

framework := $(PWD)/libsubstrate.dylib

cycc = ./cycc $(mac) -o$@ -- $(flags) $(filter %.o,$^) $(filter %.dylib,$^)

all: darwin

darwin: libsubstrate.dylib SubstrateBootstrap.dylib SubstrateLauncher.dylib SubstrateLoader.dylib cynject

%.t.hpp: %.t.cpp trampoline.sh
	./trampoline.sh $@ $*.dylib $* sed otool lipo nm ./cycc $(mac) -o$*.dylib -- -dynamiclib $< -Iinclude -Xarch_armv6 -marm

MachProtect.c: MachProtect.defs MachInterface.sh
	./MachInterface.sh $@ MachProtect.h $<

MachMemory.o: MachProtect.c
DarwinInjector.o: Trampoline.t.hpp

%.o: %.cpp
	$(cycc) $(flags_$*) -c -Iinclude $<

%.o: %.mm
	$(cycc) $(flags_$*) -c -Iinclude $<

libsubstrate.dylib: DarwinFindSymbol.o DarwinInjector.o ObjectiveC.o $(lsubstrate)
	$(cycc) -dynamiclib $(hde64c) -lobjc -install_name $(framework)

SubstrateBootstrap.dylib: Bootstrap.o
	$(cycc) -dynamiclib

SubstrateLauncher.dylib: DarwinLauncher.o $(lsubstrate)
	$(cycc) -dynamiclib $(hde64c)

SubstrateLoader.dylib: DarwinLoader.o Environment.o
	$(cycc) -dynamiclib -framework CoreFoundation

cynject: cynject.o libsubstrate.dylib
	$(cycc)
	#ldid -Stask_for_pid.xml $@

%: %.o
	$(cycc) -framework CoreFoundation -framework Foundation

extrainst_ postrm: LaunchDaemons.o Cydia.o

deb: extrainst_ postrm
	./package.sh i386

package: deb

install: deb
	PATH=/Library/Cydia/bin:/usr/sbin:/usr/bin:/sbin:/bin sudo dpkg -i com.cydia.substrate_$(shell ./version.sh)_cydia.deb

upgrade: all
	sudo cp -a libsubstrate.dylib $(framework)/CydiaSubstrate
	sudo cp -a SubstrateBootstrap.dylib $(framework)/Libraries
	sudo cp -a SubstrateLauncher.dylib $(framework)/Libraries
	sudo cp -a SubstrateLoader.dylib $(framework)/Libraries

clean:
	rm -f MachProtect.h MachProtect.c *.o libsubstrate.dylib SubstrateBootstrap.dylib SubstrateLauncher.dylib SubstrateLoader.dylib extrainst_ postrm cynject

TestSuperCall: libsubstrate.dylib

test: TestSuperCall
	arch -x86_64 ./TestSuperCall

.PHONY: all clean darwin deb install test package upgrade
