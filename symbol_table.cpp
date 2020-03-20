#include <iostream>
#include <string>
#include <list>
#include <utility>
#include "symbol_table.h"

using namespace std;


ErrorElement::ErrorElement(error_types e,string id, int ln):error(e), identifier(id), line_no(ln)
{;}

string ErrorElement::error_string(error_types e){
	switch(e){
		case ET_WRONG_RETURN:
			return "Return type mismatch";
		case ET_WRONG_PARAMETER_TYPE:
			return "No matching function(incompatible args)";
		case ET_CANNOT_ASSIGN:
			return "Cannot assign, incompatible types";
		case ET_INCORRECT_NUMBER_OF_ARGUMENTS:
			return "No matching function(number of args)";
		case ET_VOID_VARIABLE:
			return "Cannot declare void variable";
		case ET_ARRAY_ARGUMENT_INTEGER:
			return "Array argument must be integer";
		case ET_ARRAY_DECLARATION_INTEGER:
			return "Array declaration must be integer";
		case ET_ARRAY_UNDER_USAGE:
			return "Array cannot be casted to variable";
		case ET_ARRAY_OVER_USAGE:
			return "Array call for atomic type";
		case ET_UNDECLARED:
			return "Undeclared function";
		case ET_VARIABLE_UNDECLARED:
			return "Undeclared variable";
		case ET_NOT_A_FUNCTION:
			return "Variable not a function";
		case ET_REDECLARTION:
			return "Function redeclared";
		case ET_VARIABLE_REDECLARATION:
			return "Variable redeclared";
		case ET_CANNOT_CAST:
			return "Function not a variable";
		case ET_AMBIGUOUS_FUNCALL:
			return "Ambiguous function call";
		case ET_NO_MAIN:
			return "No main found";
		case ET_MULTIPLE_MAIN:
			return "Multiple main cannot be declared";
		case ET_NON_INT_MAIN:
			return "No INT typed main was found";
		case ET_INVALID_FORMAT_STRING:
			return "Invalid format string";
		case ET_UNMATCHING_FORMAT_STRING:
			return "Type mismatch with format string";
		case ET_TOO_FEW_FORMAT_STRING:
			return "Too few arguments for format string";
		case ET_TOO_MANY_FORMAT_STRING:
			return "Too many arguments for format string";
		default:
			return "Runtime Error";
	}
}
void ErrorElement::print_error(){
	if(error != ET_NO_MAIN) 
	cout<<"  error:"<<line_no+1<<": "<<error_string(error)<<" in line "<<line_no+1<<" for "<<identifier<<endl;
	else cout<<"error: No int main() found"<<endl;
}

mDataType::mDataType(data_type data_t){
	dt = data_t;
	switch(dt){
		case DT_INT:
		case DT_FLOAT:
			size = 4;
			break;
		case DT_VOID:
			size = 0;
			break;
		default: 
			size =0;
	}
}

pair<string,string> mDataType::get_string(){
	if(dt == DT_FLOAT || dt == DT_INT || dt == DT_VOID  || dt == DT_STRING)
		return make_pair(data_type_to_string(dt),"");
	else if(dt == DT_ARRAY){
		mDataType* mdt = this;
		string arg = "";
		while(mdt->dt == DT_ARRAY){
			arg += "[" + to_string(index_size) + "]";
			mdt = mdt->subArray;
		}
		string type = data_type_to_string(mdt->dt);
		return make_pair(type,arg);
	}
}

void mDataType::print(){
	if(dt == DT_FLOAT)
		cout<<"float";
	else if(dt == DT_INT)
		cout<<"int";
	else if(dt == DT_VOID)
			cout<<"void";
	else if(dt == DT_ARRAY){
		cout<<"array("<<index_size<<",";
		subArray->print();
		cout<<")";
	}
}

data_type mDataType::are_compatible(data_type d1, data_type d2, int mode_funcall){
	switch(d1){
		case DT_INT:
			if(d2 == DT_INT)
				return DT_INT;
			if(d2 == DT_FLOAT)
				return DT_FLOAT;
			return DT_ERROR;
		case DT_FLOAT:
			if(d2 == DT_FLOAT || d2 == DT_INT)
				return DT_FLOAT;
			return DT_ERROR;
		case DT_VOID:
			if(d2 == DT_VOID)
				return DT_VOID;
			return DT_ERROR;
		case DT_ARRAY:
			if(d2 == DT_ARRAY && mode_funcall == 1)
				return DT_ARRAY;
			return DT_ERROR;
		default:
			return DT_ERROR;
	}
}


