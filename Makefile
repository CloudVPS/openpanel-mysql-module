include makeinclude

OBJ	= main.o mysqlcontrol.o version.o

all: module.xml mysqlmodule.exe
	mkapp mysqlmodule 

module.xml: module.def
	mkmodulexml < module.def > module.xml

version.cpp:
	mkversion version.cpp

mysqlmodule.exe: $(OBJ)
	$(LD) $(LDFLAGS) -o mysqlmodule.exe $(OBJ) $(LIBS) \
	../opencore/api/c++/lib/libcoremodule.a -lz -lssl

clean:
	rm -f *.o *.exe
	rm -rf mysqlmodule.app
	rm -f mysqlmodule

SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -I../opencore/api/c++/include -c -g $<
