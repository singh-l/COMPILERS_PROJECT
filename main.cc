#include <iostream>
#include <fstream>
#include <list>
#include <cstring>

#include "Scanner.h"
#include "Parser.h"
using namespace std;
int main (int argc, char** argv)
{
	char* position = NULL;
	for (int i = 1; i < argc; i++) { 
        if (i + 1 != argc && strcmp(argv[i] ,"-f") == 0){
        	position = argv[i+1];
        	break;
        }
    }

  Parser parser;

  if(position == NULL){
  	parser.position_selected = -1;
  }else{
  	parser.position_selected = atoi(position);
  }
  
  
  parser.parse();
}


