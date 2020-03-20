#include <list>
#include <set>
#include <cstring>
#include <fstream>
#include <iostream>
#include "symbol_table.h"

#define DEBUG_MODE 0
using namespace std;

int address_size = 4;
//to ignore the l expressions 
bool l_expression_ignore = false;
list<data_type> empty_dt;

//contains the globals ... function_dt is data type of function
//array_dt is the dynamic data type of arrays or variables (to check count of indexes)
//static_array_dt is the constant data type of the entire array
mDataType function_dt, array_dt, static_array_dt;

//The stacks for layered calls to arrays
list<mDataType> array_stack,static_array_stack;
list<string> array_name_stack;
list<ExpAst*> print_expast;
//the data type for variables for declaration and parameters
mDataType* variable_dt,*variable_type_dt;

//the data type of the current state
data_type global_inherent_dt;

//the symbol tables and parameters and variables
SymbolTable global_symbol_table, current_parameter_list,current_variable_list;
//the correct function corresponding to function call
SymbolEntry correct_function;

//the identifer for function and variables stored as globals
string function_identifier,variable_identifier;

//the current identifier and array name
string global_var_string, array_name,array_prev_name;
int global_line_number;

//global for error existance
bool error_exists = false;

//the default mdata type
mDataType DEFAULT_DT = mDataType(DT_INT);

list<ErrorElement> error_list;


//FUNCTION declarations
list<data_type> getFunctionParams(list<SymbolEntry> locals);
list<mDataType> getmDataFunctionParams(list<SymbolEntry> locals);


//Adds to the list of errors
void addErrorElement(error_types e,string id, int ln){
	ErrorElement ee(e,id,ln);
	error_list.push_back(ee);
}

//parses the variable to get the line number
void parseVar(string input){
	int space = input.find(" ");
	if(space == string::npos){
		global_var_string = input;
		global_line_number = 0;
		return;
	}
	global_var_string  = input.substr(0,space);
	string line_number = input.substr(space+1);
	global_line_number = stoi(line_number);
	return;

}

//is already defined function/variable
bool is_already_defined(string var_name, int mode = 0){
	if(mode == 0)
		return (current_parameter_list.contains_declaration(var_name)
			|| current_variable_list.contains_declaration(var_name)
			|| global_symbol_table.contains_declaration(var_name));
	else if(mode == 1)
		return (current_parameter_list.contains_declaration(var_name)
			|| current_variable_list.contains_declaration(var_name));
}

//checks if the 2 list of argmuments and function parameters are "compatible"
int same_list(list<data_type> l1, list<data_type> l2, int mode = 1){
	if(l1.size() != l2.size())
		return 1;

	if(mode == 1 && l1==l2)
		return 2;
	else if(mode == 1)
		return 3;
	
	list<data_type>::iterator li2 = l2.begin();
	for(list<data_type>::iterator li1 = l1.begin(); li1 != l1.end(); li1++,li2++){
		if(mDataType::are_compatible(*li1,*li2,1) == DT_ERROR){
			return 4;
		}		
	}
	return 5;

}

//checks if 2 arrays are the same type and hence actually compatible
bool check_same_array(mDataType d1, mDataType d2){
	if(d1.dt != d2.dt)
		return false;
	if(d1.dt != DT_ARRAY)
		return true;
	if(d1.index_size != d2.index_size)
		return false;

	return check_same_array(*d1.subArray, *d2.subArray);
}

//Checks if the 2 lists of arguments and function parameters are same wrt arrays
bool same_list_for_arrays(list<ExpAst*> l1, list<mDataType> l2){

	if(l1.size() != l2.size())
		return false;
	
	list<mDataType>::iterator li2 = l2.begin();
	for(list<ExpAst*>::iterator li1 = l1.begin(); li1 != l1.end(); li1++,li2++){
		if((*li1)->inherent_dt == DT_ARRAY){
			if(!check_same_array((*li1)->mdata_type,(*li2)))
				return false;
		}
	}
	return true;
}
//Checks if the 2 lists of arguments and function parameters are same wrt arrays
bool same_list_for_arrays(list<mDataType> l1, list<mDataType> l2){

	if(l1.size() != l2.size())
		return false;
	
	list<mDataType>::iterator li2 = l2.begin();
	for(list<mDataType>::iterator li1 = l1.begin(); li1 != l1.end(); li1++,li2++){
		if(li1->dt == DT_ARRAY){
			if(!check_same_array((*li1),(*li2)))
				return false;
		}
	}
	return true;
}

