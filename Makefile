INPUTS := $(shell find src -name '*.cc')
HEADERS := $(shell find lib -name '*.hh')

ARCH ?= x86_64

all: compiler

compiler: bin/compiler

target:
	mkdir target

lexer: target/lexer.yy.cc

parser: target/parser.yy.cc target/parser.yy.hh

syscalls: target/syscalls.hh target/syscalls/$(ARCH).hh

target/syscalls.hh target/syscalls/$(ARCH).hh: scripts/generate_syscalls.py | target
	python3 scripts/generate_syscalls.py $(ARCH)

target/lexer.yy.cc: src/lexicon/main.l | target
	flex -o target/lexer.yy.cc src/lexicon/main.l

target/parser.yy.cc target/parser.yy.hh: src/syntax/main.y | target
	bison -Wno-other -d src/syntax/main.y -o target/parser.yy.cc

bin:
	mkdir bin

bin/compiler: lexer parser syscalls $(INPUTS) $(HEADERS) | bin
	g++ -lfl -I lib -I target $(INPUTS) target/*.cc -o bin/compiler

clean:
	rm -rf bin target
