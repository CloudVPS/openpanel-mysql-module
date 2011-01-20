# This file is part of OpenPanel - The Open Source Control Panel
# OpenPanel is free software: you can redistribute it and/or modify it 
# under the terms of the GNU General Public License as published by the Free 
# Software Foundation, using version 3 of the License.
#
# Please note that use of the OpenPanel trademark may be subject to additional 
# restrictions. For more information, please visit the Legal Information 
# section of the OpenPanel website on http://www.openpanel.com/

include makeinclude

OBJ	= main.o mysqlcontrol.o version.o

all: module.xml mysqlmodule.exe down_mysqldb.png
	grace mkapp mysqlmodule 

down_mysqldb.png: mysqldb.png
	convert -modulate 50,100,100 mysqldb.png down_mysqldb.png

module.xml: module.def
	mkmodulexml < module.def > module.xml

version.cpp:
	grace mkversion version.cpp

mysqlmodule.exe: $(OBJ)
	$(LD) $(LDFLAGS) -o mysqlmodule.exe $(OBJ) $(LIBS) \
	/usr/lib/openpanel-core/libcoremodule.a -lz

install:
	mkdir -p ${DESTDIR}/var/openpanel/modules/MySQL.module
	mkdir -p ${DESTDIR}/var/openpanel/conf/staging/MySQL
	cp -rf ./mysqlmodule.app    ${DESTDIR}/var/openpanel/modules/MySQL.module/
	ln -sf mysqlmodule.app/exec ${DESTDIR}/var/openpanel/modules/MySQL.module/action
	cp     module.xml          ${DESTDIR}/var/openpanel/modules/MySQL.module/module.xml
	install -m 755 verify      ${DESTDIR}/var/openpanel/modules/MySQL.module/verify
	cp *.html ${DESTDIR}/var/openpanel/modules/MySQL.module

clean:
	rm -f *.o *.exe
	rm -rf mysqlmodule.app
	rm -f mysqlmodule

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -I../opencore/api/c++/include -c -g $<
