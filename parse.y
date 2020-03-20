%scanner Scanner.h
%scanner-token-function d_scanner.lex()
        
%token   IF ELSE FOR WHILE IDENTIFIER INT_CONSTANT FLOAT_CONSTANT STRING_LITERAL FLOAT INT VOID RETURN OR_OP AND_OP EQ_OP NE_OP LE_OP GE_OP INC_OP

%polymorphic STMT_TYPE : StmtAst*; EXP_TYPE : ExpAst*; 
              SEQ_TYPE: Seq*; ASS_TYPE: Ass*; RETURN_TYPE: Return*;
              IF_TYPE: If*; WHILE_TYPE: While*; FOR_TYPE: For*;
              OPB_TYPE: OpBinary*; OPU_TYPE: OpUnary*; FUNCALL_TYPE: Funcall*;
              FLOAT_CONST_TYPE: FloatConstant*; INT_CONST_TYPE: IntConstant*;
              STRING_CONST_TYPE: StringConstant*; ARRAY_TYPE: ArrayRef*;
              INDEX_TYPE: Index*; INDENTIFIER_TYPE: Identifier*; STRING: string;
              EXP_LIST_TYPE: list<ExpAst*>; STMT_LIST_TYPE: list<StmtAst*>; OPERATORS: operators;
              DATA_TYPE: mDataType*;


%type <STMT_TYPE> function_definition compound_statement statement iteration_statement program
%type <EXP_TYPE> constant_expression expression logical_and_expression equality_expression relational_expression additive_expression multiplicative_expression unary_expression postfix_expression primary_expression 
%type <STMT_LIST_TYPE> statement_list translation_unit
%type <ASS_TYPE> assignment_statement
%type <EXP_LIST_TYPE> expression_list
%type <IF_TYPE> selection_statement
%type <OPERATORS> unary_operator
%type <ARRAY_TYPE> l_expression
%type <STRING> INT_CONSTANT FLOAT_CONSTANT STRING_LITERAL IDENTIFIER 
%type <DATA_TYPE> type_specifier


%%

program:
	translation_unit
	{
		list<StmtAst*> initial = $1;
		$$ = new Seq(initial);
    		cout<<endl;
		cout<<"<<SYMBOL TABLE>>"<<endl;

		global_symbol_table.print("  ");
		
    //We check if there is exactly one main and main is an int main()
    list<SymbolEntry> main_match = matching_functions("main");
    if(main_match.size() == 0){
      addErrorElement(ET_NO_MAIN,"main",global_line_number);  
    }else if(main_match.size() > 1){
      addErrorElement(ET_MULTIPLE_MAIN,"main",global_line_number);  
    }else if((main_match.front()).dt.dt != DT_INT){
      addErrorElement(ET_NON_INT_MAIN,"main",global_line_number);  
    }

	//We print all errors here		
    if(error_list.size() != 0){
      cout<<endl<<"<<ERRORS>>"<<endl;
      for(list<ErrorElement>::iterator li = error_list.begin(); li!= error_list.end();li++){
          li->print_error();
      }
    }else{
	//If no errors we print the AST
      for(list<StmtAst*>::iterator li = initial.begin(); li!=initial.end(); li++){
        (*li)->gen_code();
      }
      abstract_astnode::Generator.print();

      cout<<endl<<"<<Functions>>"<<endl;
      vector<string> functions_list(global_symbol_table.table.size());
      int position_to_select = 1;
      for(list<SymbolEntry>::iterator li=global_symbol_table.table.begin(); li!= global_symbol_table.table.end(); li++){
          string function_title = (*li).identifier + (*li).print_params();
          functions_list[position_to_select-1] = function_title;
          cout<<position_to_select<<") "<<function_title<<endl;
          position_to_select++;
      }

      //cout<<"Choose an AST to print(0 for none, -1 for all)..."<<endl;
      // int position_selected;
      // cin>>position_selected;
      if(position_selected!=0){
        position_to_select = 1;
        for(list<StmtAst*>::iterator li = initial.begin(); li!=initial.end(); li++){
          if(position_to_select == position_selected || position_selected == -1){
            cout<<endl<<"//"<<functions_list[position_to_select - 1]<<"//"<<endl;
            (*li)->print();
            cout<<endl;
          }
          position_to_select++;
        }      
      }
    }

    cout<<endl;
};

