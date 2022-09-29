INPUTS := $(shell find src -name '*.cc')
HEADERS := $(shell find lib -name '*.hh')

all: compiler

compiler: bin/compiler

target:
	mkdir target

lexer: target/lexer.yy.cc

parser: target/parser.yy.cc target/parser.yy.hh

target/lexer.yy.cc: src/lexicon/main.l | target
	flex -o target/lexer.yy.cc src/lexicon/main.l

target/parser.yy.cc target/parser.yy.hh: src/syntax/main.y | target
	bison -Wno-other -d src/syntax/main.y -o target/parser.yy.cc

bin:
	mkdir bin

bin/compiler: lexer parser $(INPUTS) $(HEADERS) | bin
	g++ -lfl -I lib -I target $(INPUTS) target/*.cc -o bin/compiler

clean:
	rm -rf bin target
