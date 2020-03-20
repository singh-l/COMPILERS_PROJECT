#ifndef CT_H_INCLUDED_
#define CT_H_INCLUDED_

#include <iostream>
#include <list>
#include <vector>
#include <map>
#include <string>
#include "symbol_table.h"
#include "codegen.h"
using namespace std;
/*enum describing the types of classes*/

enum typeExp{
    STMT_AST = 0,
    EXP_AST, SEQ_AST,
    ASS_AST, RETURN_AST,
    IF_AST, WHILE_AST,
    FOR_AST, OPb_AST,
    OPu_AST, FUNCALL_AST,
    FLOAT_CONSTANT_AST,
    INT_CONSTANT_AST,
    STRING_CONSTANT_AST,
    IDENTIFIER_AST,
    ARRAY_REF_AST,
    INDEX_AST,
};
/*enum describing operators*/
enum operators{
	OR_OPERATION = 0,
	AND_OPERATION,
	NE_OPERATION,
	EQ_OPERATION,
	GE_OPERATION,
	LE_OPERATION,
	LT_OPERATION,
	GT_OPERATION,
	PLUS_OPERATION,
	SUB_OPERATION,
	MULT_OPERATION,
	DIV_OPERATION,
	MINUS_OPERATION,
	BANG_OPERATION,
	ASSIGN_OPERATION,
	INC_OPERATOR,
	FLOAT_CAST,
	INT_CAST,
	BOOLEAN_CAST,
	PUSH_OP,
	POP_OP,
	LOAD_OP,
	STORE_OP
};
/*The most general class*/
class abstract_astnode
{
	public:
		virtual void print (int tabs = 0);
		int getType();
		bool checkTypeofAST();
		data_type inherent_dt;
		int line_no;
		mDataType mdata_type;

		//Needed for Code Generation
		static Codegen Generator;
		int min_register;
		bool is_leaf = false, has_assignment = false, is_assignment = false;
		bool is_boolean = false, is_constant = false;
		bool isEmpty = false, fall_through = false;
		static int get_new_label(int,int);
		virtual void gen_code(int mode=0);
		virtual string get_address();
		virtual string get_variable();
		virtual void setLabel(int argument_number){;}
		list<int> truelist, falselist, nextlist;
		
	protected:
		void setType(typeExp type);
		string getOperatorStr(operators val);
		void print_tabs(int tabs);
		data_type are_compatible_datatypes(data_type d1, data_type d2);
		operators get_cast(data_type dt);
	private:
		typeExp astnode_type;
};


class StmtAst : public abstract_astnode
{
	public:
	StmtAst();
};	
class ExpAst : public abstract_astnode
{
	public:
	ExpAst();
};

class Seq : public StmtAst
{
	SymbolTable* st;
	int position;
	data_type function_dt;
	string function_identifier;
	list<StmtAst*> lst;
	static int parameter_offset;
	static int variable_offset;
	public:
	Seq();
	Seq(list<StmtAst *> ls);
	static void return_call();
	void setSymbolTable(SymbolTable* s, int p,string function_identifier,data_type dt);
	void print(int tabs);
	void setLabel(int argument_number);
	void gen_code(int mode=0);
};

class Ass : public StmtAst
{
	ExpAst *exp_ast_1,*exp_ast_2;
	public:
	Ass();
	Ass(ExpAst *exp_ast_a,ExpAst *exp_ast_b);
	void setLabel(int argument_number);
	void print(int tabs);
	void gen_code(int mode=0);
};

class Return : public StmtAst
{
	//here
	ExpAst *exp_ast_1;
	public:
	Return(ExpAst *exp_ast_a,mDataType data);
	void setLabel(int argument_number);
	void print(int tabs);
	void gen_code(int mode=0);
};

class If : public StmtAst
{
	ExpAst *exp_ast_1;
	StmtAst *stmt_ast_2;
	StmtAst *stmt_ast_3;
	public:
	If(ExpAst *exp_ast_a,
		StmtAst *stmt_ast_b,
		StmtAst *stmt_ast_c);
	void setLabel(int argument_number);
	void print(int tabs);
	void gen_code(int mode=0);
};

