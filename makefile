#make file for viewer GUI using ROOT and FLTK
ROOTINC = -I$(shell root-config --incdir)
ROOTLIB = $(shell root-config --nonew --libs)
FLTKINC = $(shell fltk-config --cxxflags)
FLTKLIB = $(shell fltk-config --libs --ldflags)
CPPFLAGS += $(ROOTINC)
CPPFLAGS +=$(FLTKINC)
PACKAGE = christuart_trit_diff
VERSION = 0.1
DATE := $(shell date +%y%m%d-%H%:%M)
DEFS = -DPACKAGE=\"$(PACKAGE)\" -DVERSION=\"$(VERSION)\" -DDATE=\"$(DATE)\"
CPPFLAGS += $(DEFS)

guiinc:= -I/usr/include
#guilibs:= -L/usr/lib -lfltk -lfltk_images
x11libs:= -L/usr/X11R6/lib -lX11 -lXext

default:
	g++ -c -O3 -Wno-deprecated -std=c++0x -m32 -g \
	-I/usr/include \
	$(CPPFLAGS) $(guiinc) $(CXXFLAGS) \
	*.cc *.cxx *.cpp

	g++ -m32 *.o -o trit-dif \
	-L/usr/include \
	$(ROOTLIB) $(guilibs) $(x11libs) $(FLTKLIB)

64:
	g++ -c -O3 -Wno-deprecated -std=c++0x -m64 -g \
	-I/usr/include \
	$(CPPFLAGS) $(guiinc) $(CXXFLAGS) \
	*.cc *.cxx *.cpp

	g++ -m64 *.o -o trit-dif \
	-L/usr/include \
	$(ROOTLIB) $(guilibs) $(x11libs) $(FLTKLIB)

veto_file.o: viewer.hh

access:
	chmod a+rwx trit-dif

clean:
	rm -f *.o trit-dif
rm:
	rm *.txt *.root

docs:
	~/.local/bin/cldoc generate -O3 -Wno-deprecated -std=c++0x -m32 -g $(CPPFLAGS) $(guiinc) -- --output docstest/ *.hh

