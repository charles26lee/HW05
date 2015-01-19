#--Source code--
OBJ = BouncingBall.cpp

#--Compiler used--
CC = g++

#--Libraries we're linking against.--
LIBRARY_LINKS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_image

#--Name of our exectuable--
OBJ_NAME = BouncingBall

#--This is the target that compiles our executable--
all : $(OBJS)  
	$(CC) $(C++11) $(OBJ) $(LIBRARY_LINKS) -o $(OBJ_NAME)