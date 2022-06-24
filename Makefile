all: compiler

compiler: bin/compiler

lexer: src/lexer.yy.cc

parser: src/parser.yy.cc lib/parser.yy.hh

bin/compiler: main.cc src/*.cc src/nodes/*.cc lib/*.hh lib/nodes/**.hh lib/parser.yy.hh src/parser.yy.cc src/lexer.yy.cc | bin
	g++ -lfl -I lib main.cc src/*.cc src/nodes/*.cc -o bin/compiler

src/lexer.yy.cc: main.l
	flex -o src/lexer.yy.cc main.l

src/parser.yy.cc lib/parser.yy.hh: main.y
	bison -d main.y -o src/parser.yy.cc && mv src/parser.yy.hh lib/

bin:
	mkdir bin

clean:
	rm -rf bin src/*.yy.cc lib/*.yy.hh
