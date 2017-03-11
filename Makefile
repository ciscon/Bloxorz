all: sample2D

sample2D: bloxorz.cpp
	g++ -g -o bloxorz bloxorz.cpp -lGLEW -lglfw  -framework OpenGL -ldl

clean:
	rm sample2D