// Here we push the function in the list of StmtAst
translation_unit
  : function_definition
  {
    list<StmtAst*> empty;
    empty.push_back($1);
    $$ = empty;
  }
  | translation_unit function_definition
  {
    list<StmtAst*> initial = $1;
    initial.push_back($2);
    $$ = initial;
  }
;

function_definition
  : type_specifier 
      { function_dt = (*$1); }
    fun_declarator compound_statement
  {
    $$ = $4;
	//Address size added for ebp (check return address)
  //current_parameter_list.add_offset(address_size);
	

	//Combine lists of parameters and locals
  	SymbolTable * function_st = new SymbolTable();
    list<data_type> params = getFunctionParams(current_parameter_list.table);
  	function_st->table = current_parameter_list.table;
  	function_st->table.splice(function_st->table.end(),current_variable_list.table);

    global_symbol_table.addEntry(SymbolEntry(function_identifier,
      	VT_FUNCTION, function_dt, 0, params,function_st,global_symbol_table.table.size()));

    ((Seq*)$4)->setSymbolTable(function_st,global_symbol_table.table.size()-1,function_identifier,function_dt.dt);

    current_parameter_list.clear_data();
    current_variable_list.clear_data();

  };

type_specifier
  : VOID
  {
    $$ = new mDataType(DT_VOID);
  }
  | INT
  {
    $$ = new mDataType(DT_INT);
  }
  | FLOAT
  {
    $$ = new mDataType(DT_FLOAT);
  }
;

//For every function we check if a function with the same signature was declared earlier
fun_declarator 
  : IDENTIFIER '(' 
    parameter_list
    ')'{
      error_exists = false;
      parseVar($1);
      function_identifier = global_var_string;
      if(function_identifier == "printf"){
          addErrorElement(ET_REDECLARTION,function_identifier,global_line_number);
      }else if(is_already_defined(function_identifier)){
        if(!is_polymorphic_function(current_parameter_list.table,function_identifier)){
          addErrorElement(ET_REDECLARTION,function_identifier,global_line_number);
          error_exists = true;///FLAG ERROR
        }
      }
  }
  | IDENTIFIER '('
    ')'{
      error_exists = false;
    parseVar($1);
    function_identifier = global_var_string;
    if(is_already_defined(function_identifier)){
        if(!is_polymorphic_function(current_parameter_list.table,function_identifier)){
          addErrorElement(ET_REDECLARTION,function_identifier,global_line_number);
          error_exists = true;///FLAG ERROR
        }
      }
  }
  ;

parameter_list
  : parameter_declaration
  | parameter_list ',' parameter_declaration
  ;

//Here we add each variable to a list of parameters and check if the same name has been used earlier or not
parameter_declaration
  : type_specifier 
      {
        variable_dt = $1;
        variable_type_dt = $1;
      }
    declarator
  {
      error_exists = false;
    if(variable_dt->dt == DT_VOID){
        addErrorElement(ET_VOID_VARIABLE,variable_identifier,global_line_number);
        error_exists = true;
    }else if(is_already_defined(variable_identifier)){
      addErrorElement(ET_VARIABLE_REDECLARATION,variable_identifier,global_line_number);
	  	error_exists = true;///FLAG ERROR
	}
    if(current_parameter_list.current_offset == 0)
      current_parameter_list.current_offset = address_size;

    current_parameter_list.addEntry(variable_identifier,
      VT_PARAMETER,*variable_dt,current_parameter_list.current_offset,NULL);
    current_parameter_list.current_offset += variable_type_dt->size; 
  }
  ;

