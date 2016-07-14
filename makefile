#make file for viewer GUI using ROOT and FLTK
ROOTINC = -I$(shell root-config --incdir)
ROOTLIB = $(shell root-config --nonew --libs)
FLTKINC = $(shell fltk-config --cxxflags)
FLTKLIB = $(shell fltk-config --libs --ldflags)
CPPFLAGS += $(ROOTINC)
CPPFLAGS +=$(FLTKINC)
PACKAGE = AH_viewer_gui
VERSION = 0.3
DATE := $(shell date +%y%m%d-%H%:%M)
DEFS = -DPACKAGE=\"$(PACKAGE)\" -DVERSION=\"$(VERSION)\" -DDATE=\"$(DATE)\"
CPPFLAGS += $(DEFS)

guiinc:= -I/usr/include
#guilibs:= -L/usr/lib -lfltk -lfltk_images
x11libs:= -L/usr/X11R6/lib -lX11 -lXext

raVen:
	g++ -c -O3 -Wno-deprecated -std=c++0x -m32 -g \
	-I/usr/include \
	$(CPPFLAGS) $(guiinc) $(CXXFLAGS) \
	*.cc *.cxx *.cpp

	g++ -m32 *.o -o viewer \
	-L/usr/include \
	$(ROOTLIB) $(guilibs) $(x11libs) $(FLTKLIB)

64:
	g++ -c -O3 -Wno-deprecated -std=c++0x -m64 -g \
	-I/usr/include \
	$(CPPFLAGS) $(guiinc) $(CXXFLAGS) \
	*.cc *.cxx *.cpp

	g++ -m64 *.o -o viewer \
	-L/usr/include \
	$(ROOTLIB) $(guilibs) $(x11libs) $(FLTKLIB)

veto_file.o: viewer.hh

access:
	chmod a+rwx viewer

clean:
	rm -f *.o viewer
rm:
	rm *.txt *.root