//find the match count amongst 2 lists
int match_count(list<data_type> l1, list<data_type> l2){
	int count = 0;
	list<data_type>::iterator li2 = l2.begin();
	for(list<data_type>::iterator li1 = l1.begin(); li1 != l1.end(); li1++,li2++){
		if(*li1 == *li2){
			count++;
		}
	}
	return count;
}

//finds the list of matching functions with given name
list<SymbolEntry> matching_functions(string fname){
	list<SymbolEntry> matching;
	for(list<SymbolEntry>::iterator li = global_symbol_table.table.begin();
		li!=global_symbol_table.table.end(); li++){
		if(li->identifier == fname)
			matching.push_back(*li);
	}
	return matching;
}

//checks if the function is already defined or not 
//(needs to be specially defined to allow for polymorphism)
bool is_polymorphic_function(list<SymbolEntry>param_table,string fname){
	list<data_type> params = getFunctionParams(param_table);
	list<mDataType> params_mData = getmDataFunctionParams(param_table);

	list<SymbolEntry> matching = matching_functions(fname);
	for(list<SymbolEntry>::iterator li = matching.begin(); li!=matching.end(); li++){
		if(same_list(li->params,params,1) == 2){
			if(same_list_for_arrays(getmDataFunctionParams(li->lst->table),params_mData))
				return false;
		}
	}
	return true;
}

//searchs a vriable/function/etc from the current set of variables/functions/parameters
SymbolEntry search_variable(string var_name){
	if(current_parameter_list.contains_declaration(var_name)){
		return current_parameter_list.get_declaration(var_name);
	}
	if(current_variable_list.contains_declaration(var_name)){
		return current_variable_list.get_declaration(var_name);
	}
	if(global_symbol_table.contains_declaration(var_name)){
		return global_symbol_table.get_declaration(var_name);
	}
	if(function_identifier == var_name){
		return SymbolEntry(var_name,VT_FUNCTION,function_dt.dt,0,&current_parameter_list);
	}
	SymbolEntry null_entry("",VT_NULL,DT_ERROR,0,NULL);
	return null_entry;
}

//gets the function parameters as data_types
list<data_type> getFunctionParams(list<SymbolEntry> locals){
	list<data_type> params;
	for(list<SymbolEntry>::iterator li = locals.begin();li!=locals.end();li++){
		if(li->offset >= 0)
			params.push_back((*li).dt.dt);
	}
	return params;
}
//gets the function parameters as mDataTypes
list<mDataType> getmDataFunctionParams(list<SymbolEntry> locals){
	list<mDataType> params;
	for(list<SymbolEntry>::iterator li = locals.begin();li!=locals.end();li++){
		if(li->offset >= 0)
			params.push_back((*li).dt);
	}
	return params;
}
bool validPrintF(list<ExpAst*> lst){
	vector<ExpAst*> v;
	list<ExpAst*> new_list;
	for(list<ExpAst*>::iterator li = lst.begin(); li!=lst.end(); li++){
		v.push_back(*li);
	}
	if((v[0])->inherent_dt != DT_STRING){
		addErrorElement(ET_INVALID_FORMAT_STRING,global_var_string,global_line_number);
		return false;
	}
	string format = ((StringConstant*)v[0])->get_value();
	format = format.substr(1,format.length()-2);
	bool dollar = false;
	string to_print = "";
	int position = 1;
	for(int i = 0 ; i < format.length(); i++){
		if(format[i] == '%' && dollar){
			dollar = false;
			to_print += '%';
		}
		else if(format[i] == '%')
			dollar = true;
		else if(dollar){
			dollar = false;
			if(position >= v.size()){
				addErrorElement(ET_TOO_FEW_FORMAT_STRING,global_var_string,global_line_number);
				return false;
			}
			if(format[i] == 's' || format[i] == 'd' || format[i] == 'f'){
				if(format[i] == 's' && v[position]->inherent_dt != DT_STRING){
					addErrorElement(ET_UNMATCHING_FORMAT_STRING,global_var_string,global_line_number);
					return false;
				}
				if(format[i] != 's' && (
					v[position]->inherent_dt != DT_INT  &&
					v[position]->inherent_dt != DT_FLOAT)){
					addErrorElement(ET_UNMATCHING_FORMAT_STRING,global_var_string,global_line_number);
					return false;
				}
					
				if(format[i] == 'd' && v[position]->inherent_dt == DT_FLOAT){
					v[position] = new OpUnary(v[position],INT_CAST);					
				}
				if(format[i] == 'f' && v[position]->inherent_dt == DT_INT){
					v[position] = new OpUnary(v[position],FLOAT_CAST);
				}

				if(to_print != ""){
					StringConstant* s = new StringConstant('\"' + to_print + '\"');
					new_list.push_back(s);
					to_print = "";
				}
				new_list.push_back(v[position]);
				position++;
			}else{
				addErrorElement(ET_INVALID_FORMAT_STRING,global_var_string,global_line_number);
				return false;
			}			
		}else{
			to_print += format[i];
		}
	}
	if(dollar){
		addErrorElement(ET_INVALID_FORMAT_STRING,global_var_string,global_line_number);
		return false;
	}else if(position < v.size()){
		addErrorElement(ET_TOO_MANY_FORMAT_STRING,global_var_string,global_line_number);
		return false;
	}
	if(to_print != ""){
		StringConstant* s = new StringConstant('\"' + to_print + '\"');
		new_list.push_back(s);
	}
	print_expast = new_list;
	return true;
}