//For array declarations we check if the size is an int or not
declarator
  : IDENTIFIER
  {
    parseVar($1);
    variable_identifier = global_var_string;
  }
  | declarator '[' constant_expression ']'
  {
      error_exists = false;
    if($3->getType() != INT_CONSTANT_AST){
      addErrorElement(ET_ARRAY_DECLARATION_INTEGER,variable_identifier,global_line_number);
    	error_exists = true;
    }

   	variable_dt = new mDataType(variable_dt,((IntConstant*) $3)->get_value());
  }
;

constant_expression: 
  INT_CONSTANT
  {
    parseVar($1);
    $$ = new IntConstant(global_var_string);
  }
  |FLOAT_CONSTANT
  {
    parseVar($1);
    $$ = new FloatConstant(global_var_string);
  }
;

compound_statement
  : '{' '}'
  {
  	$$ =new Seq();
  }
  | '{' statement_list '}'
  {
    $$ = new Seq($2);
  }
  | '{' 
    declaration_list statement_list '}'
   {
   	$$ = new Seq($3);
   	// current_variable_list.add_offset(-1*current_variable_list.current_offset); //This is required to reverse the order
   }
  ;

statement_list
  : statement
  {
    list<StmtAst*> empty;
    empty.push_back($1);
    $$ = empty;
  }
  | statement_list statement
  {
    list<StmtAst*> empty = $1;
    empty.push_back($2);
    $$ = empty;
  }
  ;

statement
  : '{' statement_list '}'
  {
    $$ = new Seq($2);
  }
  | selection_statement
  {
    $$ = $1;
  }
  | iteration_statement
  {
    $$ = $1;
  }
  | assignment_statement
  {
    $$ = $1;
  }
  | RETURN expression ';'
  {
    $$ = new Return($2,function_dt);
    if($$->inherent_dt == DT_ARRAY){
        addErrorElement(ET_WRONG_RETURN,function_identifier,global_line_number);
        $$->inherent_dt = DT_ERROR;
    }
  }
  | IDENTIFIER '(' ')' ';' 
  {
    parseVar($1);
    list<ExpAst*> empty;

    if(global_var_string == "printf"){
      addErrorElement(ET_INCORRECT_NUMBER_OF_ARGUMENTS,global_var_string,global_line_number);
      $$ = new FuncallStmt(global_var_string,empty,DT_ERROR,empty_dt,true);
    }
    else{
    SymbolEntry se = search_variable(global_var_string);
    error_exists = false;

	   //We check if the identifier is declared preciously and is a valid function with no parameters
      if(se.vt == VT_NULL){
        addErrorElement(ET_UNDECLARED,global_var_string,global_line_number);
        error_exists = true;
      }else if(se.vt != VT_FUNCTION){
        addErrorElement(ET_NOT_A_FUNCTION,global_var_string,global_line_number);
        error_exists = true;
      }else if(!isValidFunctionCall(empty,global_var_string))
        error_exists = true;
      
      if(error_exists)
        $$ = new FuncallStmt(global_var_string,empty,DT_ERROR,correct_function.params,false,correct_function.position_in_table);
      else
        $$ = new FuncallStmt(global_var_string,empty,correct_function.dt,correct_function.params,false,correct_function.position_in_table);
    }
  }

  | IDENTIFIER '(' expression_list ')' ';' 
  {
    parseVar($1);  
    if(global_var_string == "printf"){
      if(validPrintF($3)){
        $$ = new FuncallStmt(global_var_string,print_expast,DT_VOID,empty_dt,true);
      }else{
        $$ = new FuncallStmt(global_var_string,$3,DT_VOID,empty_dt,true);
      }
    }else{
        SymbolEntry se = search_variable(global_var_string);
        list<ExpAst*> empty = $3;
          error_exists = false;

      //We check if the identifier is declared preciously and is a valid function with the appropriate parameters
        if(se.vt == VT_NULL){
          addErrorElement(ET_UNDECLARED,global_var_string,global_line_number);
          error_exists = true;
        }else if(se.vt != VT_FUNCTION){
          addErrorElement(ET_NOT_A_FUNCTION,global_var_string,global_line_number);
          error_exists = true;
        }else if(!isValidFunctionCall(empty,global_var_string))
          error_exists = true;
        
        if(error_exists)
          $$ = new FuncallStmt(global_var_string,empty,DT_ERROR,correct_function.params,false,correct_function.position_in_table);
        else
          $$ = new FuncallStmt(global_var_string,empty,correct_function.dt,correct_function.params,false,correct_function.position_in_table)  ;
    }
  }

  ;

