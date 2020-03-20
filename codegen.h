#ifndef CODEGEN_H
#define CODEGEN_H
#include <list>
#include <map>
#include <vector>
#include <string>
#include "symbol_table.h"

using namespace std;
enum machine_function{
	MF_ADD_I,
	MF_ADD_F,
	MF_MULT_I,
	MF_MULT_F,
	MF_DIV_I,
	MF_DIV_F,
	MF_COMP_I,
	MF_COMP_F,
	MF_J,
	MF_JE,
	MF_JNE,
	MF_JL,
	MF_JLE,
	MF_JG,
	MF_JGE,
	MF_MOVE,
	MF_INT_TO_FLOAT,
	MF_FLOAT_TO_INT,
	MF_STORE_I,
	MF_STORE_F,
	MF_LOAD_I,
	MF_LOAD_F,
	MF_PUSH_I,
	MF_PUSH_F,
	MF_POP_I,
	MF_POP_F,
	MF_PRINT_INT,
	MF_PRINT_FLOAT,
	MF_PRINT_STRING
};

enum boolean_mode{
	MODE_IF_GOTO_ELSE_FALL,
	MODE_IFNOT_GOTO_ELSE_FALL,
	MODE_IF_GOTO_ELSE_GOTO,
	MODE_IFNOT_GOTO_ELSE_GOTO
};

class Codegen
{
	vector<string> code;
	list<int> requested_labels;
	list<string> registers;
	
	public:
		map<string,data_type> register_content;
		Codegen();
		static string convert_machine_function(machine_function command);
		
		int insert_code(string command);
		int insert_code(machine_function command,
			string arg1="NULL", string arg2="NULL", string arg3="NULL");
		int insert_code(machine_function command,
			double arg1, string arg2);
		int insert_code(machine_function command,
			float arg1, string arg2);
		int insert_code(machine_function command,
			int arg1, string arg2="NULL");

		string mem_offset(string reg, int offset=0);
		string mem_offset(string reg, string offset);
		string create_function_string(string function_identifier,
			list<SymbolEntry> &locals,int position);
		
		int get_size();
		string get_register(string reg_name = "");
		string front_register();
		void put_register(string reg);
		void swap();
		int nextinstr();
		
		void print();

		pair<int,int> boolean_code(string reg, boolean_mode mode, int type = 0);
		int make_boolean(string reg, int type = 0);
		int backpatch(int line, int target);
		int backpatch(list<int> l, int target);
		int add_label(int line);
		int add_comment(string comment);
		list<string> caller_saved_registers();
		list<string> get_current_registers();
};



// Codegen Generator;

#endif