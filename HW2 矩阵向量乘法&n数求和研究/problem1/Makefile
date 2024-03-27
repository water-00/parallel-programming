# 定义编译器
CC=g++

# 定义编译选项
CFLAGS=-Wall -Werror

N?=5000

all: matrix matrix_unroll matrix_template

matrix: matrix.cpp
	$(CC) $(CFLAGS) -DN=$(N) matrix.cpp -o matrix_$(N)
	./matrix_$(N) $(N)

matrix_unroll: matrix_unroll.cpp
	$(CC) $(CFLAGS) -DN=$(N) matrix_unroll.cpp -o matrix_unroll_$(N)
	./matrix_unroll_$(N) $(N)

matrix_template: matrix_template.cpp
	$(CC) $(CFLAGS) -DN=$(N) -ftemplate-depth=1500 matrix_template.cpp -o matrix_template_$(N)
	./matrix_template_$(N) $(N)

clean:
	find . -type f \( -name "matrix*" -o -name "matrix_unroll*" -o -name "matrix_template*" \) ! -name "*.cpp" -exec rm {} \;
	
.PHONY: clean all