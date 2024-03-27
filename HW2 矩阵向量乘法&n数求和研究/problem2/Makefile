# 定义编译器
CC=g++

# 定义编译选项
CFLAGS=-Wall -Werror

N?=20

all: n_sum

n_sum: n_sum.cpp
	$(CC) $(CFLAGS) -DN=$(N) n_sum.cpp -o n_sum_$(N)
	./n_sum_$(N) $(N)


clean:
	find . -type f \( -name "n_sum*" \) ! -name "*.cpp" -exec rm {} \;
	
.PHONY: clean all