class While : public StmtAst
{
	ExpAst *exp_ast_1;
	StmtAst *stmt_ast_2;
	public:
	While(ExpAst *exp_ast_a,StmtAst *stmt_ast_b);
	void setLabel(int argument_number);
	void print(int tabs);
	void gen_code(int mode=0);
};

class For : public StmtAst
{
	ExpAst *exp_ast_1,*exp_ast_2,*exp_ast_3;
	StmtAst *stmt_ast_4;
	public:
	For(ExpAst *e1,ExpAst *e2,ExpAst *e3, StmtAst *s4);
	void print(int tabs);
	void setLabel(int argument_number);
	void gen_code(int mode=0);
};


class OpBinary : public ExpAst
{
	ExpAst *exp_ast_1,*exp_ast_2;
	operators op;
	public:
	OpBinary(ExpAst *exp_ast_a,operators o,ExpAst *exp_ast_b);
	void print(int tabs);
	void setLabel(int argument_number);
	void gen_code(int mode=0);
	void optimizeOrder();
	void compHelper(int code, machine_function J1, 
		machine_function J2, machine_function J3);
	static void genCode(ExpAst* exp_ast, bool fall = true);
	static machine_function chooseOperation(int op, int type);
	static void genCodeBool(ExpAst* exp_ast);
};

class OpUnary : public ExpAst
{
	ExpAst *exp_ast;
	operators op;
	public:
	OpUnary(ExpAst *exp_ast_a,operators o);
	void print(int tabs);
	void gen_code(int mode=0);
	void setLabel(int argument_number);
};


class FuncallStmt : public StmtAst
{
	string func_name;
	int position;
	list<ExpAst*> list_exp_ast;
	public:
	FuncallStmt(string fn, list<ExpAst*> lst,mDataType data, list<data_type> params,bool ignore_rule,int position=0);
	FuncallStmt(string fn);
	void setLabel(int argument_number);
	void print(int tabs);
	void gen_code(int mode=0);
};


class Funcall : public ExpAst
{
	string func_name;
	int position;
	list<ExpAst*> list_exp_ast;
	public:
	Funcall(string fn, list<ExpAst*> lst,mDataType data, list<data_type> params,bool ignore_rule,int position = 0);
	Funcall(string fn);
	void print(int tabs);
	void setLabel(int argument_number);
	void gen_code(int mode=0);
};

class FloatConstant : public ExpAst
{
	float float_val;
	public:
	FloatConstant(float fl);
	FloatConstant(string fl);
	float get_value();
	void print(int tabs);
	void set_value(float no);
	void setLabel(int argument_number);
	void gen_code(int mode=0);
};

class IntConstant : public ExpAst
{
	int int_val;
	public:
	IntConstant(int intv);
	IntConstant(string s);
	int get_value();
	void print(int tabs);
	void set_value(int no);
	void setLabel(int argument_number);
	void gen_code(int mode=0);
};

class StringConstant : public ExpAst
{
	string string_val;
	public:
	StringConstant(string str);
	void print(int tabs);
	void setLabel(int argument_number);
	void gen_code(int mode=0);
	string get_value();
};

class ArrayRef : public ExpAst
{
	public:
	ArrayRef();
};

class Index : public ArrayRef
{
	SymbolEntry symbol_entry;
	ArrayRef *array_ref;
	ExpAst *exp_ast;
	public:
	Index(ArrayRef *arr,ExpAst *exp,data_type inherent_dt,mDataType data);
	void print(int tabs);
	void setLabel(int argument_number);
	void gen_code(int mode=0);
	string get_address();
	string get_variable();
	string get_base_address();
	void index_gen_code(int mode, mDataType* mdt, int size_from_top = 1);
};

class Identifier : public ArrayRef
{
	string identifier;
	public:
	SymbolEntry symbol_entry;
	Identifier(string str);
	Identifier(string str,SymbolEntry sym_entry);
	void print(int tabs);
	void setLabel(int argument_number);
	void gen_code(int mode=0);
	string get_address();
	string get_variable();
};

#endif
