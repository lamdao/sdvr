CXX=x86_64-w64-mingw32-g++
RES=x86_64-w64-mingw32-windres
CSTD=-std=c++11 
OPTS=-O3 -s -msse2 -march=native -mtune=native -ffast-math
SDIR=src
WIN32PP=-IWin32++/include
CXXFLAGS=-I$(SDIR) $(WIN32PP) $(OPTS) $(CSTD)

SRCS=$(SDIR)/box.cpp $(SDIR)/main.cpp $(SDIR)/Picture.cpp $(SDIR)/View.cpp $(SDIR)/VolumeRender.cpp
OBJS=box.o main.o Picture.o View.o VolumeRender.o Resource.o
LIBS=-lkernel32 -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32 -lshell32 -luuid -lcomctl32 -static -lgcc
MSYS=-mwindows

TARGET=sdvr.exe

%.o: $(SDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $<

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(MSYS) -s -o $@ $(OBJS) $(LIBS)

clean:
	rm -f *.o

distclean: clean
	rm -f $(TARGET)

Resource.o: $(SDIR)/Resource.rc
	$(RES) $(WIN32PP) -I$(SDIR) -o $@ $<

box.o: $(SDIR)/box.cpp
main.o: $(SDIR)/main.cpp
Picture.o: $(SDIR)/Picture.cpp
View.o: $(SDIR)/View.cpp $(SDIR)/core.cpp $(SDIR)/volume.h
VolumeRender.o: $(SDIR)/VolumeRender.cpp
