
all: minesweeper

minesweeper: minesweeper.cpp
	g++ -o minesweeper minesweeper.cpp -lGL -lGLU -lglut -lGLEW

clean:
	rm minesweeper

