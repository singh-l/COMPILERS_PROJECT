#include <list>
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include "codegen.h"
#include "symbol_table.h"

using namespace std;
/*
MF_ADD_I,
	MF_ADD_F,
	MF_MULT_I,
	MF_MULT_F,
	MF_DIV_I,
	MF_DIV_F
*/

Codegen::Codegen(){
	registers.push_back("eax");
	registers.push_back("ebx");
	registers.push_back("ecx");
	registers.push_back("edx");
}

string Codegen::convert_machine_function(machine_function command){
	switch(command){
		case MF_ADD_I:
			return "addi";
		case MF_ADD_F:
			return "addf";
		case MF_MULT_I:
			return "muli";
		case MF_MULT_F:
			return "mulf";
		case MF_DIV_I:
			return "divi";
		case MF_DIV_F:
			return "divf";
		case MF_COMP_I:
			return "cmpi";
		case MF_COMP_F:
			return "cmpf";
		case MF_J:
			return "j";
		case MF_JE:
			return "je";
		case MF_JNE:
			return "jne";
		case MF_JL:
			return "jl";
		case MF_JLE:
			return "jle";
		case MF_JG:
			return "jg";
		case MF_JGE:
			return "jge";
		case MF_MOVE:
			return "move";
		case MF_INT_TO_FLOAT:
			return "intTofloat";
		case MF_FLOAT_TO_INT:
			return "floatToint";
		case MF_STORE_F:
			return "storef";
		case MF_STORE_I:
			return "storei";
		case MF_LOAD_I:
			return "loadi";
		case MF_LOAD_F:
			return "loadf";
		case MF_PUSH_I:
			return "pushi";
		case MF_PUSH_F:
			return "pushf";
		case MF_POP_I:
			return "popi";
		case MF_POP_F:
			return "popf";
		case MF_PRINT_INT:
			return "print_int";
		case MF_PRINT_FLOAT:
			return "print_float";
		case MF_PRINT_STRING:
			return "print_string";
		default:
			return "";
	}
}

string Codegen::mem_offset(string reg, int offset){
	if(offset == 0)
		return "ind(" + reg + ")";
	return "ind(" + reg + "," + to_string(offset) + ")";
}
string Codegen::mem_offset(string reg, string offset){
	return "ind(" + reg + "," + offset + ")";
}
int Codegen::add_comment(string comment){
	return insert_code("//" + comment);
}
int Codegen::insert_code(string command_res){
	code.push_back(command_res);
	// cout<<">>>>"<<command_res<<endl;
	return (code.size()-1);
}
int Codegen::insert_code(machine_function command, 
	string arg1, string arg2, string arg3){
	string command_res = "\t";
	command_res += convert_machine_function(command);
	switch(command){
		case MF_ADD_I:
		case MF_MULT_I:
		case MF_DIV_I:
		case MF_FLOAT_TO_INT:
		case MF_LOAD_I:
			register_content[arg2] = DT_INT;
	
		case MF_ADD_F:
		case MF_MULT_F:	
		case MF_DIV_F:
		case MF_INT_TO_FLOAT:
		case MF_LOAD_F:
			register_content[arg2] = DT_FLOAT;
			break;
		case MF_MOVE:
			if(arg1 == "eax" || arg1 == "ebx" || arg1 == "ecx" || arg1 == "edx")
				register_content[arg2] = register_content[arg1];
			break;
	}
	if(arg1 == "NULL")
		command_res += "(" + command_res + ");";
	else if(arg2 == "NULL")
		command_res += "(" + arg1 + ");";
	else if(arg3 == "NULL")
		command_res += "(" + arg1 + "," + arg2 + ");";
	else
		command_res += "(" + arg1 + "," + arg2 + "," + arg3 + ");";
	return insert_code(command_res);
}
int Codegen::insert_code(machine_function command,
	double arg1, string arg2){
	if(command == MF_MOVE)
		register_content[arg2] = DT_FLOAT;
	return insert_code(command,to_string(arg1),arg2);
}
int Codegen::insert_code(machine_function command,
	float arg1, string arg2){
	if(command == MF_MOVE)
		register_content[arg2] = DT_FLOAT;
	return insert_code(command,to_string(arg1),arg2);
}
int Codegen::insert_code(machine_function command,
	int arg1, string arg2){
	if(command == MF_MOVE)
		register_content[arg2] = DT_INT;
	if(arg2 == "NULL")
		return insert_code(command,to_string(arg1));	
	return insert_code(command,to_string(arg1),arg2);
}

