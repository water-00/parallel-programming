# 指定编译器
CC=g++

# 编译选项
CFLAGS=-mavx -msse4.1 -std=c++17 -g

# 源文件
SRC=main.cpp

# 目标执行文件
TARGET=main

# 默认目标
all: $(TARGET) run convert clean

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run:
	./$(TARGET) $(UL)

convert:
	convert out.ppm out.png

clean:
	rm -f out.ppm $(TARGET)

.PHONY: all run convert clean debug