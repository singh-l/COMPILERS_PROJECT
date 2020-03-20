#ifndef ST_H_INCLUDED_
#define ST_H_INCLUDED_


#include <iostream>
#include <string>
#include <list>
#include <utility>
using namespace std;


enum error_types{
	ET_WRONG_RETURN,
	ET_WRONG_PARAMETER_TYPE,
	ET_CANNOT_ASSIGN,
	ET_INCORRECT_NUMBER_OF_ARGUMENTS,
	ET_VOID_VARIABLE,
	ET_ARRAY_ARGUMENT_INTEGER,
	ET_ARRAY_DECLARATION_INTEGER,
	ET_ARRAY_UNDER_USAGE,
	ET_ARRAY_OVER_USAGE,
	ET_UNDECLARED,
	ET_VARIABLE_UNDECLARED,
	ET_NOT_A_FUNCTION,
	ET_REDECLARTION,
	ET_VARIABLE_REDECLARATION,
	ET_CANNOT_CAST,
	ET_AMBIGUOUS_FUNCALL,
	ET_NO_MAIN,
	ET_MULTIPLE_MAIN,
	ET_NON_INT_MAIN,
	ET_INVALID_FORMAT_STRING,
	ET_UNMATCHING_FORMAT_STRING,
	ET_TOO_FEW_FORMAT_STRING,
	ET_TOO_MANY_FORMAT_STRING
};

/*
	//TODO
	array parameters
	polymorphism
*/

enum variable_type{
	VT_VARIABLE,
	VT_PARAMETER,
	VT_FUNCTION,
	VT_NULL
};

enum data_type{
	DT_INT,
	DT_FLOAT,
	DT_VOID,
	DT_ARRAY,
	DT_STRING,
	DT_ERROR
};

class ErrorElement{
public:
	error_types error;
	string identifier;
	int line_no;
	ErrorElement(error_types e,string id, int ln);
	void print_error();
	static string error_string(error_types e);
};

class SymbolTable;
class mDataType{
public:
	data_type dt;
	int size;
	mDataType* subArray;
	int index_size;

	mDataType();
	mDataType(data_type data_t);
	mDataType(mDataType* subArr, int index_s);
	static string data_type_to_string(data_type data);
	pair<string,string> get_string();
	static data_type are_compatible(data_type d1, data_type d2,int mode_funcall = 0);
	void print();
};

class SymbolEntry{
	public:
	string identifier;
	variable_type vt;
	mDataType dt;
	list<data_type> params;
	int offset;
	SymbolTable* lst; //Local Symbol Table Pointer
	int position_in_table;// for functions only

	SymbolEntry();
	SymbolEntry(string id, variable_type var_t, mDataType data_t, int ofs, SymbolTable* st);
	SymbolEntry(string id, variable_type var_t, mDataType data_t, int ofs, list<data_type> ldt, SymbolTable* st);
	SymbolEntry(string id, variable_type var_t, mDataType data_t, int ofs, list<data_type> ldt, SymbolTable* st,int position);
	void print(string tabs="");
	string print_params();
	void print_vt();
};

class SymbolTable{
	public:
	list<SymbolEntry> table;
	int current_offset;

	void addEntry(string id,variable_type var_t, mDataType data_t, int ofs, SymbolTable* st);
	void addEntry(SymbolEntry se);

	void clear_data();

	void add_offset(int value);

	void print(string tabs="");
	bool contains_declaration(string var_name);
	SymbolEntry get_declaration(string var_name);

};

#endif // ST_H_INCLUDED_
