%{
    #include <cstdlib>
    #include <cstdio>
    #include <iostream>
    #include "ast.hpp"
    
    #define YYDEBUG 1
    #define YYINITDEPTH 10000
    
    int yylex(void);
    void yyerror(const char *);
    
    extern ASTNode* astRoot;
%}

%error-verbose
// %glr-parser
/* NOTE: You may use the %glr-parser directive, which may allow your parser to
         work even with some shift/reduce conflicts remianing. */

%token T_PRINT
%token T_RETURN
%token T_IF
%token T_ELSE
%token T_NEW
%token T_INT
%token T_BOOLEAN
%token T_NONE
%token T_TRUE
%token T_FALSE
%token T_EXTENDS
%token T_WHILE
%token T_DO
%token T_QUESTION
%token T_COLON
%token T_OR
%token T_AND
%token T_GREATER
%token T_GREATER_EQUAL
%token T_EQUAL_EQUAL
%token T_PLUS
%token T_MINUS
%token T_MULTIPLIES
%token T_DIVIDES
%token T_NOT
%token T_LEFT_BRACKET
%token T_RIGHT_BRACKET
%token T_LEFT_PARENTHESIS
%token T_RIGHT_PARENTHESIS
%token T_SEMICOLON
%token T_ID
%token T_INTEGER
%token T_COMMA
%token T_PERIOD
%token T_EQUAL
%token T_ARROW

%right T_QUESTION
%left T_OR
%left T_AND
%left T_GREATER T_GREATER_EQUAL T_EQUAL_EQUAL
%left T_PLUS T_MINUS
%left T_MULTIPLIES T_DIVIDES
%right T_NOT T_U_MINUS


%type <program_ptr> Start
%type <class_list_ptr> ClassList
%type <class_ptr> Class
%type <declaration_list_ptr> ClassMembers Declarations
%type <method_list_ptr> Methods
%type <declaration_ptr> ClassMember FunctionMember
%type <type_ptr> Type ReturnType
%type <method_ptr> Method
%type <parameter_list_ptr> Parameters ParameterList
%type <methodbody_ptr> Body
%type <parameter_ptr> Parameter LastParameter
%type <statement_list_ptr> Statements Block
%type <returnstatement_ptr> OptionalReturn
%type <identifier_list_ptr> IdentifierList
%type <expression_ptr> Expression
%type <statement_ptr> Statement
%type <assignment_ptr> Assignment
%type <call_ptr> Call
%type <methodcall_ptr> MethodCall
%type <ifelse_ptr> IfElse
%type <while_ptr> While
%type <dowhile_ptr> DoWhile
%type <print_ptr> Print
%type <expression_list_ptr> Arguments Argument

%type <identifier_ptr> ID
%type <integer_ptr> INTEGER
%type <base_char_ptr> T_ID
%type <base_int> T_INTEGER

%%

Start : Class ClassList {$2->push_front($1); $$ = new ProgramNode($2); astRoot = $$;}
      ;
ClassList : Class ClassList {$$ = $2; $$->push_front($1);}
          | %empty {$$ = new std::list<ClassNode*>();}
          ;
Class : ID T_EXTENDS ID T_LEFT_BRACKET ClassMembers Methods T_RIGHT_BRACKET {$$ = new ClassNode($1, $3, $5, $6);}
      | ID T_LEFT_BRACKET ClassMembers Methods T_RIGHT_BRACKET {$$ = new ClassNode($1, NULL, $3, $4);}
      ;

ClassMembers : ClassMembers ClassMember {$$ = $1; $$->push_back($2);}
             | %empty {$$ = new std::list<DeclarationNode*>();}
             ;
ClassMember : Type ID T_SEMICOLON {std::list<IdentifierNode*>* temp = new std::list<IdentifierNode*>();
                                    temp->push_back($2); $$ = new DeclarationNode($1, temp);}
            ;

Methods : Method Methods {$$ = $2; $$->push_front($1);}
        | %empty {$$ = new std::list<MethodNode*>();}
        ;
Method : ID T_LEFT_PARENTHESIS Parameters T_RIGHT_PARENTHESIS T_ARROW ReturnType T_LEFT_BRACKET Body T_RIGHT_BRACKET {$$ = new MethodNode($1, $3, $6, $8);}
       ;
Parameters : ParameterList {$$ = $1;}
           | %empty {$$ = NULL;}
           ;
ParameterList : Parameter ParameterList {$$ = $2; $$->push_front($1);}
              | LastParameter {$$ = new std::list<ParameterNode*>(); $$->push_front($1);}
              ;
Parameter : Type ID T_COMMA {$$ = new ParameterNode($1, $2);}
          ;
LastParameter : Type ID {$$ = new ParameterNode($1, $2);}
              ;

Body : Declarations Statements OptionalReturn {$$ = new MethodBodyNode($1, $2, $3);}
     ;
Declarations : Declarations FunctionMember {$$ = $1; $$->push_back($2);}
             | %empty {$$ = new std::list<DeclarationNode*>();}
             ;

FunctionMember : Type IdentifierList T_SEMICOLON {$$ = new DeclarationNode($1, $2);}
               ;
IdentifierList : ID {$$ = new std::list<IdentifierNode*>(); $$->push_front($1);}
               | ID T_COMMA IdentifierList {$$ = $3; $$->push_front($1);}
               ;

