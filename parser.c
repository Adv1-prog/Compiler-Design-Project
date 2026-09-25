/*
Program         ::= ( Declaration | Rule )* EOF ;
Declaration     ::= ("SENSOR" | "ACTUATOR") IDENTIFIER ":" Type ";" ;
Type            ::= "float" | "int" | "bool" ;
Rule            ::= "RULE" IDENTIFIER ":" "IF" Condition "THEN" ActionList ";" ;
Condition       ::= LogicalOr ;
LogicalOr       ::= LogicalAnd ( "OR" LogicalAnd )* ;
LogicalAnd      ::= PrimaryCond ( "AND" PrimaryCond )* ;
PrimaryCond     ::= IDENTIFIER RelOp Literal | "(" Condition ")" | "NOT" PrimaryCond ;
RelOp           ::= ">" | "<" | ">=" | "<=" | "==" | "!=" ;
ActionList      ::= Action ( "," Action )* ;
Action          ::= IDENTIFIER "(" ( Literal | IDENTIFIER )? ")" ;
Literal         ::= INT_LITERAL | FLOAT_LITERAL | STRING_LITERAL ;
*/