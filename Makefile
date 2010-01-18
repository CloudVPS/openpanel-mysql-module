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
	/usr/lib/opencore/libcoremodule.a -lz -lssl

clean:
	rm -f *.o *.exe
	rm -rf mysqlmodule.app
	rm -f mysqlmodule

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -I../opencore/api/c++/include -c -g $<