assignment_statement
  : ';'
  {
    $$ = new Ass();
  }
  |  l_expression '=' expression ';'
  {
    valid_l_expression();
	//An assignment is an error if the two types aren't compatible or one of the LHS or RHS is erroneous
    if(
      mDataType::are_compatible($1->inherent_dt,$3->inherent_dt) == DT_ERROR
      && $3->inherent_dt != DT_ERROR && $1->inherent_dt != DT_ERROR){
      addErrorElement(ET_CANNOT_ASSIGN,array_prev_name,global_line_number);
    }   
    
    $$ = new Ass($1,$3);
  }
  ;

expression
  : logical_and_expression
  {
    $$ = $1;
  }
  | expression OR_OP logical_and_expression
  {
    $$ = new OpBinary($1,OR_OPERATION,$3);
  }
  ;

logical_and_expression
  : equality_expression
  {
    $$ = $1;
  }
  | logical_and_expression AND_OP equality_expression
  {
    $$ = new OpBinary($1,AND_OPERATION,$3);
  }
  ;

equality_expression
  : relational_expression
  {
    $$ = $1;
  }
  | equality_expression EQ_OP relational_expression
  {
    $$ = new OpBinary($1,EQ_OPERATION,$3);
  }
  | equality_expression NE_OP relational_expression
  {
    $$ = new OpBinary($1,NE_OPERATION,$3);
  }
  ;
relational_expression
  : additive_expression
  {
    $$ = $1;
  }
  | relational_expression '<' additive_expression
  {
    $$ = new OpBinary($1,LT_OPERATION,$3); 
  }
  | relational_expression '>' additive_expression
  {
    $$ = new OpBinary($1,GT_OPERATION,$3);
  }
  | relational_expression LE_OP additive_expression
  {
    $$ = new OpBinary($1,LE_OPERATION,$3);
  }
  | relational_expression GE_OP additive_expression
  {
    $$ = new OpBinary($1,GE_OPERATION,$3);
  }
  ;

additive_expression
  : multiplicative_expression
  {
    $$ = $1;
  }
  | additive_expression '+' multiplicative_expression
  {
    $$ = new OpBinary($1,PLUS_OPERATION,$3);
  }
  | additive_expression '-' multiplicative_expression
  {
    $$ = new OpBinary($1,SUB_OPERATION,$3);
  }
  ;

multiplicative_expression
  : unary_expression
  {
    $$ = $1;
  }
  | multiplicative_expression '*' unary_expression
  {
    $$ = new OpBinary($1,MULT_OPERATION,$3);
  }
  | multiplicative_expression '/' unary_expression
  {
    $$ = new OpBinary($1,DIV_OPERATION,$3);
  }
  ;
unary_expression
  : postfix_expression
  {
    $$ = $1;
  }
  | unary_operator postfix_expression
  {
    $$ = new OpUnary($2,$1);
  }
  ;

