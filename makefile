all:    lex.cc parse.cc codegen.cc main.cc Scanner.h Scannerbase.h Scanner.ih Parser.h Parserbase.h Parser.ih

	@sed -i '/#include <iostream>/a #include "class_structure.h"' Parserbase.h;
	@sed -i '/#include <iostream>/a #include "symbol_table.h"' Parserbase.h;
	@sed -i '/#include <iostream>/a #include <string>' Parserbase.h;
	@sed -i '/int parse();/a int position_selected;' Parser.h;
	@./sedscript
	@echo "...Class Structure and Symbol Classes Added"

	@g++  -g --std=c++11  lex.cc codegen.cc class_structure.cpp symbol_table.cpp parse.cc  main.cc;
	@echo "...Compiled"

	@./a.out < testfile.cpp > working.out;
	# @./a.out < error_testcase.cpp > not_working.out;
	@echo "...Finished"	
        
run:
	@./a.out < testfile.cpp > working.out;
	@echo "...Finished"	

lex.cc: clean lex.l Scanner.ih 
	@flexc++ lex.l; 
	@sed -i '/#include "Scanner.ih"/a int newLno = 0;' lex.cc
	@#sed -i '/#include <memory>/a int newLno;' Scannerbase.h
	@echo "...Scanner Script Generated"

parse.cc:  parse.y symbol_table.h Parser.ih Parser.h Parserbase.h
	@bisonc++ parse.y; 
	@sed -i '/#include "Parser.ih"/r function_source.cc' parse.cc
	@echo "...Parse Script Generated"

graph: 
	@echo "}" >> graph_g.gv
	@dot -Tps graph_g.gv -o graph.ps

machine: machine.cc code.asm
	g++ -g3 -m32 -O0 machine.cc -o machine.out
	./machine.out

clean:
	@rm -f Parserbase.h
	@rm -f Scannerbase.h
	@rm -f Scanner.h
	@rm -f Parser.h
	@rm -f Parser.ih
	@rm -f Scanner.ih
	@rm -f parse.cc
	@rm -f lex.cc
	@rm -f junk
	@rm -f graph_g.gv
	@rm -f graph.ps
	@rm -f debug
	@rm -f parse.y.output
	@rm -f lab_parser
	@rm -f output.scm
	@rm -f a.out
	@rm -f working.out
	@rm -f not_working.out
	@rm -f code.asm
	@rm -f machine.out
	@echo "...Directory Cleaned"

Parser.ih: parse.y
Parser.h:  parse.y
Parserbase.h: parse.y
Scanner.ih: lex.l
Scanner.h: lex.l
Scannerbase.h: lex.l
codegen.cc: codegen.h