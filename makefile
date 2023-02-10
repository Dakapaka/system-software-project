all: asembler linker emulator

asembler: parser lexer src/main.cpp src/assembler.cpp
	g++ -o $@ src/main.cpp misc/parser.tab.cpp misc/lex.yy.cpp src/assembler.cpp

linker: src/linkerMain.cpp src/linker.cpp
	g++ -o $@ src/linkerMain.cpp src/linker.cpp

emulator: src/emulatorMain.cpp src/emulator.cpp
	g++ -o $@ src/emulatorMain.cpp src/emulator.cpp
	
parser: misc/parser.ypp
	bison -o misc/parser.tab.cpp -d $<

lexer: misc/lexer.lpp
	flex -o misc/lex.yy.cpp $<

clean:
	rm -f misc/parser.tab.* misc/lex.yy.* asembler emulator linker

.PHONY: all clean parser lexer