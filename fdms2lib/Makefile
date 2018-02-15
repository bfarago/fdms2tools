#
# FDMS2TOOLS
#
LIBS=libfdms2.so
OBJS=fdms2.o fdms2dispdata.o fdms2lib.o fdms2part.o fdms2pos.o

all:$(LIBS) fdms2lib.exe

clean:
	rm -f *.so *.exe *.backup *.o look .depend l_*.c $(LIBS)

%.so : %.o
	$(CC) -shared -Xlinker -x -o $@ $<
        
%.o: %.cpp
	$(CC) -c -o$@ $<

$(LIBS): $(OBJS)
	$(CC) -shared -Xlinker -x -o $@ $(OBJS) -lgcc -lstdc++

fdms2lib.exe: fdms2lib.cs
	mcs fdms2lib.cs
