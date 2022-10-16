INPUTS := $(shell find src -name '*.cc')
HEADERS := $(shell find lib -name '*.hh')

ARCH := $(shell uname -m)

all: compiler

compiler: bin/compiler

target:
	mkdir target

lexer: target/lexer.yy.cc

parser: target/parser.yy.cc target/parser.yy.hh

syscalls: target/syscalls.hh target/syscalls/$(ARCH).hh

target/syscalls.hh target/syscalls/$(ARCH).hh: scripts/generate_syscalls.py scripts/syscalls/$(ARCH).csv | target
	python3 scripts/generate_syscalls.py $(ARCH)

target/lexer.yy.cc: src/lexicon/main.l | target
	flex -o target/lexer.yy.cc src/lexicon/main.l

target/parser.yy.cc target/parser.yy.hh: src/syntax/main.y scripts/extract_token_types.py | target
	bison -Wno-other -d src/syntax/main.y -o target/parser.yy.cc && \
	python3 scripts/extract_token_types.py

bin:
	mkdir bin

bin/compiler: lexer parser syscalls $(INPUTS) $(HEADERS) | bin
	g++ -s -lfl -I lib -I target --std=c++17 $(INPUTS) target/*.cc -o bin/compiler

clean:
	rm -rf bin target