postfix_expression
  : primary_expression
  {
    $$ = $1;
  }
  | IDENTIFIER '(' ')'
  {

    list<ExpAst*> empty;
    parseVar($1);
    if(global_var_string == "printf"){
      addErrorElement(ET_INCORRECT_NUMBER_OF_ARGUMENTS,global_var_string,global_line_number);
      $$ = new Funcall(global_var_string,empty,DT_ERROR,empty_dt,true);
    }
    else{
    	SymbolEntry se = search_variable(global_var_string);
      list<ExpAst*> empty;
        error_exists = false;
    	if(se.vt == VT_NULL){ 
     //We have an error if the function is void
        error_exists = true;
        addErrorElement(ET_UNDECLARED,global_var_string,global_line_number);
      }else if(se.vt != VT_FUNCTION){ //or the identifier is not a function
        error_exists = true;
        addErrorElement(ET_NOT_A_FUNCTION,global_var_string,global_line_number);
      }else if(!isValidFunctionCall(empty,global_var_string)) //or if the parameters don't match
        error_exists = true;
      
      if(error_exists)
        $$ = new Funcall(global_var_string,empty,DT_ERROR,correct_function.params,false,correct_function.position_in_table);
      else
        $$ = new Funcall(global_var_string,empty,correct_function.dt,correct_function.params,false,correct_function.position_in_table);
    }
  }
  | IDENTIFIER '(' expression_list ')'
  {
    error_exists = false;
    parseVar($1);
    if(global_var_string == "printf"){
      $$ = new Funcall(global_var_string,$3,DT_VOID,empty_dt,true);
    }
    else{
      	SymbolEntry se = search_variable(global_var_string);
    	//Same checks as above
      	if(se.vt == VT_NULL){
          addErrorElement(ET_UNDECLARED,global_var_string,global_line_number);
          error_exists = true;
        }else if(se.vt != VT_FUNCTION){
          addErrorElement(ET_NOT_A_FUNCTION,global_var_string,global_line_number);
          error_exists = true;
        }else if(!isValidFunctionCall($3,global_var_string))
          error_exists = true;
       
        if(error_exists)
          $$ = new Funcall(global_var_string,$3,DT_ERROR,correct_function.params,false,correct_function.position_in_table);
        else
         $$ = new Funcall(global_var_string,$3,correct_function.dt,correct_function.params,false,correct_function.position_in_table); 
    }
  }
  | l_expression INC_OP
  {
    valid_l_expression();
    
    $$ = new OpUnary($1,INC_OPERATOR);
  }
;


primary_expression
  : l_expression
  {
    valid_l_expression(); 
    $$ = $1;
  }
  |  l_expression '=' expression 
  {
    valid_l_expression();
    $$ = new OpBinary($1,ASSIGN_OPERATION,$3);
  }
  | INT_CONSTANT
  {
    parseVar($1);
    $$ = new IntConstant(global_var_string);
  }
  | FLOAT_CONSTANT
  {
    parseVar($1);
    $$ = new FloatConstant(global_var_string);
  }
  | STRING_LITERAL
  {
    $$ = new StringConstant($1);
  }  
  | '(' expression ')'
  {
    $$ = $2;
  }
  ;

