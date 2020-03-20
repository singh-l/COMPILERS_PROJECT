digits                         [0-9]*
some_digits                         [0-9]+
exp_regex_1                     [eE][+-][0-9]*
exp_regex_2                     [eE][0-9]*
escapable_chars                [tn\\"]
%%
[ \t]
[\n]			newLno++;                     
//Type Specifiers
"float"           return Parser::FLOAT;
"int"            return Parser::INT;
"void"            return Parser::VOID;


//Double Digit Operators
[=][=]            return Parser::EQ_OP;
[!][=]            return Parser::NE_OP;
[<][=]            return Parser::LE_OP;
[>][=]            return Parser::GE_OP;
[&][&]            return Parser::AND_OP;
[|][|]            return Parser::OR_OP;
[+][+]            return Parser::INC_OP;


"return"             return Parser::RETURN;
\. 					return '.';


[1-9]{digits}	{Parser::d_val__ = std::string(matched()) + " " + std::to_string(newLno); return Parser::INT_CONSTANT;}
0[bB][01]*		{Parser::d_val__ = std::string(matched()) + " " + std::to_string(newLno); return Parser::INT_CONSTANT;}
0[0-7]*			{Parser::d_val__ = std::string(matched()) + " " + std::to_string(newLno); return Parser::INT_CONSTANT;}
0[xX][0-9a-fA-F]*	{Parser::d_val__ = std::string(matched()) + " " + std::to_string(newLno); return Parser::INT_CONSTANT; }

\.{some_digits}({exp_regex_1}|{exp_regex_2})? {Parser::d_val__ = std::string(matched()) + " " + std::to_string(newLno); return Parser::FLOAT_CONSTANT;}
{some_digits}\.({exp_regex_1}|{exp_regex_2})? {Parser::d_val__ = std::string(matched()) + " " + std::to_string(newLno); return Parser::FLOAT_CONSTANT;}
{some_digits}\.{some_digits}({exp_regex_1}|{exp_regex_2})? {Parser::d_val__ = std::string(matched()) + " " + std::to_string(newLno); return Parser::FLOAT_CONSTANT;}
{some_digits}({exp_regex_1}|{exp_regex_2})? {Parser::d_val__ = std::string(matched()) + " " + std::to_string(newLno); return Parser::FLOAT_CONSTANT;}


//\'((.)|(\\{escapable_chars}))\'         return Parser::CHAR_CONSTANT;
\"([^\\"]|([\\]{escapable_chars}))*\"                {Parser::d_val__ = std::string(matched()); return Parser::STRING_LITERAL;}

"if"            return Parser::IF;
"else"            return Parser::ELSE;
"while"           return Parser::WHILE;
"for"            return Parser::FOR;

[[:alpha:]_][[:alpha:][:digit:]_]*          {Parser::d_val__ = std::string(matched()) + " " + std::to_string(newLno); return Parser::IDENTIFIER;}
.            return matched()[0];