OptionalReturn : T_RETURN Expression T_SEMICOLON {$$ = new ReturnStatementNode($2);}
               | %empty {$$ = NULL;}
               ;

Block : Statement Statements {$$ = $2; $$->push_front($1);}
      ;
Statements : Statement Statements {$$ = $2; $$->push_front($1);}
           | %empty {$$ = new std::list<StatementNode*>();}
           ;
Statement : Assignment {$$ = $1;}
          | Call {$$ = $1;}
          | IfElse {$$ = $1;}
          | While {$$ = $1;}
          | DoWhile {$$ = $1;}
          | Print {$$ = $1;}
          ;

Assignment : ID T_EQUAL Expression T_SEMICOLON {$$ = new AssignmentNode($1, NULL, $3);}
           | ID T_PERIOD ID T_EQUAL Expression T_SEMICOLON {$$ = new AssignmentNode($1, $3, $5);}
           ;

IfElse : T_IF Expression T_LEFT_BRACKET Block T_RIGHT_BRACKET {$$ = new IfElseNode($2, $4, new std::list<StatementNode*>());}
       | T_IF Expression T_LEFT_BRACKET Block T_RIGHT_BRACKET T_ELSE T_LEFT_BRACKET Block T_RIGHT_BRACKET {$$ = new IfElseNode($2, $4, $8);}
       ;

While : T_WHILE Expression T_LEFT_BRACKET Block T_RIGHT_BRACKET {$$ = new WhileNode($2, $4);}
      ;
DoWhile : T_DO T_LEFT_BRACKET Block T_RIGHT_BRACKET T_WHILE T_LEFT_PARENTHESIS Expression T_RIGHT_PARENTHESIS T_SEMICOLON {$$ = new DoWhileNode($3, $7);}
        ;
Print : T_PRINT Expression T_SEMICOLON {$$ = new PrintNode($2);}
      ;


Expression : Expression T_PLUS Expression {$$ = new PlusNode($1, $3);}
           | Expression T_MINUS Expression {$$ = new MinusNode($1, $3);}
           | Expression T_MULTIPLIES Expression {$$ = new TimesNode($1, $3);}
           | Expression T_DIVIDES Expression {$$ = new DivideNode($1, $3);}
           | Expression T_GREATER Expression {$$ = new GreaterNode($1, $3);}
           | Expression T_GREATER_EQUAL Expression {$$ = new GreaterEqualNode($1, $3);}
           | Expression T_EQUAL_EQUAL Expression {$$ = new EqualNode($1, $3);}
           | Expression T_AND Expression {$$ = new AndNode($1, $3);}
           | Expression T_OR Expression {$$ = new OrNode($1, $3);}
           | T_NOT Expression {$$ = new NotNode($2);}
           | Expression T_QUESTION Expression T_COLON Expression           %prec T_QUESTION {$$ = new QMNode($1, $3, $5);}
           | T_MINUS Expression                                            %prec T_U_MINUS {$$ = new NegationNode($2);}
           | ID {$$ = new VariableNode($1);}
           | ID T_PERIOD ID {$$ = new MemberAccessNode($1, $3);}
           | MethodCall {$$ = $1;}
           | T_LEFT_PARENTHESIS Expression T_RIGHT_PARENTHESIS {$$ = $2;}
           | INTEGER {$$ = new IntegerLiteralNode($1);}
           | T_TRUE {$$ = new BooleanLiteralNode(new IntegerNode(1));}
           | T_FALSE {$$ = new BooleanLiteralNode(new IntegerNode(0));}
           | T_NEW ID {$$ = new NewNode($2, new std::list<ExpressionNode*>());}
           | T_NEW ID T_LEFT_PARENTHESIS Arguments T_RIGHT_PARENTHESIS {$$ = new NewNode($2, $4);}
           ;
Call : MethodCall T_SEMICOLON{$$ = new CallNode($1);}
     ;
MethodCall : ID T_LEFT_PARENTHESIS Arguments T_RIGHT_PARENTHESIS {$$ = new MethodCallNode($1, NULL, $3);}
           | ID T_PERIOD ID T_LEFT_PARENTHESIS Arguments T_RIGHT_PARENTHESIS {$$ = new MethodCallNode($1, $3, $5);}
           ;
Arguments : Argument {$$ = $1;}
          | %empty {$$ = new std::list<ExpressionNode*>();}
          ;
Argument : Argument T_COMMA Expression {$$ = $1; $$->push_back($3);}
         | Expression {$$ = new std::list<ExpressionNode*>(); $$->push_back($1);}
         ;

Type : T_INT {$$ = new IntegerTypeNode();}
     | T_BOOLEAN {$$ = new BooleanTypeNode();}
     | ID {$$ = new ObjectTypeNode($1);}
     ;
ReturnType : Type {$$ = $1;}
           | T_NONE {$$ = new NoneNode();}
           ;

ID : T_ID {$$ = new IdentifierNode($1);}
   ;
INTEGER : T_INTEGER {$$ = new IntegerNode($1);}
        ;

%%

extern int yylineno;

void yyerror(const char *s) {
  fprintf(stderr, "%s at line %d\n", s, yylineno);
  exit(1);
}