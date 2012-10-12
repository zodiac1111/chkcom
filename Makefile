CXX = arm-linux-g++
SRC = test485.cpp uart.cpp
wflag = -Wall -Wextra 
cflag = -march=armv4t
all:$(SRC)
	$(CXX) $(SRC) $(wflag) $(cflag) 
#	mv a.out test485
