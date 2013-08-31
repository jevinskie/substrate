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
flags += -O2 -g0

flags += -isystem extra
flags += -fno-exceptions
flags += -fvisibility=hidden
flags += -fPIC

flags_Hooker := -Ihde64c/include
flags_PosixMessage := -Xarch_armv6 -marm

hde64c := hde64c/src/hde64.c
lsubstrate := Debug.o Hooker.o PosixMemory.o hde64c/src/hde64.c

cycc = g++ -o$@ $(flags) $(filter %.o,$^) $(filter %.so,$^)

all: darwin

darwin: libsubstrate.so SubstrateBootstrap.so SubstrateLauncher.so SubstrateLoader.so cynject
ios: darwin

%.t.hpp: %.t.cpp trampoline.sh
	./trampoline.sh $@ $*.so $* sed otool lipo nm ./cycc $(ios) $(mac) -o$*.so -- -shared $< -Iinclude -Xarch_armv6 -marm

PosixProtect.c: PosixProtect.defs PosixInterface.sh
	./PosixInterface.sh $@ PosixProtect.h $<

PosixMemory.o: PosixMemory.cpp
DarwinInjector.o: Trampoline.t.hpp

%.o: %.cpp
	$(cycc) $(flags_$*) -c -Iinclude $<

%.o: %.mm
	$(cycc) $(flags_$*) -c -Iinclude $<

libsubstrate.so: $(lsubstrate)
	$(cycc) -shared $(hde64c)

SubstrateBootstrap.so: Bootstrap.o
	$(cycc) -shared

SubstrateLauncher.so: DarwinLauncher.o $(lsubstrate)
	$(cycc) -shared $(hde64c)

SubstrateLoader.so: DarwinLoader.o Environment.o
	$(cycc) -shared -framework CoreFoundation

cynject: cynject.o libsubstrate.so
	$(cycc)
	ldid -Stask_for_pid.xml $@

%: %.o
	$(cycc)

extrainst_ postrm: LaunchDaemons.o Cydia.o

deb: ios extrainst_ postrm
	./package.sh i386
	./package.sh arm

package: deb

install: deb
	PATH=/Library/Cydia/bin:/usr/sbin:/usr/bin:/sbin:/bin sudo dpkg -i com.cydia.substrate_$(shell ./version.sh)_cydia.deb

upgrade: all
	sudo cp -a libsubstrate.so $(framework)/CydiaSubstrate
	sudo cp -a SubstrateBootstrap.so $(framework)/Libraries
	sudo cp -a SubstrateLauncher.so $(framework)/Libraries
	sudo cp -a SubstrateLoader.so $(framework)/Libraries

clean:
	rm -f PosixProtect.h PosixProtect.c *.o libsubstrate.so SubstrateBootstrap.so SubstrateLauncher.so SubstrateLoader.so extrainst_ postrm cynject

TestSuperCall: libsubstrate.so

test: TestSuperCall
	arch -i386 ./TestSuperCall
	arch -x86_64 ./TestSuperCall

.PHONY: all clean darwin deb install ios test package upgrade