int Codegen::get_size(){
	return registers.size();
}

string Codegen::get_register(string reg_name){
	if(reg_name != ""){
		registers.remove(reg_name);
		return reg_name;		
	}else{
		string reg = registers.front();
		registers.pop_front();
		return reg;
	}	
}
list<string> Codegen::get_current_registers(){
	return registers;
}
string Codegen::front_register(){
	return registers.front();
}

void Codegen::put_register(string reg){
	registers.push_front(reg);
}

list<string> Codegen::caller_saved_registers(){
	list<string> result;
	result.push_back("eax");
	result.push_back("ebx");
	result.push_back("ecx");
	result.push_back("edx");

	for(list<string>::iterator li = registers.begin(); li != registers.end(); li++){
		result.remove(*li);
	}
	return result;
}

void Codegen::swap(){
	if(get_size() <= 1){
		cout<<"DEBUG::: USELESS SWAP";
	}else{
		string s1 = get_register();
		string s2 = get_register();
		put_register(s1);
		put_register(s2);
	}
}

int Codegen::make_boolean(string reg, int type){
	//i-1 cmp(eax,0)
	//i move(0,eax)
	//i+1 jne(i+3)
	//i+2 move(1,eax)

	if(type == 0){
		insert_code(MF_COMP_F,0.0,reg);
	}else{
		insert_code(MF_COMP_I,0,reg);
	}
	insert_code(MF_MOVE,0,reg);
	add_label(nextinstr() + 2);
	insert_code(MF_JNE,"l" + to_string(nextinstr() + 2));
	return insert_code(MF_MOVE,1,reg);
}

pair<int,int> Codegen::boolean_code(string reg, boolean_mode mode, int type){
	if(type == 0)
		insert_code(MF_COMP_F,0.0,reg);
	else
		insert_code(MF_COMP_I,0,reg);
	
	if(mode == MODE_IF_GOTO_ELSE_FALL)
		return make_pair(insert_code(MF_JNE,"_"),-1);
	else if(mode == MODE_IFNOT_GOTO_ELSE_FALL)
		return make_pair(insert_code(MF_JE,"_"),-1);
	else if(mode == MODE_IF_GOTO_ELSE_GOTO)
		return make_pair(insert_code(MF_JNE,"_"),
					insert_code(MF_J,"_"));
	else if(mode == MODE_IFNOT_GOTO_ELSE_GOTO)
		return make_pair(insert_code(MF_JE,"_"),
					insert_code(MF_J,"_"));
}

int Codegen::add_label(int line){
	requested_labels.push_back(line);
	return line;
}

int Codegen::backpatch(int line, int target){
	add_label(target);
	int pos = code[line].find("_");
	code[line].replace(pos,1,"l"+to_string(target));
	return target;
}
int Codegen::backpatch(list<int> l, int target){
	for(list<int>::iterator li = l.begin(); li != l.end(); li++){
		backpatch(*li,target);
	}
	return target;
}

int Codegen::nextinstr(){
	return code.size();
}

string Codegen::create_function_string(string function_identifier,list<SymbolEntry> &locals,
	int position){	
	string output = function_identifier + "_" + to_string(position);
	output += "(";
	list<string> params;
	for(list<SymbolEntry>::iterator li = locals.begin(); li!=locals.end();li++){
		if(li->offset >= 0){
			string param;
			string identifier = li->identifier;
			pair<string,string> argument = li->dt.get_string();
			params.push_back(argument.first + " " + identifier + argument.second);
		}
	}
	int pos = 0;
	for(list<string>::iterator li = params.begin(); li != params.end(); li++){
		output += *li;
		if(pos != params.size() - 1){
			output += ",";
			pos++;
		}
	}
	output += ")";
	return output;
}
void Codegen::print(){
	ofstream fout;
	fout.open("code.asm");
	vector<bool> has_label(code.size(),false);
	for(list<int>::iterator li = requested_labels.begin(); li!= requested_labels.end(); li++){
		has_label[*li] = true;
	}

	for(int i = 0; i < code.size(); i ++ ){
		if(has_label[i])
			fout<<"\tl"<<i<<":";
		fout<<code[i]<<endl;
	}
	
	fout.close();
}