//Checks if the function call is valid
bool isValidFunctionCall(list<ExpAst*> lst,string fname){

	list<data_type> input_lst;

	for(list<ExpAst*>::iterator li = lst.begin(); li!=lst.end(); li++){
		data_type dt = (*li)->inherent_dt;
		input_lst.push_back(dt);
	}
	
	list<SymbolEntry> current_functions = matching_functions(fname);
	
	if(function_identifier == fname)
		current_functions.push_back(SymbolEntry(fname,VT_FUNCTION,
			function_dt.dt,0,
			getFunctionParams(current_parameter_list.table),
			&current_parameter_list,global_symbol_table.table.size()));

	bool arg_count_error = false, arg_type_error = false; 
	bool found_a_max = false, max_conflict = false;
	int answer_count = 0;

	SymbolEntry best_current_fn;

	for(list<SymbolEntry>::iterator li = current_functions.begin();
		li!=current_functions.end();li++){
		list<data_type> params = li->params;
		int e_type = same_list(input_lst,params,2);

		int tmp_count;
		if(e_type == 5){
			if(!same_list_for_arrays(lst,getmDataFunctionParams(li->lst->table)))
				e_type = 4;
			else{
				tmp_count = match_count(input_lst,params);
				answer_count++;
				if(tmp_count == params.size()){
					found_a_max = true;
					best_current_fn = *li;
				}else if(!found_a_max){
					best_current_fn = *li;
				}
			}
		}
		if(e_type == 1)
			arg_count_error = true;
		if(e_type == 4)
			arg_type_error = true;
	}

	string fname_msg = fname + "(";
	int pc = 1;
	for(list<data_type>::iterator li = input_lst.begin(); li!= input_lst.end(); li++){
		if(*li == DT_ERROR)
			fname_msg += "?";
		else
			fname_msg += mDataType::data_type_to_string(*li);

		if(pc != input_lst.size()){
			pc++;
			fname_msg += ",";
		}					
	}
	fname_msg += ")";

	if(!found_a_max && answer_count > 1){
		addErrorElement(ET_AMBIGUOUS_FUNCALL,fname_msg,global_line_number);
		return false;
	}else if(found_a_max || answer_count == 1){
		correct_function = best_current_fn;
		return true;
	}

	if(arg_type_error){
		addErrorElement(ET_WRONG_PARAMETER_TYPE,fname_msg,global_line_number);
		return false;
	}
	if(arg_count_error){
		addErrorElement(ET_INCORRECT_NUMBER_OF_ARGUMENTS,fname_msg,global_line_number);
		return false;
	}
	
}

//undos the effect of a l_expression
void valid_l_expression(){
	if(l_expression_ignore == true){
		l_expression_ignore = false;
		return;
	}
    array_prev_name = array_name;
    static_array_dt = static_array_stack.back();
    array_dt = array_stack.back();
    array_name = array_name_stack.back();
    static_array_stack.pop_back();
    array_stack.pop_back();
    array_name_stack.pop_back();
    return;
}