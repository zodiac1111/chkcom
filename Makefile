CXX = arm-linux-g++
SRC = test485.cpp uart.cpp
wflag = -Wall -Wextra -g
cflag = -march=armv4t
all:$(SRC)
	$(CXX) $^ $(wflag) $(cflag)  -o chkcom
#	mv a.out test485
