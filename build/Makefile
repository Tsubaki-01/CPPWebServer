CXX = g++
CFLAGS = -std=c++14 -O2 -Wall -g 

TARGET = server
OBJS = ../src/log/log.cpp ../src/pool/DBConnPool/*.cpp \
		 ../src/timer/*.cpp \
       ../src/http/*.cpp ../src/server/*.cpp \
       ../src/buffer/*.cpp ../src/main.cpp 

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o ../bin/$(TARGET)  -pthread -lmysqlclient

clean:
	rm -rf ../bin/$(OBJS) $(TARGET)