string mDataType::data_type_to_string(data_type data){
	if(data == DT_INT)
		return "int";
	if(data == DT_FLOAT)
		return "float";
	if(data == DT_VOID)
		return "void";
	if(data == DT_STRING)
		return "string";
	if(data == DT_ERROR)
		return "error";
	if(data == DT_ARRAY)
		return "array";
	return "others";
}

mDataType::mDataType(){;}

mDataType::mDataType(mDataType* subArr, int index_s){
	subArray = subArr;
	index_size = index_s;
	dt = DT_ARRAY;
	size = index_s*subArray->size;
}

SymbolEntry::SymbolEntry(){;}
SymbolEntry::SymbolEntry(string id, variable_type var_t, mDataType data_t, int ofs, SymbolTable* st){
	identifier = id;
	dt = data_t;
	vt = var_t;
	offset = ofs;
	lst = st;
}
SymbolEntry::SymbolEntry(string id, variable_type var_t, 
	mDataType data_t, int ofs, list<data_type> ldt, SymbolTable* st){
	identifier = id;
	dt = data_t;
	vt = var_t;
	offset = ofs;
	lst = st;
	params = ldt;
}
SymbolEntry::SymbolEntry(string id, variable_type var_t, 
	mDataType data_t, int ofs, list<data_type> ldt, SymbolTable* st, int pos){
	identifier = id;
	dt = data_t;
	vt = var_t;
	offset = ofs;
	lst = st;
	params = ldt;
	position_in_table = pos;
}
void SymbolEntry::print_vt(){
	if(vt == VT_VARIABLE)
		cout<<"variable";
	else if(vt == VT_PARAMETER)
		cout<<"parameter";
	else if(vt == VT_FUNCTION)
		cout<<"function";
}
string SymbolEntry::print_params(){
	string fname_msg = "(";
	int pc = 1;
	for(list<data_type>::iterator li = params.begin(); li!= params.end(); li++){
		if(*li == DT_ERROR)
			fname_msg += "?";
		else
			fname_msg += mDataType::data_type_to_string(*li);

		if(pc != params.size()){
			pc++;
			fname_msg += ",";
		}else
			fname_msg += ")";		
	}
	if(params.size()==0)
		fname_msg += ")";			
	return fname_msg;
}

void SymbolEntry::print(string tabs){
	string seperator = "  |  ";
	cout<<tabs<<"|  ";
	cout<<identifier;	cout<<seperator;
	print_vt();	 cout<<seperator;
	dt.print();  cout<<seperator;
	cout<<offset;	cout<<seperator;
	cout<<endl;

	if(lst!=NULL){
		lst->print(tabs + "    ");
	}
}

	bool SymbolTable::contains_declaration(string var_name){
		for(list<SymbolEntry>::iterator li = table.begin(); li!=table.end(); li++){
			if(li->identifier == var_name)
				return true;
		}
		return false;
	}

	SymbolEntry SymbolTable::get_declaration(string var_name){
		for(list<SymbolEntry>::iterator li = table.begin(); li!=table.end(); li++){
			if(li->identifier == var_name)
				return (*li);
		}
	}

	void SymbolTable::addEntry(string id,variable_type var_t, mDataType data_t, int ofs, SymbolTable* st){
		SymbolEntry se(id,var_t,data_t,ofs,st);
		table.push_back(se);
	}
	void SymbolTable::addEntry(SymbolEntry se){
		table.push_back(se);
	}
	void SymbolTable::clear_data(){
		table.clear();
		current_offset = 0;
	}

	void SymbolTable::add_offset(int value){
		for(list<SymbolEntry>::iterator li = table.begin(); li!=table.end(); li++){
			li->offset = li->offset + value;
		}
	}

	void SymbolTable::print(string tabs){
		for(list<SymbolEntry>::iterator li = table.begin(); li!=table.end(); li++){
			li->print(tabs);
		}	
		cout<<endl;
	}
