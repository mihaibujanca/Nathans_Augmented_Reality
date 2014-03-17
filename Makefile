CXX = g++
CXXFLAGS = -ansi -Wall -O2
LD = $(CXX) -s

INC_DIR= ARToolKit/include
LIB_DIR= ARToolKit/lib
BIN_DIR= bin
MD2_DIR= md2loader

LDFLAG=-pthread -lgstreamer-0.10 -lgobject-2.0 -lgmodule-2.0 -lxml2 -lgthread-2.0 -lrt -lglib-2.0 -L/usr/X11R6/lib -L/usr/local/lib -L$(LIB_DIR)
LIBS= -lARgsub -lARvideo -lAR -lpthread -lglut -lGLU -lGL -lGLEW -lXi -lX11 -lm -pthread -lgstreamer-0.10 -lgobject-2.0 -lgmodule-2.0 -lxml2 -lgthread-2.0 -lrt -lglib-2.0
CFLAG= -O -pthread -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/gstreamer-0.10 -I/usr/include/libxml2 -I/usr/X11R6/include -g -I$(INC_DIR)

OBJS = bullet.o object.o game_functions.o $(MD2_DIR)/Md2Model.o $(MD2_DIR)/Md2Player.o $(MD2_DIR)/Texture.o $(MD2_DIR)/Image.o

HEADDERS = object.h game_functions.h bullet.h $(MD2_DIR)/Anorms.h $(MD2_DIR)/DataManager.h $(MD2_DIR)/Image.h $(MD2_DIR)/Md2Model.h $(MD2_DIR)/Md2Player.h $(MD2_DIR)/Texture.h $(MD2_DIR)/TextureManager.h

all: $(BIN_DIR)/nathansTest 

md2loader:
	(cd $(MD2_DIR);    make -f Makefile)

$(BIN_DIR)/nathansTest: nathansTest.o $(OBJS) $(HEADDERS)
	$(CXX) -o $(BIN_DIR)/nathansTest nathansTest.o $(OBJS) $(LDFLAG) $(LIBS)

nathansTest.o: nathansTest.cpp $(HEADDERS)
	$(CXX) -c $(CFLAG) nathansTest.cpp

bullet.o: bullet.cpp $(HEADDERS)
	$(CXX) -c $(CFLAG) bullet.cpp

object.o: object.c $(HEADDERS)
	$(CXX) -c $(CFLAG) object.c

game_functions.o: game_functions.c $(HEADDERS)
	$(CXX) -c $(CFLAG) game_functions.c

#
#Md2Model.o: $(MD2_DIR)/Md2Model.cpp $(MD2_DIR)/Md2Model.h $(MD2_DIR)/Texture.h $(MD2_DIR)/TextureManager.h $(MD2_DIR)/DataManager.h $(MD2_DIR)/DataManager.inl
#Md2Player.o: $(MD2_DIR)/Md2Player.cpp $(MD2_DIR)/Md2Player.h $(MD2_DIR)/Md2Model.h $(MD2_DIR)/Texture.h
#Texture.o: $(MD2_DIR)/Texture.cpp $(MD2_DIR)/Texture.h $(MD2_DIR)/Image.h
#Image.o: $(MD2_DIR)/Image.cpp $(MD2_DIR)/Image.h

clean:
	rm -f *.o
	rm -f $(BIN_DIR)/nathansTest
	rm -f $(MD2_DIR)/*.o

allclean:
	rm -f *.o
	rm -f $(MD2_DIR)/*.o
	rm -f $(BIN_DIR)/nathansTest
	rm -f Makefile