l_expression
  : IDENTIFIER
  {
    l_expression_ignore = false;
    static_array_stack.push_back(static_array_dt);
    array_stack.push_back(array_dt);
    array_name_stack.push_back(array_name);
    
    error_exists = false;
    parseVar($1);
	//We search for the variable in the symbol table and fetch its entry
  	SymbolEntry se = search_variable(global_var_string);
  	if(se.vt == VT_NULL){ //Error if no such variable found
      addErrorElement(ET_VARIABLE_UNDECLARED,global_var_string,global_line_number);
      error_exists = true;
      array_dt = DEFAULT_DT;
    }else if(se.vt == VT_FUNCTION){ // Error if the identifier is a function 
      addErrorElement(ET_CANNOT_CAST,global_var_string,global_line_number);
  		error_exists = true;
      array_dt = DEFAULT_DT;
    }else{
      array_dt = se.dt;  
      static_array_dt = array_dt;
    } 	

    variable_identifier = global_var_string;
    array_name = variable_identifier;
    mDataType tmp_dt = array_dt;
    global_inherent_dt = se.dt.dt;
    while(global_inherent_dt == DT_ARRAY){ //If an array we store its type in a global to be used later
      tmp_dt = *(tmp_dt.subArray);
      global_inherent_dt = tmp_dt.dt;
    }

    $$ = new Identifier(global_var_string,se);
  }
  | l_expression '[' expression ']' 
  {
    l_expression_ignore = false;
    error_exists = false;
  	if(($3)->inherent_dt != DT_INT){ //If the expression isn't of type int we throw an error
  		addErrorElement(ET_ARRAY_ARGUMENT_INTEGER,variable_identifier,global_line_number);
      error_exists = true;
      array_dt = *(array_dt.subArray);
  	}else if(search_variable(array_name).vt == VT_NULL){ 
      ;
    }else if(array_dt.dt != DT_ARRAY){//If the l_exp isn't an array we throw an error
      addErrorElement(ET_ARRAY_OVER_USAGE,variable_identifier,global_line_number);
      error_exists = true;
    }else{
      array_dt = *(array_dt.subArray);
    }

    if(!error_exists){
      $$ = new Index($1,$3,global_inherent_dt,static_array_dt);
    }
    else{
      $$ = new Index($1,$3,DT_ERROR,static_array_dt);
    }
  }
  ;
expression_list
  : expression
  {
    list<ExpAst*> empty;
    empty.push_back($1);
    $$ = empty;
  }
  | expression_list ',' expression
  {
    list<ExpAst*> empty = $1;
    empty.push_back($3);
    $$ = empty;
  }
;

unary_operator
  : '-'
    {
      $$ = MINUS_OPERATION;
    }
  | '!'
    {
      $$ = BANG_OPERATION;
    }
  ;

selection_statement
  : IF '(' expression ')' statement ELSE statement
  {
    $$ = new If($3,$5,$7);
  }
  ;

iteration_statement
  : WHILE '(' expression ')' statement
  {
      $$ = new While($3,$5);
  }
  | FOR '(' expression ';' expression ';' expression ')' statement 
  { 
      $$ = new For($3,$5,$7,$9);
  }
  ;

declaration_list
  : declaration
  | declaration_list declaration
  ;

declaration
  : type_specifier 
      {
        variable_dt = $1;
        variable_type_dt = $1;
      }
    declarator_list';'
  ;

declarator_list
  : declarator
    {
      error_exists = false;
      if(variable_dt->dt == DT_VOID){ //We have an error if the variable is of type void
        addErrorElement(ET_VOID_VARIABLE,variable_identifier,global_line_number);
        error_exists = true;
      }else if(is_already_defined(variable_identifier,1)){ //Error if the variable's name has already been used
      addErrorElement(ET_VARIABLE_REDECLARATION,variable_identifier,global_line_number);
	  	error_exists = true;///FLAG ERROR
	    }
      if(current_variable_list.current_offset == 0)
        current_variable_list.current_offset = -4;

      current_variable_list.addEntry(variable_identifier,
        VT_VARIABLE,*variable_dt,current_variable_list.current_offset,NULL);
      current_variable_list.current_offset -= variable_dt->size;

      variable_dt = variable_type_dt;
    }
  | declarator_list ',' 
    declarator
    {
      error_exists = false;
      if(variable_dt->dt == DT_VOID){
        addErrorElement(ET_VOID_VARIABLE,variable_identifier,global_line_number);
        error_exists = true;
      }else if(is_already_defined(variable_identifier,1)){
      addErrorElement(ET_VARIABLE_REDECLARATION,variable_identifier,global_line_number);
	  	error_exists = true;///FLAG ERROR
	  }
    if(current_variable_list.current_offset == 0)
        current_variable_list.current_offset = -4;

    current_variable_list.addEntry(variable_identifier,
       VT_VARIABLE,*variable_dt,current_variable_list.current_offset,NULL);
    current_variable_list.current_offset -= variable_dt->size;
     variable_dt = variable_type_dt; 
    }
  ;
