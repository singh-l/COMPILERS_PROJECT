
#include <iostream>
#include <list>
#include <string>

#include "class_structure.h"

#define CALLEE_SAVED_COUNT 0
#define CALLER_SAVED_COUNT 4

using namespace std;

		void abstract_astnode::print (int tabs){;}
		void abstract_astnode::gen_code (int mode){;}
		string abstract_astnode::get_address(){return "";}
		string abstract_astnode::get_variable(){ return "";}

		Codegen abstract_astnode::Generator = Codegen();
		int abstract_astnode::getType(){
			return astnode_type;
		}
		bool abstract_astnode::checkTypeofAST(){;}
	
		void abstract_astnode::setType(typeExp type){
			astnode_type = type;
		}

		int abstract_astnode::get_new_label(int reg_1,int reg_2){
			if(reg_1 == reg_2)
				return reg_1 + 1;
			if(reg_1 > reg_2)
				return reg_1;
			return reg_2;
		}
		/*Gives string corresponding to operation for printing*/
		string abstract_astnode::getOperatorStr(operators val){
			if(val==OR_OPERATION)
				return "OR";
			else if(val==AND_OPERATION)
				return "AND";
			else if(val==NE_OPERATION)
				return "NE";
			else if(val==EQ_OPERATION)
				return "EQ";
			else if(val==GE_OPERATION)
				return "GE";
			else if(val==LE_OPERATION)
				return "LE";
			else if(val==LT_OPERATION)
				return "LT";
			else if(val==GT_OPERATION)
				return "GT";
			else if(val==PLUS_OPERATION)
				return "ADD";
			else if(val==SUB_OPERATION)
				return "SUB";
			else if(val==MULT_OPERATION)
				return "MULT";
			else if(val==DIV_OPERATION)
				return "DIV";
			else if(val==MINUS_OPERATION)
				return "NEG";
			else if(val==BANG_OPERATION)
				return "NOT";
			else if(val==ASSIGN_OPERATION)
				return "Assign_exp";
			else if(val==INC_OPERATOR)
				return "PP";
			else if(val==FLOAT_CAST)
				return "FloatCast";
			else if(val==INT_CAST)
				return "IntegerCast";
			else
				return "OPERATOR";
		}
		void abstract_astnode::print_tabs(int tabs){
			if(tabs!=0)
				cout<<endl;
			for(int i=0;i<tabs;i++)
				cout<<"\t";
		}
		/*Gives the appropriate casting function to obtain the given type*/ 
		operators abstract_astnode::get_cast(data_type dt){
			if(dt == DT_INT)
				return INT_CAST;
			return FLOAT_CAST;
		}
		/* Gives as output the type of the object obtained on operating d1 with d2 
		(not considering bool, < or assignment)*/
		data_type abstract_astnode::are_compatible_datatypes(data_type d1, data_type d2){
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
				default:
					return DT_ERROR;
			}
		}

	StmtAst::StmtAst(){
		setType(STMT_AST);
	}

	ExpAst::ExpAst(){
		setType(EXP_AST);
	}

	Seq::Seq(){
		st = NULL;}

	void Seq::setSymbolTable(SymbolTable* s, int p, string id, data_type dt){
		st = s;
		position = p;
		function_identifier = id;
		function_dt = dt;
	}

	/* A seq is of type error iff one of the constituents are erroneous */
	Seq::Seq(list<StmtAst *> ls){
		st = NULL;
		lst = ls;
		setType(SEQ_AST);
		inherent_dt = DT_VOID;
		for(list<StmtAst*>::iterator li = lst.begin(); li!=lst.end(); li++){
			if((*li)->inherent_dt == DT_ERROR){	
				inherent_dt = DT_ERROR;
				break;
			}
		}
	}	
	int Seq::parameter_offset = 0;
	int Seq::variable_offset = 0;

	void Seq::return_call(){
		if(variable_offset != 0)
			abstract_astnode::Generator.insert_code(MF_ADD_I,variable_offset,"esp");
		abstract_astnode::Generator.insert_code(MF_LOAD_I,"ind(ebp)","ebp");
		abstract_astnode::Generator.insert_code(MF_POP_I,1);
		if(parameter_offset != 0)
			abstract_astnode::Generator.insert_code(MF_ADD_I,parameter_offset,"esp");
	}
	void Seq::gen_code (int mode){
		int variable_size = 4;

		if(st != NULL){
			parameter_offset = 0;
			variable_offset = 0;
			// string function_name = abstract_astnode::Generator.create_function_string(function_identifier,
			// 	st->table,position);
			string function_name = function_identifier + "_" + to_string(position) + "()";
			if(function_identifier == "main")
				function_name = "main()";
			abstract_astnode::Generator.insert_code("void " + function_name + "{");

			abstract_astnode::Generator.insert_code(MF_PUSH_I,"ebp");
			abstract_astnode::Generator.insert_code(MF_MOVE,"esp","ebp");
			
			for(list<SymbolEntry>::iterator li = (st->table).begin();
					li != (st->table).end(); li++){
				if(li->vt == VT_VARIABLE){
					variable_offset += (li->dt).size;
				}else if(li->vt == VT_PARAMETER){
					parameter_offset += variable_size;
				}
			}
			if(variable_offset != 0)
				abstract_astnode::Generator.insert_code(MF_ADD_I,-1*variable_offset,"esp");
		}
		for(list<StmtAst*>::iterator li = lst.begin(); li!=lst.end(); li++){
			(*li)->setLabel(0);
			(*li)->gen_code();
			abstract_astnode::Generator.backpatch((*li)->nextlist, abstract_astnode::Generator.nextinstr());
		}

		if(st != NULL){
			//return wala code
			return_call();
			if(function_dt == DT_FLOAT)
				abstract_astnode::Generator.insert_code(MF_PUSH_F,0.0);
			if(function_dt == DT_INT)
				abstract_astnode::Generator.insert_code(MF_PUSH_I,0);
			
			abstract_astnode::Generator.insert_code("}\n");
		}

	}
	void Seq::print(int tabs){
		print_tabs(tabs);
		isEmpty = true;
		cout<<"(Block [";
		for(list<StmtAst*>::iterator li = lst.begin(); li!=lst.end(); ){
			(*li)->print(tabs+1);
			if((*li)->isEmpty == false)
				isEmpty = false;
			li++;
		}
		cout<<"])";
	}
	void Seq::setLabel(int argument_number){

	}
	Ass::Ass(){
		isEmpty = true;
	}
	//Assignments follows different rules as compared to other binary operations. They are implemented here.
	Ass::Ass(ExpAst *exp_ast_a,ExpAst *exp_ast_b):exp_ast_1(exp_ast_a), exp_ast_2(exp_ast_b){
		setType(ASS_AST);
		isEmpty = false;
		is_assignment = true;
		has_assignment = true;
		if(exp_ast_1->inherent_dt == DT_ERROR ||
			exp_ast_2->inherent_dt == DT_ERROR ||
			are_compatible_datatypes(exp_ast_1->inherent_dt,
				exp_ast_2->inherent_dt) == DT_ERROR)
			inherent_dt = DT_ERROR;
		else{
			inherent_dt = DT_VOID;
			if(exp_ast_1->inherent_dt != exp_ast_2->inherent_dt && !is_constant){
				operators op = get_cast(exp_ast_1->inherent_dt);
				//Cast applied on rhs if lhs and rhs type don't match
				exp_ast_2 = new OpUnary(exp_ast_2,op);
			}
		}
	}
	void Ass::gen_code (int mode){	//Done with nextlist
		if(!isEmpty){	
			abstract_astnode::Generator.add_comment(" ASSIGNMENT of " + exp_ast_1->get_variable());
			/*if(exp_ast_1->inherent_dt == DT_FLOAT){
				if(exp_ast_2->is_constant){
					abstract_astnode::Generator.insert_code(MF_STORE_F,((FloatConstant*) exp_ast_2)->get_value(),exp_ast_1->get_address());
				}else{
					OpBinary::genCode(exp_ast_2);
					string reg = abstract_astnode::Generator.front_register();
					abstract_astnode::Generator.insert_code(MF_STORE_F,reg,exp_ast_1->get_address());
				}
				
			}
			else if(exp_ast_1->inherent_dt == DT_INT){
				if(exp_ast_2->is_constant){
					abstract_astnode::Generator.insert_code(MF_STORE_I,((IntConstant*) exp_ast_2)->get_value(),exp_ast_1->get_address());
				}else{
					OpBinary::genCode(exp_ast_2);
					string reg = abstract_astnode::Generator.get_register();
					abstract_astnode::Generator.insert_code(MF_STORE_I,reg,exp_ast_1->get_address());
					abstract_astnode::Generator.put_register(reg);
				}
				
			}*/
			if(exp_ast_2->getType() ==  INT_CONSTANT_AST) {
				abstract_astnode::Generator.insert_code(MF_STORE_I,((IntConstant*) exp_ast_2)->get_value(),exp_ast_1->get_address());
			}
			else if(exp_ast_2->getType() ==  FLOAT_CONSTANT_AST){
				abstract_astnode::Generator.insert_code(MF_STORE_F,((FloatConstant*) exp_ast_2)->get_value(),exp_ast_1->get_address());
			}
			else if(exp_ast_1->min_register < 4 && (exp_ast_1->min_register < exp_ast_2->min_register)){
				OpBinary::genCode(exp_ast_2);
				abstract_astnode::Generator.backpatch(exp_ast_2->nextlist, abstract_astnode::Generator.nextinstr());
				string reg = abstract_astnode::Generator.get_register();
				abstract_astnode::Generator.insert_code(OpBinary::chooseOperation(STORE_OP, exp_ast_2->inherent_dt),reg,exp_ast_1->get_address());
				abstract_astnode::Generator.put_register(reg);
			}
			else if(exp_ast_2->min_register < abstract_astnode::Generator.get_size() && !exp_ast_1->has_assignment && !exp_ast_2->has_assignment) {
				string val = exp_ast_1->get_address();
				string reg = abstract_astnode::Generator.get_register();
				OpBinary::genCode(exp_ast_2);
				abstract_astnode::Generator.backpatch(exp_ast_2->nextlist, abstract_astnode::Generator.nextinstr());
				abstract_astnode::Generator.insert_code(OpBinary::chooseOperation(STORE_OP, exp_ast_2->inherent_dt),abstract_astnode::Generator.front_register(),val);
				abstract_astnode::Generator.put_register(reg);
				abstract_astnode::Generator.swap();
			}
			else {
				OpBinary::genCode(exp_ast_2);
				abstract_astnode::Generator.backpatch(exp_ast_2->nextlist, abstract_astnode::Generator.nextinstr());
				string reg2 = abstract_astnode::Generator.front_register();
				abstract_astnode::Generator.insert_code(OpBinary::chooseOperation(PUSH_OP, exp_ast_2->inherent_dt), reg2);
				string val = exp_ast_1->get_address();
				string reg1 = abstract_astnode::Generator.get_register();
				reg2 = abstract_astnode::Generator.front_register();
				abstract_astnode::Generator.insert_code(OpBinary::chooseOperation(LOAD_OP, exp_ast_2->inherent_dt),abstract_astnode::Generator.mem_offset("esp"),reg2);
				abstract_astnode::Generator.insert_code(OpBinary::chooseOperation(POP_OP, exp_ast_2->inherent_dt), 1);
				abstract_astnode::Generator.insert_code(OpBinary::chooseOperation(STORE_OP, exp_ast_2->inherent_dt),reg2,val);
				abstract_astnode::Generator.put_register(reg1);
				abstract_astnode::Generator.swap();	
			}
		}
		
	}
	void Ass::setLabel(int argument_number){
		if(!isEmpty){
			exp_ast_1->setLabel(0);
			exp_ast_2->setLabel(0);
		}
	}
	void Ass::print(int tabs){
		print_tabs(tabs);
		if(isEmpty){
			cout<<"(Empty)";
		}else{
			cout<<"(Ass ";
			exp_ast_1->print(tabs+1);
			exp_ast_2->print(tabs+1);
			cout<<")";
		}
	}
	//Type checking for return depends on the return type of the function. 
	Return::Return(ExpAst *exp_ast_a,mDataType data):exp_ast_1(exp_ast_a){
		setType(RETURN_AST);
		
		if(exp_ast_1->inherent_dt != DT_ERROR && data.dt != DT_ERROR &&
			are_compatible_datatypes(exp_ast_1->inherent_dt,data.dt) == DT_ERROR){
			inherent_dt = DT_ARRAY;
			//JUST TO HANDLE THIS CASE, OTHER WISE ARRAYS ARE NOT POSSIBLE
		}else if(exp_ast_1->inherent_dt == DT_ERROR ||
			are_compatible_datatypes(exp_ast_1->inherent_dt,data.dt) == DT_ERROR)
			inherent_dt = DT_ERROR;
		else{
			inherent_dt = DT_VOID;

			if(exp_ast_1->inherent_dt != data.dt && !is_constant){
				operators op = get_cast(data.dt);
				exp_ast_1 = new OpUnary(exp_ast_1,op);
			}
		}
	}
	void Return::gen_code (int mode){
		abstract_astnode::Generator.add_comment(" RETURN block");
		OpBinary::genCode(exp_ast_1);
		Seq::return_call();
		string reg = abstract_astnode::Generator.front_register();
		if(exp_ast_1->inherent_dt == DT_FLOAT)
			abstract_astnode::Generator.insert_code(MF_PUSH_F,reg);
		if(exp_ast_1->inherent_dt == DT_INT)
			abstract_astnode::Generator.insert_code(MF_PUSH_I,reg);
		abstract_astnode::Generator.insert_code("return;");
	}
	void Return::setLabel(int argument_number){
		exp_ast_1->setLabel(0);
	}

	void Return::print(int tabs){
		print_tabs(tabs);
		cout<<"(Return ";
		exp_ast_1->print(tabs+1);
		cout<<")";
	}

	//An if statement is erroneous if any of the constituents are erroneous
	If::If(ExpAst *exp_ast_a,
		StmtAst *stmt_ast_b,
		StmtAst *stmt_ast_c):exp_ast_1(exp_ast_a), stmt_ast_2(stmt_ast_b), stmt_ast_3(stmt_ast_c){
		setType(IF_AST);

		if(exp_ast_1->inherent_dt == DT_ERROR || 
			exp_ast_1->inherent_dt == DT_VOID || 
			stmt_ast_2->inherent_dt == DT_ERROR ||
			stmt_ast_3->inherent_dt == DT_ERROR)
			inherent_dt = DT_ERROR;
		else{
			inherent_dt = DT_VOID;
			// exp_ast_1 = new OpUnary(exp_ast_a,BOOLEAN_CAST);
		}
	}
	void If::gen_code (int mode){ // Done with nextlist
		abstract_astnode::Generator.add_comment(" IF block");

		if(stmt_ast_2->isEmpty && stmt_ast_3->isEmpty && exp_ast_1->has_assignment){
			OpBinary::genCode(exp_ast_1);
			nextlist = exp_ast_1->nextlist;
		}else if(stmt_ast_2->isEmpty && !stmt_ast_3->isEmpty){
			exp_ast_1->fall_through = false;
			OpBinary::genCodeBool(exp_ast_1);
			abstract_astnode::Generator.backpatch(exp_ast_1->falselist, abstract_astnode::Generator.nextinstr());
			stmt_ast_3->gen_code();
			//abstract_astnode::Generator.backpatch(exp_ast_1->truelist, abstract_astnode::Generator.nextinstr());
			(stmt_ast_3->nextlist).splice((stmt_ast_3->nextlist).end(), exp_ast_1->truelist);//Aliter
			nextlist = stmt_ast_3->nextlist;
			/*//if(reg) goto L
			string reg = abstract_astnode::Generator.get_register();
			pair<int,int> if_else_code = abstract_astnode::Generator.boolean_code(reg,MODE_IF_GOTO_ELSE_FALL,1);
			abstract_astnode::Generator.put_register(reg);
			//else ka code
			abstract_astnode::Generator.add_comment("ELSE code");
			stmt_ast_3->gen_code();
			//L:
			abstract_astnode::Generator.backpatch(if_else_code.first,abstract_astnode::Generator.nextinstr());
			*/

		}else if(!stmt_ast_2->isEmpty && stmt_ast_3->isEmpty){
			exp_ast_1->fall_through = true;
			OpBinary::genCodeBool(exp_ast_1);
			abstract_astnode::Generator.backpatch(exp_ast_1->truelist, abstract_astnode::Generator.nextinstr());
			stmt_ast_2->gen_code();
			//abstract_astnode::Generator.backpatch(exp_ast_1->falselist, abstract_astnode::Generator.nextinstr());
			(stmt_ast_2->nextlist).splice((stmt_ast_2->nextlist).end(), exp_ast_1->falselist);//Aliter
			nextlist = stmt_ast_2->nextlist;
			/*exp_ast_1->gen_code();
			//if(!reg) goto L
			string reg = abstract_astnode::Generator.get_register();
			pair<int,int> if_else_code = abstract_astnode::Generator.boolean_code(reg,MODE_IFNOT_GOTO_ELSE_FALL,1);
			abstract_astnode::Generator.put_register(reg);
			//if ka code
			abstract_astnode::Generator.add_comment("IF code");
			stmt_ast_2->gen_code();
			//L:
			abstract_astnode::Generator.backpatch(if_else_code.first,abstract_astnode::Generator.nextinstr());*/
		}else if(!stmt_ast_2->isEmpty && !stmt_ast_3->isEmpty){
			exp_ast_1->fall_through = true;
			OpBinary::genCodeBool(exp_ast_1);
			abstract_astnode::Generator.backpatch(exp_ast_1->truelist, abstract_astnode::Generator.nextinstr());
			stmt_ast_2->gen_code();
			int goOut_pos = abstract_astnode::Generator.insert_code(MF_J,"_");
			abstract_astnode::Generator.backpatch(exp_ast_1->falselist, abstract_astnode::Generator.nextinstr());
			stmt_ast_3->gen_code();
			//backpatch(goOut_pos, abstract_astnode::Generator.nextinstr());
			(stmt_ast_2->nextlist).splice((stmt_ast_2->nextlist).end(), stmt_ast_3->nextlist);//Aliter
			(stmt_ast_2->nextlist).push_back(goOut_pos);
			nextlist = stmt_ast_2->nextlist;
			
			/*//if(reg) goto L1
			string reg = abstract_astnode::Generator.get_register();
			pair<int,int> if_else_code = abstract_astnode::Generator.boolean_code(reg,MODE_IF_GOTO_ELSE_FALL,1);
			abstract_astnode::Generator.put_register(reg);
			//else ka code
			abstract_astnode::Generator.add_comment("ELSE code");
			stmt_ast_3->gen_code();
			//goto L
			int else_pos = abstract_astnode::Generator.insert_code(MF_J,"_");
			//L1: if ka code
			abstract_astnode::Generator.add_comment("IF code");
			int label = abstract_astnode::Generator.add_label(abstract_astnode::Generator.nextinstr());
			stmt_ast_2->gen_code();
			abstract_astnode::Generator.backpatch(if_else_code.first,label);			
			//L
			abstract_astnode::Generator.backpatch(else_pos,abstract_astnode::Generator.nextinstr());
			*/
		}
	}
	void If::print(int tabs){
		print_tabs(tabs);
		cout<<"(If ";
		exp_ast_1->print(tabs+1);
		stmt_ast_2->print(tabs+1);
		stmt_ast_3->print(tabs+1);
		cout<<")";
	}
	void If::setLabel(int argument_number){
		exp_ast_1->setLabel(0);
		stmt_ast_2->setLabel(0);
		stmt_ast_3->setLabel(0);
	}

	//A while statement is erroneous if any of the constituents are erroneous
	While::While(ExpAst *exp_ast_a,StmtAst *stmt_ast_b):exp_ast_1(exp_ast_a), stmt_ast_2(stmt_ast_b){
		setType(WHILE_AST);

		if(exp_ast_1->inherent_dt == DT_ERROR ||
			exp_ast_1->inherent_dt == DT_VOID || 
			stmt_ast_2->inherent_dt == DT_ERROR)
			inherent_dt = DT_ERROR;
		else{
			inherent_dt = DT_VOID;
			// exp_ast_1 = new OpUnary(exp_ast_a,BOOLEAN_CAST);
		}
	}
	void While::gen_code (int mode){ // Done with nextlist
		abstract_astnode::Generator.add_comment(" WHILE block");
		//L: calculate reg
		int label = abstract_astnode::Generator.add_label(abstract_astnode::Generator.nextinstr());
		exp_ast_1->fall_through = true;
		OpBinary::genCodeBool(exp_ast_1);
		abstract_astnode::Generator.backpatch(exp_ast_1->truelist,abstract_astnode::Generator.nextinstr());
		stmt_ast_2->gen_code();
		int redo_pos = abstract_astnode::Generator.insert_code(MF_J,"_");
		abstract_astnode::Generator.backpatch(redo_pos,label);	
		abstract_astnode::Generator.backpatch(stmt_ast_2->nextlist, label);		
		//abstract_astnode::Generator.backpatch(exp_ast_1->falselist,abstract_astnode::Generator.nextinstr());
		nextlist = exp_ast_1->falselist;

		/*//if(!reg) goto L1
		string reg = abstract_astnode::Generator.get_register();
		pair<int,int> if_else_code = abstract_astnode::Generator.boolean_code(reg,MODE_IFNOT_GOTO_ELSE_FALL,1);
		abstract_astnode::Generator.put_register(reg);
		//code in while
		stmt_ast_2->gen_code();
		//goto L
		int redo_pos = abstract_astnode::Generator.insert_code(MF_J,"_");
		//L1:
		int next_label = abstract_astnode::Generator.add_label(abstract_astnode::Generator.nextinstr());

		abstract_astnode::Generator.backpatch(redo_pos,label);			
		*/
		
	}	
	void While::print(int tabs){
		print_tabs(tabs);
		cout<<"(While ";
		exp_ast_1->print(tabs+1);
		stmt_ast_2->print(tabs+1);
		cout<<")";
	}
	void While::setLabel(int argument_number){
		exp_ast_1->setLabel(0);
		stmt_ast_2->setLabel(0);
	}

	//A for statement is erroneous if any of the constituents are erroneous
	For::For(ExpAst *e1,ExpAst *e2,ExpAst *e3, 
		StmtAst *s4):exp_ast_1(e1), 
		exp_ast_2(e2), exp_ast_3(e3), 
		stmt_ast_4(s4){
		setType(FOR_AST);

		if(exp_ast_1->inherent_dt == DT_ERROR ||
			exp_ast_2->inherent_dt == DT_ERROR ||
			exp_ast_2->inherent_dt == DT_VOID || 
			exp_ast_3->inherent_dt == DT_ERROR ||
			stmt_ast_4->inherent_dt == DT_ERROR)
			inherent_dt = DT_ERROR;
		else{
			inherent_dt = DT_VOID;
			// exp_ast_2 = new OpUnary(e2,BOOLEAN_CAST);
		}
	}
	void For::gen_code (int mode){ // Done with nextlist
		abstract_astnode::Generator.add_comment(" FOR block");
		//calculate first
		OpBinary::genCode(exp_ast_1);
		int label = abstract_astnode::Generator.add_label(abstract_astnode::Generator.nextinstr());
		abstract_astnode::Generator.backpatch(exp_ast_1->nextlist, label);
		exp_ast_2->fall_through = true;
		OpBinary::genCodeBool(exp_ast_2);
		abstract_astnode::Generator.backpatch(exp_ast_2->truelist,abstract_astnode::Generator.nextinstr());			
		stmt_ast_4->gen_code();
		abstract_astnode::Generator.backpatch(stmt_ast_4->nextlist, abstract_astnode::Generator.nextinstr());
		OpBinary::genCode(exp_ast_3);
		abstract_astnode::Generator.backpatch(exp_ast_3->nextlist, label);
		int redo_pos = abstract_astnode::Generator.insert_code(MF_J,"_");
		abstract_astnode::Generator.backpatch(redo_pos,label);			
		//abstract_astnode::Generator.backpatch(exp_ast_2->falselist,abstract_astnode::Generator.nextinstr());			
		nextlist = exp_ast_2->falselist; //Aliter
		/*//L: calculate reg
		int label = abstract_astnode::Generator.add_label(abstract_astnode::Generator.nextinstr());
		exp_ast_2->gen_code();
		//if(!reg) goto L1
		string reg = abstract_astnode::Generator.get_register();
		pair<int,int> if_else_code = abstract_astnode::Generator.boolean_code(reg,MODE_IFNOT_GOTO_ELSE_FALL,1);
		abstract_astnode::Generator.put_register(reg);
		//code in while
		stmt_ast_4->gen_code();
		//code for e3
		exp_ast_3->gen_code();
		//goto L
		int redo_pos = abstract_astnode::Generator.insert_code(MF_J,"_");
		//L1:
		int next_label = abstract_astnode::Generator.add_label(abstract_astnode::Generator.nextinstr());

		abstract_astnode::Generator.backpatch(redo_pos,label);			
		abstract_astnode::Generator.backpatch(if_else_code.first,next_label);
		;*/
	}
	void For::print(int tabs){
		print_tabs(tabs);
		cout<<"(For ";
		exp_ast_1->print(tabs+1);
		exp_ast_2->print(tabs+1);
		exp_ast_3->print(tabs+1);
		stmt_ast_4->print(tabs+1);
		cout<<")";
	}
	void For::setLabel(int argument_number){
		exp_ast_1->setLabel(0);
		exp_ast_2->setLabel(0);
		exp_ast_3->setLabel(0);
		stmt_ast_4->setLabel(0);
	}

	//Things done diffrently for &&, || ,= (ass_exp) and <
	OpBinary::OpBinary(ExpAst *exp_ast_a,operators o,ExpAst *exp_ast_b):exp_ast_1(exp_ast_a), exp_ast_2(exp_ast_b), op(o){
		setType(OPb_AST);
		is_boolean = false;
		if(op == OR_OPERATION || op == AND_OPERATION) is_boolean = true;
		if(exp_ast_1->has_assignment || exp_ast_2->has_assignment)
				has_assignment = true;

		if(exp_ast_1->inherent_dt == DT_ERROR ||
			exp_ast_2->inherent_dt == DT_ERROR || 
			are_compatible_datatypes(exp_ast_1->inherent_dt,exp_ast_2->inherent_dt) == DT_ERROR)
			inherent_dt = DT_ERROR;
		else if(op == ASSIGN_OPERATION){
			is_assignment = true;
			has_assignment = true;
			inherent_dt = exp_ast_1->inherent_dt;
			if(exp_ast_1->inherent_dt != exp_ast_2->inherent_dt && !is_constant){
				operators op = get_cast(exp_ast_1->inherent_dt); 
				//Casting applied if for ass lhs.type is != to RHS.type 
				exp_ast_2 = new OpUnary(exp_ast_2,op);
			}
		}
		else{
			inherent_dt = are_compatible_datatypes(exp_ast_1->inherent_dt,exp_ast_2->inherent_dt);
			if(exp_ast_1->inherent_dt != exp_ast_2->inherent_dt){
				operators op = get_cast(inherent_dt);
				if(exp_ast_1->inherent_dt != inherent_dt && !is_constant)
					exp_ast_1 = new OpUnary(exp_ast_1,op);
				else if(exp_ast_2->inherent_dt != inherent_dt && !is_constant)
					exp_ast_2 = new OpUnary(exp_ast_2,op);
			}
		}
		optimizeOrder();
	}

	void OpBinary::optimizeOrder(){
		if(op == SUB_OPERATION){
			op = PLUS_OPERATION;
			if(exp_ast_2->is_constant) {
				if(exp_ast_2->getType() ==  INT_CONSTANT_AST) {
					int value2 = -1 * ((IntConstant*)exp_ast_2)->get_value();
					((IntConstant*)exp_ast_2)->set_value(value2);
				}
				else if(exp_ast_2->getType() ==  FLOAT_CONSTANT_AST) {
					float value2 = -1 * ((FloatConstant*)exp_ast_2)->get_value();
					((FloatConstant*)exp_ast_2)->set_value(value2);
				}
			}
			else exp_ast_2 = new OpUnary(exp_ast_2,MINUS_OPERATION);
		}
		if(exp_ast_2->is_constant){
			ExpAst* tmp_exp;
			bool switch_operands = false;
			if(op == PLUS_OPERATION  || op == MULT_OPERATION
				||op == NE_OPERATION || op == EQ_OPERATION){
				switch_operands = true;
			}else if(op == GE_OPERATION){
				op = LE_OPERATION;
				switch_operands = true;
			}else if(op == LE_OPERATION){
				op = GE_OPERATION;
				switch_operands = true;
			}else if(op == LT_OPERATION){
				op = GT_OPERATION;
				switch_operands = true;
			}else if(op == GT_OPERATION){
				op = LT_OPERATION;
				//abstract_astnode::Generator.add_comment("Hell NO");
				switch_operands = true;
			}

			if(switch_operands){
				exp_ast_2->min_register = 0;
				tmp_exp = exp_ast_1;
				exp_ast_1 = exp_ast_2;
				exp_ast_2 = tmp_exp;
			}
		}
	}
	machine_function OpBinary::chooseOperation(int op, int type) {
		if(type == DT_INT) {
			if(op == PLUS_OPERATION) return MF_ADD_I;
			if(op == MULT_OPERATION) return MF_MULT_I;
			if(op == DIV_OPERATION) return MF_DIV_I;
			if(op == PUSH_OP) return MF_PUSH_I;
			if(op == POP_OP) return MF_POP_I;
			if(op == LOAD_OP) return MF_LOAD_I;
			if(op == STORE_OP) return MF_STORE_I;
			return MF_COMP_I;
		}
		if(type == DT_FLOAT) {
			if(op == PLUS_OPERATION) return MF_ADD_F;
			if(op == MULT_OPERATION) return MF_MULT_F;
			if(op == DIV_OPERATION) return MF_DIV_F;
			if(op == PUSH_OP) return MF_PUSH_F;
			if(op == POP_OP) return MF_POP_F;
			if(op == LOAD_OP) return MF_LOAD_F;
			if(op == STORE_OP) return MF_STORE_F;
			return MF_COMP_F;
		}
	}

	void OpBinary::genCode(ExpAst* exp_ast, bool fall) { //Done with Nextlist
		exp_ast->fall_through = fall;
		exp_ast->gen_code();
		if(exp_ast->is_boolean){
				string reg = abstract_astnode::Generator.front_register();
				int locT, locM, locF;
				abstract_astnode::Generator.backpatch(exp_ast->nextlist, abstract_astnode::Generator.nextinstr());
				(exp_ast->nextlist).clear();
				if(exp_ast->fall_through) {
					locT = abstract_astnode::Generator.insert_code(MF_MOVE,1,reg);
					locM = abstract_astnode::Generator.insert_code(MF_J, "_");
					locF = abstract_astnode::Generator.insert_code(MF_MOVE,0,reg);
				}
				else {
					locF = abstract_astnode::Generator.insert_code(MF_MOVE,0,reg);	
					locM = abstract_astnode::Generator.insert_code(MF_J, "_");
					locT = abstract_astnode::Generator.insert_code(MF_MOVE,1,reg);
				}
				//abstract_astnode::Generator.backpatch(locM, abstract_astnode::Generator.nextinstr());
				(exp_ast->nextlist).push_back(locM);//Aliter
				abstract_astnode::Generator.backpatch(exp_ast->truelist, locT);
				abstract_astnode::Generator.backpatch(exp_ast->falselist, locF);
				exp_ast->truelist.clear();
				exp_ast->falselist.clear();
				exp_ast->is_boolean = false;
		}
	}

	void OpBinary::genCodeBool(ExpAst* exp_ast) { // Done with next list
		exp_ast->gen_code(1);
		if(!(exp_ast->is_boolean)) {
			string reg = abstract_astnode::Generator.front_register();
			int loc;
			abstract_astnode::Generator.backpatch(exp_ast->nextlist, abstract_astnode::Generator.nextinstr());
			(exp_ast->nextlist).clear();
			if(exp_ast->inherent_dt == DT_FLOAT) abstract_astnode::Generator.insert_code(MF_COMP_F,0,reg);
			else abstract_astnode::Generator.insert_code(MF_COMP_I,0,reg);
			if(exp_ast->fall_through) {
				loc = abstract_astnode::Generator.insert_code(MF_JE, "_");
				(exp_ast->falselist).push_back(loc);
			}
			else {
				loc = abstract_astnode::Generator.insert_code(MF_JNE, "_");
				(exp_ast->truelist).push_back(loc);
			}
			exp_ast->is_boolean = true;  
		}
	}

	void OpBinary::compHelper(int code, machine_function J1, machine_function J2, 
			machine_function J3) { // Done with Nextlist
		if(code == 0) {
			int loc1 = abstract_astnode::Generator.insert_code(J1,"_");
			string reg = abstract_astnode::Generator.front_register();	
			abstract_astnode::Generator.insert_code(MF_MOVE,0,reg);
			int loc2 = abstract_astnode::Generator.insert_code(MF_J,"_");
			abstract_astnode::Generator.backpatch(loc1, abstract_astnode::Generator.nextinstr());
			abstract_astnode::Generator.insert_code(MF_MOVE,1,reg);
			//abstract_astnode::Generator.backpatch(loc2, abstract_astnode::Generator.nextinstr());
			nextlist.push_back(loc2);//Aliter
		}
		else {
			if(fall_through) {
				//abstract_astnode::Generator.add_comment("HELLO TRUE");
				int loc = abstract_astnode::Generator.insert_code(J2,"_");
				is_boolean = true;
				falselist.push_back(loc);
			}
			else {
				//abstract_astnode::Generator.add_comment("HELLO False");
				int loc = abstract_astnode::Generator.insert_code(J3,"_");
				is_boolean = true;
				truelist.push_back(loc);
			}
		}
	}

	string noToStr(int i) {
		switch(i) {
			case 0: return "zero";
			case 1: return "one";
			case 2: return "two";
			case 3: return "three";
			case 4: return "four";
			case 5: return "five";
			default: return "six+";
		}
	}


	void OpBinary::gen_code(int code){ //done with nextlist
		abstract_astnode::Generator.add_comment(" OP BINARY block : " + abstract_astnode::getOperatorStr(op));
		abstract_astnode::Generator.add_comment(noToStr(min_register) + " : Label");
		if(NE_OPERATION <= op && op <= DIV_OPERATION) {
			if(exp_ast_1->getType() ==  INT_CONSTANT_AST) {
				genCode(exp_ast_2);
				abstract_astnode::Generator.backpatch(exp_ast_2->nextlist, abstract_astnode::Generator.nextinstr());
				string reg2 = abstract_astnode::Generator.front_register();
				int value1 = ((IntConstant*)exp_ast_1)->get_value();
				abstract_astnode::Generator.insert_code(chooseOperation(op, exp_ast_2->inherent_dt),value1,reg2);
			}
			else if(exp_ast_1->getType() ==  FLOAT_CONSTANT_AST) {
				genCode(exp_ast_2);
				abstract_astnode::Generator.backpatch(exp_ast_2->nextlist, abstract_astnode::Generator.nextinstr());
				string reg2 = abstract_astnode::Generator.front_register();
				float value1 = ((FloatConstant*)exp_ast_1)->get_value();
				abstract_astnode::Generator.insert_code(chooseOperation(op, exp_ast_2->inherent_dt),value1,reg2);
			}
			else if(exp_ast_1->min_register < 4 && !exp_ast_1->has_assignment && !exp_ast_2->has_assignment  && (exp_ast_1->min_register < exp_ast_2->min_register)){
				genCode(exp_ast_2);
				abstract_astnode::Generator.backpatch(exp_ast_2->nextlist, abstract_astnode::Generator.nextinstr());
				string reg2 = abstract_astnode::Generator.get_register();
				genCode(exp_ast_1);
				abstract_astnode::Generator.backpatch(exp_ast_1->nextlist, abstract_astnode::Generator.nextinstr());
				abstract_astnode::Generator.insert_code(chooseOperation(op, exp_ast_2->inherent_dt),abstract_astnode::Generator.front_register(),reg2);
				abstract_astnode::Generator.put_register(reg2);
			}
			else if(exp_ast_2->min_register < abstract_astnode::Generator.get_size()) {
				genCode(exp_ast_1);
				abstract_astnode::Generator.backpatch(exp_ast_1->nextlist, abstract_astnode::Generator.nextinstr());
				string reg1 = abstract_astnode::Generator.get_register();
				genCode(exp_ast_2);
				abstract_astnode::Generator.backpatch(exp_ast_2->nextlist, abstract_astnode::Generator.nextinstr());
				abstract_astnode::Generator.insert_code(chooseOperation(op, exp_ast_2->inherent_dt),reg1,abstract_astnode::Generator.front_register());
				abstract_astnode::Generator.put_register(reg1);
				abstract_astnode::Generator.swap();
			}
			else {
				genCode(exp_ast_1);
				abstract_astnode::Generator.backpatch(exp_ast_1->nextlist, abstract_astnode::Generator.nextinstr());
				string reg1 = abstract_astnode::Generator.front_register();
				//abstract_astnode::Generator.add_comment("WHAT");				
				abstract_astnode::Generator.insert_code(chooseOperation(PUSH_OP, exp_ast_2->inherent_dt),reg1);
				genCode(exp_ast_2);
				abstract_astnode::Generator.backpatch(exp_ast_2->nextlist, abstract_astnode::Generator.nextinstr());
				string reg2 = abstract_astnode::Generator.get_register();
				reg1 = abstract_astnode::Generator.front_register();
				abstract_astnode::Generator.insert_code(chooseOperation(LOAD_OP, exp_ast_2->inherent_dt),abstract_astnode::Generator.mem_offset("esp"),reg1);
				abstract_astnode::Generator.insert_code(chooseOperation(POP_OP, exp_ast_2->inherent_dt), 1);
				abstract_astnode::Generator.insert_code(chooseOperation(op, exp_ast_2->inherent_dt),reg1,reg2);
				abstract_astnode::Generator.put_register(reg2);
			}
			if(op == NE_OPERATION) compHelper(code, MF_JNE, MF_JE, MF_JNE);
			else if(op == EQ_OPERATION) compHelper(code, MF_JE, MF_JNE, MF_JE);
			else if(op == GT_OPERATION) compHelper(code, MF_JG, MF_JLE, MF_JG);
			else if(op == LT_OPERATION) compHelper(code, MF_JL, MF_JGE, MF_JL);
			else if(op == GE_OPERATION) compHelper(code, MF_JGE, MF_JL, MF_JGE);
			else if(op == LE_OPERATION) compHelper(code, MF_JLE, MF_JG, MF_JLE);
		}
		else if(op == OR_OPERATION) {
			exp_ast_1->fall_through = false;
			genCodeBool(exp_ast_1);
			//abstract_astnode::Generator.backpatch(exp_ast_1->nextlist, abstract_astnode::Generator.nextinstr());
			exp_ast_2->fall_through = fall_through;
			abstract_astnode::Generator.backpatch((exp_ast_1->falselist), abstract_astnode::Generator.nextinstr());
			genCodeBool(exp_ast_2);
			(exp_ast_1->truelist).splice((exp_ast_1->truelist).end(), (exp_ast_2->truelist));
			//nextlist = exp_ast_2->nextlist;
			truelist = (exp_ast_1->truelist);
			falselist = (exp_ast_2->falselist);
		}
		else if(op == AND_OPERATION) {
			exp_ast_1->fall_through = true;
			genCodeBool(exp_ast_1);
			//abstract_astnode::Generator.backpatch(exp_ast_1->nextlist, abstract_astnode::Generator.nextinstr());
			exp_ast_2->fall_through = fall_through;
			abstract_astnode::Generator.backpatch((exp_ast_1->truelist), abstract_astnode::Generator.nextinstr());
			genCodeBool(exp_ast_2);
			(exp_ast_1->falselist).splice((exp_ast_1->falselist).end(), (exp_ast_2->falselist));
			//nextlist = exp_ast_2->nextlist;
			truelist = (exp_ast_2->truelist);
			falselist = (exp_ast_1->falselist);
		}
		else if(op == ASSIGN_OPERATION){
			if(exp_ast_2->getType() ==  INT_CONSTANT_AST) {
				int value2 = ((IntConstant*)exp_ast_2)->get_value();
				abstract_astnode::Generator.insert_code(MF_STORE_I,value2,exp_ast_1->get_address());
				string reg = abstract_astnode::Generator.front_register();
				abstract_astnode::Generator.insert_code(MF_MOVE,value2, reg);
			}
			else if(exp_ast_2->getType() ==  FLOAT_CONSTANT_AST){
				float value2 = ((FloatConstant*)exp_ast_2)->get_value();
				abstract_astnode::Generator.insert_code(MF_STORE_F,value2,exp_ast_1->get_address());
				string reg = abstract_astnode::Generator.front_register();
				abstract_astnode::Generator.insert_code(MF_MOVE,value2, reg);
			}
			else if(exp_ast_1->min_register < 4 && (exp_ast_1->min_register < exp_ast_2->min_register)){
				genCode(exp_ast_2);
				abstract_astnode::Generator.backpatch(exp_ast_2->nextlist, abstract_astnode::Generator.nextinstr());
				string reg = abstract_astnode::Generator.get_register();
				abstract_astnode::Generator.insert_code(chooseOperation(STORE_OP, exp_ast_2->inherent_dt),reg,exp_ast_1->get_address());
				abstract_astnode::Generator.put_register(reg);
			}
			else if(exp_ast_2->min_register < abstract_astnode::Generator.get_size() && !exp_ast_1->has_assignment && !exp_ast_2->has_assignment) {
				string val = exp_ast_1->get_address();
				string reg = abstract_astnode::Generator.get_register();
				genCode(exp_ast_2);
				abstract_astnode::Generator.backpatch(exp_ast_2->nextlist, abstract_astnode::Generator.nextinstr());
				abstract_astnode::Generator.insert_code(chooseOperation(STORE_OP, exp_ast_2->inherent_dt),abstract_astnode::Generator.front_register(),val);
				abstract_astnode::Generator.put_register(reg);
				abstract_astnode::Generator.swap();
			}
			else {
				genCode(exp_ast_2);
				//abstract_astnode::Generator.add_comment("WHAT");
				abstract_astnode::Generator.backpatch(exp_ast_2->nextlist, abstract_astnode::Generator.nextinstr());
				string reg2 = abstract_astnode::Generator.front_register();
				abstract_astnode::Generator.insert_code(chooseOperation(PUSH_OP, exp_ast_2->inherent_dt), reg2);
				string val = exp_ast_1->get_address();
				string reg1 = abstract_astnode::Generator.get_register();
				reg2 = abstract_astnode::Generator.front_register();
				abstract_astnode::Generator.insert_code(chooseOperation(LOAD_OP, exp_ast_2->inherent_dt),abstract_astnode::Generator.mem_offset("esp"),reg2);
				abstract_astnode::Generator.insert_code(chooseOperation(POP_OP, exp_ast_2->inherent_dt), 1);
				abstract_astnode::Generator.insert_code(chooseOperation(STORE_OP, exp_ast_2->inherent_dt),reg2,val);
				abstract_astnode::Generator.put_register(reg1);
				abstract_astnode::Generator.swap();	
			}
		}
	}


	void OpBinary::print(int tabs){
		print_tabs(tabs);
		cout<<"("<<getOperatorStr(op)<<" ";
		exp_ast_1->print(tabs+1);
		exp_ast_2->print(tabs+1);
		cout<<")";
	}
	void OpBinary::setLabel(int argument){
		is_leaf = false;
		if(NE_OPERATION <= op && op <= DIV_OPERATION && (exp_ast_1->is_constant)) exp_ast_1->setLabel(1);
		else exp_ast_1->setLabel(0);
		if(op == ASSIGN_OPERATION && (exp_ast_2->is_constant)) exp_ast_2->setLabel(1);
		else exp_ast_2->setLabel(0);
		min_register = abstract_astnode::get_new_label(exp_ast_1->min_register,
			exp_ast_2->min_register);
		if(op == AND_OPERATION || op == OR_OPERATION) {
			if(exp_ast_1->min_register == exp_ast_2->min_register) min_register -=1;
		} 
	}
	OpUnary::OpUnary(ExpAst *exp_ast_a,operators o):exp_ast(exp_ast_a), op(o){
		setType(OPu_AST);
		inherent_dt = exp_ast->inherent_dt;
		if(op == FLOAT_CAST) inherent_dt = DT_FLOAT;
		else if(op == INT_CAST) inherent_dt = DT_INT;
		has_assignment = exp_ast->has_assignment;
		if(op == BANG_OPERATION) {
			inherent_dt = DT_INT;
			is_boolean = true;
		}
		else if(op == INC_OPERATOR) has_assignment = true;
	}
	void OpUnary::print(int tabs){
		print_tabs(tabs);
		cout<<"("<<getOperatorStr(op)<<" ";
		exp_ast->print(tabs+1);
		cout<<")";
	}
	void OpUnary::setLabel(int argument){
		is_leaf = false;
		exp_ast->setLabel(0);
		min_register = exp_ast->min_register;
	}
	void OpUnary::gen_code(int mode){ // Done with nextlist
		abstract_astnode::Generator.add_comment(" OP UNARY block : " + abstract_astnode::getOperatorStr(op));
		
		if(op == INC_OPERATOR){
			string reg = abstract_astnode::Generator.get_register();
			
			if(exp_ast->inherent_dt == DT_FLOAT){
				abstract_astnode::Generator.insert_code(MF_LOAD_F,exp_ast->get_address(),reg);
				abstract_astnode::Generator.insert_code(MF_ADD_F,1.0,reg);
				abstract_astnode::Generator.insert_code(MF_STORE_F,reg,exp_ast->get_address());
				abstract_astnode::Generator.insert_code(MF_ADD_F,-1.0,reg);
			}else if(exp_ast->inherent_dt == DT_INT){
				abstract_astnode::Generator.insert_code(MF_LOAD_I,exp_ast->get_address(),reg);
				abstract_astnode::Generator.insert_code(MF_ADD_I,1,reg);
				abstract_astnode::Generator.insert_code(MF_STORE_I,reg,exp_ast->get_address());
				abstract_astnode::Generator.insert_code(MF_ADD_I,-1,reg);
			}
			abstract_astnode::Generator.put_register(reg);
		}else if(op == BANG_OPERATION){
			exp_ast->fall_through = !fall_through;
			OpBinary::genCodeBool(exp_ast);
			/*string reg = abstract_astnode::Generator.get_register();
			if(exp_ast->inherent_dt == DT_FLOAT){
				abstract_astnode::Generator.make_boolean(reg,0);
				abstract_astnode::Generator.register_content[reg] = DT_INT;
			}else{
				abstract_astnode::Generator.make_boolean(reg,1);	
				abstract_astnode::Generator.register_content[reg] = DT_INT;
			}			
			abstract_astnode::Generator.put_register(reg);*/
			truelist = exp_ast->falselist;
			falselist = exp_ast->truelist;
		}else if(op == MINUS_OPERATION){
			string reg;

			if(exp_ast->getType() == FLOAT_CONSTANT_AST){
				float f_value = -1*((FloatConstant*)exp_ast)->get_value();
				reg = abstract_astnode::Generator.get_register();
				abstract_astnode::Generator.insert_code(MF_MOVE,f_value,reg);
			}
			else if(exp_ast->getType() == INT_CONSTANT_AST){
				int i_value = -1*((IntConstant*)exp_ast)->get_value();
				reg = abstract_astnode::Generator.get_register();
				abstract_astnode::Generator.insert_code(MF_MOVE,i_value,reg);
			}
			else{
				OpBinary::genCode(exp_ast);
				abstract_astnode::Generator.backpatch(exp_ast->nextlist, abstract_astnode::Generator.nextinstr());
				reg = abstract_astnode::Generator.get_register();
				if(exp_ast->inherent_dt == DT_FLOAT){
					abstract_astnode::Generator.insert_code(MF_MULT_F,-1.0,reg);
				}
				else if(exp_ast->inherent_dt == DT_INT){
					abstract_astnode::Generator.insert_code(MF_MULT_I,-1,reg);
				}
			}
			abstract_astnode::Generator.put_register(reg);
		}else if(op == FLOAT_CAST){
			string reg;
			if(exp_ast->getType() == INT_CONSTANT_AST){
				float f_value = (float) ((IntConstant*)exp_ast)->get_value();
				reg = abstract_astnode::Generator.get_register();
				abstract_astnode::Generator.insert_code(MF_MOVE,f_value,reg);
			}else{
				OpBinary::genCode(exp_ast);
				abstract_astnode::Generator.backpatch(exp_ast->nextlist, abstract_astnode::Generator.nextinstr());
				reg = abstract_astnode::Generator.get_register();
				abstract_astnode::Generator.insert_code(MF_INT_TO_FLOAT,reg);
			}
			abstract_astnode::Generator.put_register(reg);
			is_boolean = exp_ast->is_boolean;
		}else if(op == INT_CAST){
			string reg;
			if(exp_ast->getType() == FLOAT_CONSTANT_AST){
				int f_value = (int) ((FloatConstant*)exp_ast)->get_value();
				reg = abstract_astnode::Generator.get_register();
				abstract_astnode::Generator.insert_code(MF_MOVE,f_value,reg);
			}else{
				OpBinary::genCode(exp_ast);
				abstract_astnode::Generator.backpatch(exp_ast->nextlist, abstract_astnode::Generator.nextinstr());
				reg = abstract_astnode::Generator.get_register();
				abstract_astnode::Generator.insert_code(MF_FLOAT_TO_INT,reg);
			}
			abstract_astnode::Generator.put_register(reg);
			is_boolean = exp_ast->is_boolean;
		}
		
	}

	/*For function calls we need to fetch the parameter types and compare it with the sent parameters. 
		Casts are applied when required*/ 
		//Function statements are used when no assignments are made with the return value
	FuncallStmt::FuncallStmt(string fn, list<ExpAst*> lst,mDataType data, list<data_type> params,
		 	bool ignore_rule, int pos):list_exp_ast(lst), func_name(fn), position(pos){
		inherent_dt = data.dt;
		setType(FUNCALL_AST);
		if(!ignore_rule){
			list<data_type>::iterator di = params.begin();
			for(list<ExpAst*>::iterator li = list_exp_ast.begin(); li!=list_exp_ast.end();li++,di++){
				if((*li)->inherent_dt != *di  && !is_constant){
					operators op = get_cast(*di);
					*li = new OpUnary(*li,op);
				}
				if(*di == DT_ARRAY){
					has_assignment = true;
				}
			}
		}
	}
	FuncallStmt::FuncallStmt(string fn):func_name(fn){
		setType(FUNCALL_AST);
	}
	void FuncallStmt::gen_code (int mode){
				abstract_astnode::Generator.add_comment(" FUNCALL_STATEMENT block : " + func_name);
		//save registers....
		list<string> registers = abstract_astnode::Generator.get_current_registers();
		list<string> caller_saved = abstract_astnode::Generator.caller_saved_registers();
		map<string,data_type> mapping = abstract_astnode::Generator.register_content;

		for(list<string>::iterator li = caller_saved.begin(); li != caller_saved.end(); li++){
			if(mapping[*li] == DT_FLOAT){
				abstract_astnode::Generator.insert_code(MF_PUSH_F,*li);
			}
			else{
				abstract_astnode::Generator.insert_code(MF_PUSH_I,*li);
			}
			abstract_astnode::Generator.put_register(*li);
		}
		
		if(func_name == "printf"){
			//ADD PRINT COMMANDS IN MF INSTRUCTION SET
			list<data_type> lst_dt;
			for(list<ExpAst*>::iterator li = list_exp_ast.begin(); li != list_exp_ast.end(); li++){
				lst_dt.push_back((*li)->inherent_dt);
			}

			for(list<ExpAst*>::iterator li = list_exp_ast.begin(); li != list_exp_ast.end(); li++){
				if((*li)->is_constant){
					if((*li)->getType() == STRING_CONSTANT_AST){
						abstract_astnode::Generator.insert_code(MF_PRINT_STRING,((StringConstant*) (*li))->get_value());
					}
					else if((*li)->getType() == FLOAT_CONSTANT_AST){
						abstract_astnode::Generator.insert_code(MF_PRINT_FLOAT,((FloatConstant*) (*li))->get_value());
					}
					else if((*li)->getType() == INT_CONSTANT_AST){
						abstract_astnode::Generator.insert_code(MF_PRINT_INT,((IntConstant*) (*li))->get_value());
					}
				}else{
					OpBinary::genCode((*li));
					string out_reg = abstract_astnode::Generator.front_register();
					if((*li)->inherent_dt == DT_FLOAT){
						abstract_astnode::Generator.insert_code(MF_PRINT_FLOAT,out_reg);
					}
					if((*li)->inherent_dt == DT_INT){
						abstract_astnode::Generator.insert_code(MF_PRINT_INT,out_reg);
					}
				}
			}
		}else{
			
			list<ExpAst*>::iterator li = list_exp_ast.end();
			if(list_exp_ast.size() != 0){
				li--;
				for(;true;li--){
					OpBinary::genCode((*li));
					abstract_astnode::Generator.backpatch((*li)->nextlist, abstract_astnode::Generator.nextinstr());;
					string reg = abstract_astnode::Generator.front_register();
					if(abstract_astnode::Generator.register_content[reg] == DT_FLOAT)
						abstract_astnode::Generator.insert_code(MF_PUSH_F,reg);
					else
						abstract_astnode::Generator.insert_code(MF_PUSH_I,reg);

					if(li == list_exp_ast.begin())
						break;
				}
			}
			if(func_name != "main")
				abstract_astnode::Generator.insert_code("\t" + func_name + "_" + to_string(position) + "();");
			else
				abstract_astnode::Generator.insert_code("\t" + func_name + "();");

			//return value
			string reg_front = registers.front();
			reg_front = abstract_astnode::Generator.get_register(reg_front);
			if(inherent_dt == DT_FLOAT){
				abstract_astnode::Generator.insert_code(MF_LOAD_F,
					abstract_astnode::Generator.mem_offset("esp"),reg_front);
				abstract_astnode::Generator.insert_code(MF_POP_F,1);
			}else if(inherent_dt == DT_INT){
				abstract_astnode::Generator.insert_code(MF_LOAD_I,
					abstract_astnode::Generator.mem_offset("esp"),reg_front);
				abstract_astnode::Generator.insert_code(MF_POP_I,1);
			}
			abstract_astnode::Generator.put_register(reg_front);
		}

		list<string>::iterator li = caller_saved.end();
		if(caller_saved.size() != 0){
			li--;
			for(;true; li--){
				abstract_astnode::Generator.get_register(*li);
				if(mapping[*li] == DT_FLOAT){
					abstract_astnode::Generator.insert_code(MF_LOAD_F,
						abstract_astnode::Generator.mem_offset("esp"),*li);
					abstract_astnode::Generator.insert_code(MF_POP_F,1);
				}
				else{
					abstract_astnode::Generator.insert_code(MF_LOAD_I,
						abstract_astnode::Generator.mem_offset("esp"),*li);
					abstract_astnode::Generator.insert_code(MF_POP_I,1);
				}
				if(li == caller_saved.begin())
					break;
			}
		}

		

	}
	void FuncallStmt::setLabel(int arg){
		is_leaf = false;
		min_register = 1;
	}

	void FuncallStmt::print(int tabs){
		print_tabs(tabs);
		cout<<"(Funcall ";
		print_tabs(tabs+1);
		cout<<"(FunctionName "<<func_name<<")";
		print_tabs(tabs+1);
		cout<<"(Parameters [";
		for(list<ExpAst*>::iterator li = list_exp_ast.begin(); li!=list_exp_ast.end();){
			(*li)->print(tabs+2);
			li++;
		}
		cout<<"]))";
	}
	/* Function calls are handled in the same way as in FuncallStmt*/
	Funcall::Funcall(string fn, list<ExpAst*> lst,mDataType data, list<data_type> params,bool ignore_rule, int pos)
			:list_exp_ast(lst), func_name(fn), position(pos){
		inherent_dt = data.dt;
		setType(FUNCALL_AST);
		if(!ignore_rule){
			list<data_type>::iterator di = params.begin();
			for(list<ExpAst*>::iterator li = list_exp_ast.begin(); li!=list_exp_ast.end();li++,di++){
				if((*li)->inherent_dt != *di  && !is_constant){
					operators op = get_cast(*di);
					*li = new OpUnary(*li,op);
				}
				if(*di == DT_ARRAY){
					has_assignment = true;
				}
			}
		}
	}
	Funcall::Funcall(string fn):func_name(fn){
		setType(FUNCALL_AST);
	}
	void Funcall::gen_code (int mode){
						abstract_astnode::Generator.add_comment(" FUNCALL_STATEMENT block : " + func_name);
		//save registers....
		list<string> registers = abstract_astnode::Generator.get_current_registers();
		list<string> caller_saved = abstract_astnode::Generator.caller_saved_registers();
		map<string,data_type> mapping = abstract_astnode::Generator.register_content;

		for(list<string>::iterator li = caller_saved.begin(); li != caller_saved.end(); li++){
			if(mapping[*li] == DT_FLOAT){
				abstract_astnode::Generator.insert_code(MF_PUSH_F,*li);
			}
			else{
				abstract_astnode::Generator.insert_code(MF_PUSH_I,*li);
			}
			abstract_astnode::Generator.put_register(*li);
		}
		
		if(func_name == "printf"){
			//ADD PRINT COMMANDS IN MF INSTRUCTION SET
			for(list<ExpAst*>::iterator li = list_exp_ast.begin(); li != list_exp_ast.end(); li++){
				if((*li)->is_constant){
					if((*li)->getType() == STRING_CONSTANT_AST){
						abstract_astnode::Generator.insert_code(MF_PRINT_STRING,((StringConstant*) (*li))->get_value());
					}
					else if((*li)->getType() == FLOAT_CONSTANT_AST){
						abstract_astnode::Generator.insert_code(MF_PRINT_FLOAT,((FloatConstant*) (*li))->get_value());
					}
					else if((*li)->getType() == INT_CONSTANT_AST){
						abstract_astnode::Generator.insert_code(MF_PRINT_INT,((IntConstant*) (*li))->get_value());
					}
				}else{
					OpBinary::genCode((*li));
					string out_reg = abstract_astnode::Generator.front_register();
					if((*li)->inherent_dt == DT_FLOAT){
						abstract_astnode::Generator.insert_code(MF_PRINT_FLOAT,out_reg);
					}
					if((*li)->inherent_dt == DT_INT){
						abstract_astnode::Generator.insert_code(MF_PRINT_INT,out_reg);
					}
				}
			}
		}else{
			
			list<ExpAst*>::iterator li = list_exp_ast.end();
			if(list_exp_ast.size() != 0){
				li--;
				for(;true;li--){
					OpBinary::genCode((*li));
					abstract_astnode::Generator.backpatch((*li)->nextlist, abstract_astnode::Generator.nextinstr());;
					string reg = abstract_astnode::Generator.front_register();
					if(abstract_astnode::Generator.register_content[reg] == DT_FLOAT)
						abstract_astnode::Generator.insert_code(MF_PUSH_F,reg);
					else
						abstract_astnode::Generator.insert_code(MF_PUSH_I,reg);

					if(li == list_exp_ast.begin())
						break;
				}
			}
			if(func_name != "main")
				abstract_astnode::Generator.insert_code("\t" + func_name + "_" + to_string(position) + "();");
			else
				abstract_astnode::Generator.insert_code("\t" + func_name + "();");

			//return value
			string reg_front = registers.front();
			reg_front = abstract_astnode::Generator.get_register(reg_front);
			if(inherent_dt == DT_FLOAT){
				abstract_astnode::Generator.insert_code(MF_LOAD_F,
					abstract_astnode::Generator.mem_offset("esp"),reg_front);
				abstract_astnode::Generator.insert_code(MF_POP_F,1);
			}else if(inherent_dt == DT_INT){
				abstract_astnode::Generator.insert_code(MF_LOAD_I,
					abstract_astnode::Generator.mem_offset("esp"),reg_front);
				abstract_astnode::Generator.insert_code(MF_POP_I,1);
			}
			abstract_astnode::Generator.put_register(reg_front);
		}

		list<string>::iterator li = caller_saved.end();
		if(caller_saved.size() != 0){
			li--;
			for(;true; li--){
				abstract_astnode::Generator.get_register(*li);
				if(mapping[*li] == DT_FLOAT){
					abstract_astnode::Generator.insert_code(MF_LOAD_F,
						abstract_astnode::Generator.mem_offset("esp"),*li);
					abstract_astnode::Generator.insert_code(MF_POP_F,1);
				}
				else{
					abstract_astnode::Generator.insert_code(MF_LOAD_I,
						abstract_astnode::Generator.mem_offset("esp"),*li);
					abstract_astnode::Generator.insert_code(MF_POP_I,1);
				}
				if(li == caller_saved.begin())
					break;
			}
		}

	
	}
	void Funcall::setLabel(int arg){
		is_leaf = false;
		min_register = 1;
	}

	void Funcall::print(int tabs){
		print_tabs(tabs);
		cout<<"(Funcall ";
		print_tabs(tabs+1);
		cout<<"(FunctionName "<<func_name<<")";
		print_tabs(tabs+1);
		cout<<"(Parameters [";
		for(list<ExpAst*>::iterator li = list_exp_ast.begin(); li!=list_exp_ast.end();){
			(*li)->print(tabs+2);
			li++;
		}		
		cout<<"]))";
	}


	FloatConstant::FloatConstant(float fl): float_val(fl){
		is_constant = true;
		setType(FLOAT_CONSTANT_AST);
		inherent_dt = DT_FLOAT;
	}
	FloatConstant::FloatConstant(string fl){
		is_constant = true;
		setType(FLOAT_CONSTANT_AST);
		float_val = stof(fl);
		inherent_dt = DT_FLOAT;
	}
	void FloatConstant::print(int tabs){
		print_tabs(tabs);
		cout<<"(FloatConstant "<<float_val<<")";
	}
	void FloatConstant::gen_code (int mode){
		string reg = abstract_astnode::Generator.get_register();
		abstract_astnode::Generator.insert_code(MF_MOVE,float_val,reg);
		abstract_astnode::Generator.put_register(reg);;
	}
	void FloatConstant::setLabel(int arg){
		is_leaf = true;
		min_register = 1 - arg;
	}

	float FloatConstant::get_value(){
		return float_val;
	}

	void FloatConstant::set_value(float no){
		float_val = no;
	}
	IntConstant::IntConstant(int intv): int_val(intv){
		is_constant = true;
		setType(INT_CONSTANT_AST);
		inherent_dt = DT_INT;
	}
	IntConstant::IntConstant(string s){
		is_constant = true;
		setType(INT_CONSTANT_AST);
		int_val = stoi(s);
		inherent_dt = DT_INT;
	}
	int IntConstant::get_value(){
		return int_val;
	}

	void IntConstant::set_value(int no){
		int_val = no;
	}
	void IntConstant::gen_code (int mode){
		string reg = abstract_astnode::Generator.get_register();
		abstract_astnode::Generator.insert_code(MF_MOVE,int_val,reg);
		abstract_astnode::Generator.put_register(reg);
	}
	void IntConstant::print(int tabs){
		print_tabs(tabs);
		cout<<"(IntConstant "<<int_val<<")";
	}
	void IntConstant::setLabel(int arg){
		is_leaf = true;
		min_register = 1 - arg;
	}

	StringConstant::StringConstant(string str): string_val(str){
		is_constant = true;
		setType(STRING_CONSTANT_AST);
		inherent_dt = DT_STRING;
	}

	void StringConstant::print(int tabs){
		print_tabs(tabs);
		cout<<"(StringConstant "<<string_val<<")";
	}
	void StringConstant::gen_code (int mode){;}
	void StringConstant::setLabel(int arg){
		is_leaf = true;
		min_register = 1 - arg;
	}
	string StringConstant::get_value(){
		return string_val;
	}

	ArrayRef::ArrayRef(){
		setType(ARRAY_REF_AST);
	}

	Index::Index(ArrayRef *arr,ExpAst *exp,data_type inherent,mDataType data):array_ref(arr), exp_ast(exp){
		inherent_dt = inherent;
		mdata_type = data;
		setType(INDEX_AST);
		if(exp_ast->has_assignment || array_ref->has_assignment)
			has_assignment = true;
	}
	void Index::index_gen_code(int mode, mDataType* mdt, int size_from_top){
				int sz1 = exp_ast->min_register;
		int sz2 = array_ref->min_register;
		int msz = abstract_astnode::Generator.get_size();
		// cout<<sz1<<" :: "<<sz2<<" :: "<<msz<<" :: "<<endl;
		
		size_from_top /= mdt->index_size;

		int size_of_variable = 4;
		bool is_identifier = false;
		int offset = 0;
		if(array_ref->getType() == IDENTIFIER_AST){
			is_identifier = true;
		
			//direct
			OpBinary::genCode(exp_ast);
			abstract_astnode::Generator.backpatch(exp_ast->nextlist, abstract_astnode::Generator.nextinstr());
			string reg_exp = abstract_astnode::Generator.get_register();
			abstract_astnode::Generator.insert_code(MF_MULT_I,-1*size_from_top,reg_exp);
			// abstract_astnode::Generator.insert_code(MF_MULT_I,,reg_exp);
			
			if((((Identifier*)array_ref)->symbol_entry).vt == VT_PARAMETER){
				string reg_tmp = abstract_astnode::Generator.get_register();
				abstract_astnode::Generator.insert_code(MF_LOAD_I,array_ref->get_address(),reg_tmp);
				abstract_astnode::Generator.insert_code(MF_ADD_I,reg_tmp,reg_exp);
				abstract_astnode::Generator.put_register(reg_tmp);
			}else{
				offset = ((Identifier*)array_ref)->symbol_entry.offset;
				abstract_astnode::Generator.insert_code(MF_ADD_I,offset,reg_exp);
				abstract_astnode::Generator.insert_code(MF_ADD_I,"ebp",reg_exp);
			}
			abstract_astnode::Generator.put_register(reg_exp);

		}else if(sz1 >= msz && sz2 >= msz){
			//evaluate child,store then arg compute
			((Index*)array_ref)->index_gen_code(mode,mdt->subArray,size_from_top);
			//store reg
			string reg = abstract_astnode::Generator.front_register();
			abstract_astnode::Generator.insert_code(MF_PUSH_I,reg);
			//gencode
			OpBinary::genCode(exp_ast);
			abstract_astnode::Generator.backpatch(exp_ast->nextlist, abstract_astnode::Generator.nextinstr());
			//pullout register
			reg = abstract_astnode::Generator.get_register();
			abstract_astnode::Generator.insert_code(MF_MULT_I,-1*size_from_top*size_of_variable,reg);
			//pull another and load into that
			string reg_old = abstract_astnode::Generator.get_register();
			abstract_astnode::Generator.insert_code(MF_LOAD_I,
				abstract_astnode::Generator.mem_offset("esp"),
				reg_old);
			abstract_astnode::Generator.insert_code(MF_POP_I,1);
			abstract_astnode::Generator.insert_code(MF_ADD_I,
				reg_old,reg);
			abstract_astnode::Generator.put_register(reg_old);
			abstract_astnode::Generator.put_register(reg);

		}else if(sz1 < msz && sz2 >= msz ||
			sz1 < msz && sz2 < msz || has_assignment){
			((Index*)array_ref)->index_gen_code(mode,mdt->subArray,size_from_top);
			string reg = abstract_astnode::Generator.get_register();
			//gencode
			OpBinary::genCode(exp_ast);
			abstract_astnode::Generator.backpatch(exp_ast->nextlist, abstract_astnode::Generator.nextinstr());				
			string reg_exp = abstract_astnode::Generator.get_register();
			abstract_astnode::Generator.insert_code(MF_MULT_I,-1*size_from_top,reg_exp);
			//pull another and load into that
			
			abstract_astnode::Generator.insert_code(MF_ADD_I,
				reg_exp,reg);
			abstract_astnode::Generator.put_register(reg_exp);
			abstract_astnode::Generator.put_register(reg);
		}else if(sz1 >= msz && sz2 < msz){
			//evaluate arg, then child then compute
			OpBinary::genCode(exp_ast);//j
			abstract_astnode::Generator.backpatch(exp_ast->nextlist, abstract_astnode::Generator.nextinstr());			
			string reg_exp = abstract_astnode::Generator.get_register();
			abstract_astnode::Generator.insert_code(MF_MULT_I,-1*size_from_top,reg_exp);

			((Index*)array_ref)->index_gen_code(mode,mdt->subArray,size_from_top);
			string reg = abstract_astnode::Generator.get_register();

			abstract_astnode::Generator.insert_code(MF_ADD_I,
				reg_exp,reg);
			abstract_astnode::Generator.put_register(reg_exp);
			abstract_astnode::Generator.put_register(reg);
		}
	}
	void Index::gen_code (int mode){
		abstract_astnode::Generator.add_comment("INDEX evaluation block : ");
		index_gen_code(mode,&mdata_type,mdata_type.size);	
		string reg = abstract_astnode::Generator.get_register();
		string offset = abstract_astnode::Generator.mem_offset(reg);
		if(inherent_dt == DT_FLOAT)
		 	abstract_astnode::Generator.insert_code(MF_LOAD_F,offset,reg);
		else
		 	abstract_astnode::Generator.insert_code(MF_LOAD_I,offset,reg);
		abstract_astnode::Generator.put_register(reg);
	}
	void Index::print(int tabs){
		print_tabs(tabs);
		cout<<"(ArrayRef ";
		array_ref->print(tabs+1);
		exp_ast->print(tabs+1);
		cout<<")";
	}
	void Index::setLabel(int arg){
		is_leaf = false;
		if(array_ref->getType() != IDENTIFIER_AST || 
			((Identifier*)array_ref)->symbol_entry.vt == VT_PARAMETER)
			array_ref->setLabel(0);
		else
			array_ref->setLabel(1);
		exp_ast->setLabel(0);
		min_register = abstract_astnode::get_new_label(array_ref->min_register,
			exp_ast->min_register);
	}

	string Index::get_base_address(){
		ArrayRef* exp_ast_c = this;
		while(exp_ast_c->getType()!=IDENTIFIER_AST){
			exp_ast_c = ((Index*)exp_ast_c)->array_ref;
		}
		return exp_ast_c->get_address();
	}
	string Index::get_address(){
		index_gen_code(0,&mdata_type,mdata_type.size);
		string reg = abstract_astnode::Generator.front_register();
		return abstract_astnode::Generator.mem_offset(reg);
	}
	string Index::get_variable(){
		return array_ref->get_variable();
	}

	Identifier::Identifier(string str):identifier(str){
		setType(IDENTIFIER_AST);
	}
	Identifier::Identifier(string str,SymbolEntry data):identifier(str){
		inherent_dt = data.dt.dt;
		mdata_type = data.dt;
		symbol_entry = data;
		setType(IDENTIFIER_AST);
	}
	void Identifier::gen_code (int mode){
		string reg = abstract_astnode::Generator.get_register();
		if(inherent_dt == DT_INT || (inherent_dt == DT_ARRAY && symbol_entry.vt == VT_PARAMETER)){
			abstract_astnode::Generator.insert_code(MF_LOAD_I,get_address(),reg);
		}
		else if(inherent_dt == DT_FLOAT){
			abstract_astnode::Generator.insert_code(MF_LOAD_F,get_address(),reg);
		}else if(inherent_dt == DT_ARRAY){
			abstract_astnode::Generator.insert_code(MF_MOVE,"ebp",reg);
			abstract_astnode::Generator.insert_code(MF_ADD_I,symbol_entry.offset,reg);
		}

		abstract_astnode::Generator.put_register(reg);
	}
	void Identifier::print(int tabs){
		print_tabs(tabs);
		cout<<"(Identifier "<<identifier<<")";
	}
	void Identifier::setLabel(int arg){
		is_leaf = true;
		min_register = 1 - arg;
	}
	string Identifier::get_address(){
		return abstract_astnode::Generator.mem_offset("ebp",symbol_entry.offset);
	}
	string Identifier::get_variable(){
		return symbol_entry.identifier;
	}