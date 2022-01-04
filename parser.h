//----------------------------------------------------------------------
// NAME: Weston Averill
// FILE: parser.h
// DATE: 2/12/2021
// DESC: Create a parser
//----------------------------------------------------------------------

#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "mypl_exception.h"
#include "ast.h"
#include "printer.h"
//#include<string>
using namespace std;
class Parser
{
public:

  // create a new recursive descent parser
  Parser(const Lexer& program_lexer);

  // run the parser
  void parse(Program& ast_root);
  bool debugFlag = false;
  
private:
  Lexer lexer;
  Token curr_token;
  
  // helper functions
  void advance();
  void eat(TokenType t, std::string err_msg);
  void error(std::string err_msg);
  bool is_operator(TokenType t);
  void debug(std::string msg);
  // recursive descent functions
  void program();
  void tdecl(TypeDecl& typeDecl);
  void fdecl(FunDecl& funDecl);
  void vdecls(TypeDecl& typeDecl);
  void params(list<FunDecl::FunParam>& fParams);
  void dtype(Token& type);
  void stmts(list<Stmt*>& statements);
  void stmt();
  void vdecl_stmt(VarDeclStmt& varDeclStmt);
  void assign_stmt(AssignStmt& assignStmt);
  void lvalue(list<Token>& lvalue_lists);
  void cond_stmt(IfStmt& ifStmt);
  void condt(IfStmt& ifStmt);
  void while_stmt(WhileStmt& whileStmt);
  void for_stmt(ForStmt& forStmt);
  void call_expr(CallExpr& callExpr2);
  void args(list<Expr*>& arg_list);
  void exit_stmt(ReturnStmt& returnStmt);
  void expr(Expr& exprHead);
  void op(Expr& exprHead);
  void rvalue(SimpleTerm& simpleTerm); 
  void pval(SimpleRValue& simpleRValue);
  void idrval();
};

// constructor
Parser::Parser(const Lexer& program_lexer) : lexer(program_lexer)
{

}

// Helper functions
void Parser::advance()
{
  curr_token = lexer.next_token();
}

void Parser::eat(TokenType t, std::string err_msg)
{
  if (curr_token.type() == t)
    advance();
  else
    error(err_msg);
}

void Parser::error(std::string err_msg)
{
  std::string s = err_msg + "found '" + curr_token.lexeme() + "'";
  int line = curr_token.line();
  int col = curr_token.column();
  throw MyPLException(SYNTAX, s, line, col);
}

bool Parser::is_operator(TokenType t)
{
  return t == PLUS or t == MINUS or t == DIVIDE or t == MULTIPLY or
    t == MODULO or t == AND or t == OR or t == EQUAL or t == LESS or
    t == GREATER or t == LESS_EQUAL or t == GREATER_EQUAL or t == NOT_EQUAL;
}

//use this to see where problems occur
void Parser::debug(std::string msg)
{
  if (debugFlag)
    std::cout << msg << std::endl;
}

// Recursive-decent functions
void Parser::parse(Program& ast_root)
{
  //cout << "here";
  advance();
  while (curr_token.type() != EOS) {
    //if its a type, we need to create a typedecl object
    if (curr_token.type() == TYPE) {
      TypeDecl* typeDecl = new TypeDecl();
      tdecl(*typeDecl);
      ast_root.decls.push_back(typeDecl);
    }
    else {
      //else we now know its a function declaration
      FunDecl* funDecl = new FunDecl();
      fdecl(*funDecl);
      ast_root.decls.push_back(funDecl);
    }
    //cout << ast_root.decls.size() << " ";
  }
  //cout << ast_root.decls.size() << "here";
  eat(EOS, "expecting end-of-file ");
}

//start by checking type and ID
void Parser::tdecl(TypeDecl& typeDecl)
{
  debug("<tdecl>");
  eat(TYPE, "expecting type ");
  //typedecl was passed in so we just set the id to current token
  typeDecl.id = curr_token;
  eat(ID, "expecting an id ");
  //then call vdecls and pass typedecl to that function
  vdecls(typeDecl);
  eat(END, "expecting an end ");
}

//rule to check for function header
void Parser::fdecl(FunDecl& funDecl)
{
  debug("<fdecl>");
  eat(FUN, "expecting fun ");
  //we can just eat a nill
  if (curr_token.type() == NIL) {
    funDecl.return_type = curr_token;
    eat(NIL, " expecting a nil ");
  }
  //we need to set the retrun type of fundecls
  else {
    if (curr_token.type() == INT_TYPE) {
      funDecl.return_type = curr_token;
      eat(INT_TYPE, "expecting an int type ");
    }
    else if (curr_token.type() == DOUBLE_TYPE) {
      funDecl.return_type = curr_token;
      eat(DOUBLE_TYPE, "expecting a double type ");
    }
    else if (curr_token.type() == BOOL_TYPE) {
      funDecl.return_type = curr_token;
      eat(BOOL_TYPE, "expecting a bool type ");
    }
    else if (curr_token.type() == CHAR_TYPE) {
      funDecl.return_type = curr_token;
      eat(CHAR_TYPE, "expecting a cahr type ");
    }
    else if (curr_token.type() == STRING_TYPE) {
      funDecl.return_type = curr_token;
      eat(STRING_TYPE, "expecting a string type ");
    }
    else if (curr_token.type() == ID) {
      funDecl.return_type = curr_token;
      eat(ID, "expecting an id ");
    }
  }
  //now we can set the id of the function declaration
  funDecl.id = curr_token;
  eat(ID, "expecting an id ");
  eat(LPAREN, "expecting a '(' ");
  params(funDecl.params);
  eat(RPAREN, "expecting a ')' ");
  stmts(funDecl.stmts);
  eat(END, "expecting an end ");
}

//recursize to keep checking for rules
void Parser::vdecls(TypeDecl& typeDecl)
{
  debug("<vdecls>");
  while (curr_token.type() == VAR) {
    //basiccaly get the variable declarations
    //and push them into a list
    VarDeclStmt* varDeclStmt = new VarDeclStmt();
    typeDecl.vdecls.push_back(varDeclStmt);
    vdecl_stmt(*varDeclStmt);
  }
}

//now check the parameters
void Parser::params(list<FunDecl::FunParam>& params)
{
  debug("<params>");
  if (curr_token.type() == ID || curr_token.type() == POINTER_TYPE) {
    //create a list that stores the parameters of a function
    FunDecl::FunParam* funParam = new FunDecl::FunParam();
    funParam->id = curr_token;
    if (curr_token.type() == ID) {
      eat(ID, "expecting an ID ");
      eat(COLON, "expecting a ':' ");
    }
    else {
      eat(POINTER_TYPE, "expecting a pointer type");
      eat(COLON, "expecting a ':' ");
    }
    
    //a bunch of if statements to check the type
    //also sets the type
    if (curr_token.type() == INT_TYPE) {
      funParam->type = curr_token;
      eat(INT_TYPE, "expecting an int type ");
    }
    else if (curr_token.type() == DOUBLE_TYPE) {
      funParam->type = curr_token;
      eat(DOUBLE_TYPE, "expecting a double type ");
    }
    else if (curr_token.type() == BOOL_TYPE) {
      funParam->type = curr_token;
      eat(BOOL_TYPE, "expecting a bool type ");
    }
    else if (curr_token.type() == CHAR_TYPE) {
      funParam->type = curr_token;
      eat(CHAR_TYPE, "expecting a cahr type ");
    }
    else if (curr_token.type() == STRING_TYPE) {
      funParam->type = curr_token;
      eat(STRING_TYPE, "expecting a string type ");
    }
    else if (curr_token.type() == ID) {
      funParam->type = curr_token;
      eat(ID, "expecting an id ");
    }
    //now push that parameter to the list
    //and more check to get all paramters in the function declaration
    params.push_back(*funParam);
    while (curr_token.type() == COMMA) {
      eat(COMMA, "expecting a comma ");
      FunDecl::FunParam* funParam2 = new FunDecl::FunParam();
      funParam2->id = curr_token;
      eat(ID, "expecting an id ");
      eat(COLON, "expecting a colon ");
      if (curr_token.type() == INT_TYPE) {
        funParam2->type = curr_token;
        eat(INT_TYPE, "expecting an int type ");
      }
      else if (curr_token.type() == DOUBLE_TYPE) {
        funParam2->type = curr_token;
        eat(DOUBLE_TYPE, "expecting a double type ");
      }
      else if (curr_token.type() == BOOL_TYPE) {
        funParam2->type = curr_token;
        eat(BOOL_TYPE, "expecting a bool type ");
      }
      else if (curr_token.type() == CHAR_TYPE) {
        funParam2->type = curr_token;
        eat(CHAR_TYPE, "expecting a cahr type ");
      }
      else if (curr_token.type() == STRING_TYPE) {
        funParam2->type = curr_token;
        eat(STRING_TYPE, "expecting a string type ");
      }
      else if (curr_token.type() == ID) {
        funParam2->type = curr_token;
        eat(ID, "expecting an id ");
      }
      params.push_back(*funParam2);
    }
  }
}

//check for the dadta types
void Parser::dtype(Token& type)
{
  debug("<dtype>");
  //we can use this to check the types of tokens
  //we decided not to use this in the parser
  if (curr_token.type() == INT_TYPE) {
    type = curr_token;
    eat(INT_TYPE, "expecting an int type ");
  }
  else if (curr_token.type() == DOUBLE_TYPE) {
    type = curr_token;
    eat(DOUBLE_TYPE, "expecting a double type ");
  }
  else if (curr_token.type() == BOOL_TYPE) {
    type = curr_token;
    eat(BOOL_TYPE, "expecting a bool type ");
  }
  else if (curr_token.type() == CHAR_TYPE) {
    type = curr_token;
    eat(CHAR_TYPE, "expecting a cahr type ");
  }
  else if (curr_token.type() == STRING_TYPE) {
    type = curr_token;
    eat(STRING_TYPE, "expecting a string type ");
  }
  else if (curr_token.type() == ID) {
    type = curr_token;
    eat(ID, "expecting an id ");
  }
}

//I use stmts to to call itself instead of creating stmt function
void Parser::stmts(list<Stmt*>& statements)
{ 
  debug("<stmts>");
  //check all different types of statements we can have in a program
  if (curr_token.type() == VAR) {
    //variable declaration statement
    VarDeclStmt* varDeclStmt = new VarDeclStmt();
    statements.push_back(varDeclStmt);
    vdecl_stmt(*varDeclStmt);
  }
  else if (curr_token.type() == POINTER_TYPE) {
      AssignStmt* assignStmt = new AssignStmt();
      assignStmt->lvalue_list.push_back(curr_token);
      statements.push_back(assignStmt);
      eat(POINTER_TYPE, "expecting a pointer type");
      assign_stmt(*assignStmt);
  }
  else if (curr_token.type() == ID) {
    //id means we need to keep checking to see 
    //what kind of statement it can be
    Token temp = curr_token;
    eat(ID, "expecting an id");
    if (curr_token.type() == LPAREN) {
      //now we know its a function call
      CallExpr* callExpr = new CallExpr();
      callExpr->function_id = temp;
      statements.push_back(callExpr);
      call_expr(*callExpr);
    }
    else {
      //now we know its an assignment statement
      AssignStmt* assignStmt = new AssignStmt();
      assignStmt->lvalue_list.push_back(temp);
      statements.push_back(assignStmt);
      assign_stmt(*assignStmt); 
    } 
  }
  //if statement
  else if (curr_token.type() == IF) {
    IfStmt* ifStmt = new IfStmt();
    statements.push_back(ifStmt); 
    cond_stmt(*ifStmt);
  }
  //while statement
  else if (curr_token.type() == WHILE) {
    WhileStmt* whileStmt = new WhileStmt();
    statements.push_back(whileStmt);
    while_stmt(*whileStmt);
  }
  //for statement
  else if (curr_token.type() == FOR) {
    ForStmt* forStmt = new ForStmt();
    statements.push_back(forStmt);
    for_stmt(*forStmt);
  }
  //return statement
  else if (curr_token.type() == RETURN) {
    ReturnStmt* returnStmt = new ReturnStmt();
    statements.push_back(returnStmt);
    exit_stmt(*returnStmt);
  }
  else
    return;
  stmts(statements);
}

void Parser::vdecl_stmt(VarDeclStmt& varDeclStmt)
{
  debug("<vdecl_stmt>");
  //now we are checking the variable declaration statement
  if (curr_token.type() == VAR) { 
    eat(VAR, "expecting a var ");
    if (curr_token.type() == ID) {
      varDeclStmt.id = curr_token;
      eat(ID, "expecting an id ");
    }
    else if (curr_token.type() == POINTER_TYPE) {
      varDeclStmt.id = curr_token;
      eat(POINTER_TYPE, "expecting pointer type");
      varDeclStmt.pointer = true;
    }
  }
  //if there is a colon, then we need to get the type
  if (curr_token.type() == COLON) {
    eat(COLON, "expecting a colon ");
    if (curr_token.type() == INT_TYPE) {
      varDeclStmt.type = new Token(INT_TYPE, curr_token.lexeme(), curr_token.line(), curr_token.column());
      //arDeclStmt.type = curr_token.type();
      eat(INT_TYPE, "expecting an int type ");
    }
    else if (curr_token.type() == DOUBLE_TYPE) {
      varDeclStmt.type = new Token(DOUBLE_TYPE, curr_token.lexeme(), curr_token.line(), curr_token.column());
      //varDeclStmt.type = &curr_token;
      eat(DOUBLE_TYPE, "expecting a double type ");
    }
    else if (curr_token.type() == BOOL_TYPE) {
      varDeclStmt.type = new Token(BOOL_TYPE, curr_token.lexeme(), curr_token.line(), curr_token.column());
      //varDeclStmt.type = &curr_token;
      eat(BOOL_TYPE, "expecting a bool type ");
    }
    else if (curr_token.type() == CHAR_TYPE) {
      varDeclStmt.type = new Token(CHAR_TYPE, curr_token.lexeme(), curr_token.line(), curr_token.column());
      //varDeclStmt.type = &curr_token;
      eat(CHAR_TYPE, "expecting a cahr type ");
    }
    else if (curr_token.type() == STRING_TYPE) {
      varDeclStmt.type = new Token(STRING_TYPE, curr_token.lexeme(), curr_token.line(), curr_token.column());
     // varDeclStmt.type = &curr_token;
      eat(STRING_TYPE, "expecting a string type ");
    }
    else if (curr_token.type() == ID) {
      varDeclStmt.type = new Token(ID, curr_token.lexeme(), curr_token.line(), curr_token.column());
     // varDeclStmt.type = &curr_token;
      eat(ID, "expecting an id here");
    }
  }
  //now create an expr for assignment statement
  eat(ASSIGN, "expecting an assign ");
  Expr* expr2 = new Expr();
  varDeclStmt.expr = expr2;
  expr(*expr2);
}

//assignment rule
void Parser::assign_stmt(AssignStmt& assignStmt)
{
  debug("<assign_stmt>");
  //we need to find all of the things on the left side of assignment statement
  lvalue(assignStmt.lvalue_list);
  eat(ASSIGN, "expecting a '=' ");
  //now create an expr for right side
  Expr* expr2 = new Expr();
  assignStmt.expr = expr2;
  expr(*expr2);
}

//get the left value thats being assigned
void Parser::lvalue(list<Token>& lvalue_lists)
{
  debug("<lvalue>");
  //getting the left side things
  while (curr_token.type() == DOT) {
    eat(DOT, "expecting a dot ");
    //push them to the list
    lvalue_lists.push_back(curr_token);
    eat(ID, "expecting an id ");
  }
}

//if statement rule
void Parser::cond_stmt(IfStmt& ifStmt)
{
  debug("<cond_stmt>");
  //creat the basif if part
  BasicIf* basicIf = new BasicIf;
  ifStmt.if_part = basicIf;
  //now the expr for that if
  Expr* expr2 = new Expr();
  basicIf->expr = expr2;
  eat(IF, "expecting an if ");
  expr(*expr2);
  eat(THEN, "expecting a then ");
  stmts(basicIf->stmts);
  condt(ifStmt);
  eat(END, "expecting an end ");
}

//rule that can be use in a conditional statement
void Parser::condt(IfStmt& ifStmt)
{
  debug("<condt>");
  //now check for the esle if in an if statement
  if (curr_token.type() == ELSEIF) {
    BasicIf* basicIf = new BasicIf();
    Expr* expr2 = new Expr();
    basicIf->expr = expr2;
    ifStmt.else_ifs.push_back(basicIf);
    //eat(ELSEIF, "expecting an else if ");
    advance();
    expr(*expr2);
    eat(THEN, "expecting a then ");
    stmts(basicIf->stmts);
    condt(ifStmt);
  }
  //now check for else statements
  else if (curr_token.type() == ELSE) {
    eat(ELSE, "expecting an else ");
    stmts(ifStmt.body_stmts);
  }
}

//while statement rule
void Parser::while_stmt(WhileStmt& whileStmt)
{
  debug("<while_stmt>");
  //now check the while statement
  eat(WHILE, "expecting a while ");
  Expr* expr2 = new Expr();
  whileStmt.expr = expr2;
  expr(*expr2);
  eat(DO, "expecting a do ");
  stmts(whileStmt.stmts);
  eat(END, "expecting an end ");
}

//for loop rule
void Parser::for_stmt(ForStmt& forStmt)
{
  debug("<for_stmt>");
  //now check the for loop
  eat(FOR, "expecting a for ");
  forStmt.var_id = curr_token;
  eat(ID, "expecting an id ");
  eat(ASSIGN, "expecting a '=' ");
  Expr* startEx = new Expr();
  forStmt.start = startEx;
  expr(*startEx);
  eat(TO, "expecting a to ");
  Expr* endEx = new Expr();
  forStmt.end = endEx;
  expr(*endEx);
  eat(DO, "expecting a do ");
  stmts(forStmt.stmts);
  eat(END, "expecting an end ");
}

//need too check if functions are being called
void Parser::call_expr(CallExpr& callExpr2)
{
  debug("<call_expr>");
  //we have gotten a statement that need parens
  eat(LPAREN, "expecting a '(' ");
  args(callExpr2.arg_list);
  eat(RPAREN, "expecting a ')' here");
}

//check for arguments in a function
void Parser::args(list<Expr*>& arg_list)
{
  if (curr_token.type() == NOT) {
    Expr* expr2 = new Expr();
    arg_list.push_back(expr2);
    expr(*expr2);
    while (curr_token.type() == COMMA) {
      eat(COMMA, "expecting a comma ");
      Expr* exprRec = new Expr();
      arg_list.push_back(exprRec);
      expr(*exprRec);
    }
  }
  //expr need parens
  else if (curr_token.type() == LPAREN) {
    Expr* expr2 = new Expr();
    arg_list.push_back(expr2);
    expr(*expr2);
    while (curr_token.type() == COMMA) {
      eat(COMMA, "expecting a comma ");
      Expr* exprRec = new Expr();
      arg_list.push_back(exprRec);
      expr(*exprRec);
    }
  }
  //check the type for simpleterm
  else if (curr_token.type() == ID || curr_token.type() == NIL || 
           curr_token.type() == NEW || curr_token.type() == NEG ||
           curr_token.type() == INT_VAL ||curr_token.type() == DOUBLE_VAL ||
           curr_token.type() == BOOL_VAL || curr_token.type() == CHAR_VAL ||
           curr_token.type() == STRING_VAL || curr_token.type() == POINTER_VAL
           || curr_token.type() == POINTER_TYPE) {
    Expr* expr2 = new Expr();
    arg_list.push_back(expr2);
    expr(*expr2);
    while (curr_token.type() == COMMA) {
      eat(COMMA, "expecting a comma ");
      Expr* exprRec = new Expr();
      arg_list.push_back(exprRec);
      expr(*exprRec);
    }
  }

  /* 
  //we need to get the arguments of a function call
  if (curr_token.type()) {
    Expr* expr2 = new Expr();
    arg_list.push_back(expr2);
    expr(*expr2);
    while (curr_token.type() == COMMA) {
      eat(COMMA, "expecting a comma ");
      Expr* exprRec = new Expr();
      arg_list.push_back(exprRec);
      expr(*exprRec);
    }
  }*/
}

//now we need to return from a function
void Parser::exit_stmt(ReturnStmt& returnStmt)
{
  debug("<exit_stmt>");
  //return statement that create an expr
  eat(RETURN, "expecting a return "); 
  Expr* expr2 = new Expr();
  returnStmt.expr = expr2;
  expr(*expr2);
}

void Parser::expr(Expr& exprHead)
{
  debug("<expr>");
  //now we need to check the kind of expression we have
  if (curr_token.type() == NOT) {
    exprHead.negated = true;
    eat(NOT, "expecting a '!' ");
    ComplexTerm* complexTerm = new ComplexTerm();
    exprHead.first = complexTerm;
    Expr* expr2 = new Expr();
    complexTerm->expr = expr2;
    expr(*expr2);
  }
  //expr need parens
  else if (curr_token.type() == LPAREN) {
    eat(LPAREN, "expecting a '(' ");
    ComplexTerm* complexTerm = new ComplexTerm();
    exprHead.first = complexTerm;
    Expr* expr2 = new Expr();
    complexTerm->expr = expr2;
    expr(*expr2);
    eat(RPAREN, "expecting a ')' ");
  }
  //check the type for simpleterm
  else if (curr_token.type() == ID || curr_token.type() == NIL || 
           curr_token.type() == NEW || curr_token.type() == NEG ||
           curr_token.type() == INT_VAL ||curr_token.type() == DOUBLE_VAL ||
           curr_token.type() == BOOL_VAL || curr_token.type() == CHAR_VAL ||
           curr_token.type() == STRING_VAL || curr_token.type() == POINTER_TYPE) {
    SimpleTerm* simpleTerm = new SimpleTerm();
    exprHead.first = simpleTerm;
    rvalue(*simpleTerm);
  }
  else if (curr_token.type() == POINTER_VAL) {
    SimpleTerm* simpleTerm = new SimpleTerm();
    exprHead.first = simpleTerm;
    rvalue(*simpleTerm);
  }
  //get operator
  op(exprHead);
}

//check for all different operations
void Parser::op(Expr& exprHead) 
{
  debug("<op>");
  //we need to get the operator
  //inoder to do so, we have to create a new token of that operator
  if (curr_token.type() == PLUS) {
    exprHead.op = new Token(PLUS, curr_token.lexeme(), curr_token.line(), curr_token.column());
    //exprHead.op = &curr_token;
    eat(PLUS, "expecting a '+' ");
  }
  else if (curr_token.type() == MINUS) {
    exprHead.op = new Token(MINUS, curr_token.lexeme(), curr_token.line(), curr_token.column());
    //exprHead.op = &curr_token;
    eat(MINUS, "expecting a '-' ");
  }
  else if (curr_token.type() == DIVIDE) {
    exprHead.op = new Token(DIVIDE, curr_token.lexeme(), curr_token.line(), curr_token.column());
   // exprHead.op = &curr_token;
    eat(DIVIDE, "expecting a '/' ");
  }
  else if (curr_token.type() == MULTIPLY) {
    exprHead.op = new Token(MULTIPLY, curr_token.lexeme(), curr_token.line(), curr_token.column());
   // exprHead.op = &curr_token;
    eat(MULTIPLY, "expecting a '*' ");
  }
  else if (curr_token.type() == MODULO) {
    exprHead.op = new Token(MODULO, curr_token.lexeme(), curr_token.line(), curr_token.column());
   // exprHead.op = &curr_token;
    eat(MODULO, "expecting a '%' ");
  }
  else if (curr_token.type() == AND) { 
    exprHead.op = new Token(AND, curr_token.lexeme(), curr_token.line(), curr_token.column());
   // exprHead.op = &curr_token;
    eat(AND, "expecting a '&&' ");
  }
  else if (curr_token.type() == OR) { 
    exprHead.op = new Token(OR, curr_token.lexeme(), curr_token.line(), curr_token.column());
    //exprHead.op = &curr_token;
    eat(OR, "expecting a '||' ");
  }
  else if (curr_token.type() == EQUAL) { 
    exprHead.op = new Token(EQUAL, curr_token.lexeme(), curr_token.line(), curr_token.column());
   // exprHead.op = &curr_token;
    eat(EQUAL, "expecting a '==' ");
  }
  else if (curr_token.type() == LESS) { 
    exprHead.op = new Token(LESS, curr_token.lexeme(), curr_token.line(), curr_token.column());
   // exprHead.op = &curr_token;
    eat(LESS, "expecting a '<' ");
  }
  else if (curr_token.type() == GREATER) { 
    exprHead.op = new Token(GREATER, curr_token.lexeme(), curr_token.line(), curr_token.column());
   // exprHead.op = &curr_token;
    eat(GREATER, "expecting a '>' ");
  }
  else if (curr_token.type() == LESS_EQUAL) { 
    exprHead.op = new Token(LESS_EQUAL, curr_token.lexeme(), curr_token.line(), curr_token.column());
   // exprHead.op = &curr_token;
    eat(LESS_EQUAL, "expecting a '<=' ");
  }
  else if (curr_token.type() == GREATER_EQUAL) { 
    exprHead.op = new Token(GREATER_EQUAL, curr_token.lexeme(), curr_token.line(), curr_token.column());
   // exprHead.op = &curr_token;
    eat(GREATER_EQUAL, "expecting a '>=' ");
  }
  else if (curr_token.type() == NOT_EQUAL) { 
    exprHead.op = new Token(NOT_EQUAL, curr_token.lexeme(), curr_token.line(), curr_token.column());
   // exprHead.op = &curr_token;
    eat(NOT_EQUAL, "expecting a '!=' ");
  }
  else 
    return;
  //get the expression after the operator
  Expr* exprRest = new Expr();
  exprHead.rest = exprRest;
  expr(*exprRest);
}

//these are values that are being assigned to lvalues
void Parser::rvalue(SimpleTerm& simpleTerm)
{
  debug("<rvalue>");
  //we need to find the simple rvalue
  //nil value
  if (curr_token.type() == NIL) {
    SimpleRValue* simpleRValue = new SimpleRValue();
    simpleRValue->value = curr_token;
    simpleTerm.rvalue = simpleRValue;
    eat(NIL, "expecting a nil ");
  }
  //this is a new value
  else if (curr_token.type() == NEW) {
    eat(NEW, "expecting a new ");
    NewRValue* newRValue = new NewRValue();
    newRValue->type_id = curr_token;
    simpleTerm.rvalue = newRValue;
    eat(ID, "expecting an id ");
  }
  //simple id
  else if (curr_token.type() == ID) {
    Token temp = curr_token;
    eat(ID, "expecting an id ");
    if (curr_token.type() == LPAREN) {
      CallExpr* callExpr2 = new CallExpr();
      //callExpr.arg_list.push_back(temp);
      callExpr2->function_id = temp;
      simpleTerm.rvalue = callExpr2;
      call_expr(*callExpr2);
    }
    else {
      IDRValue* idrValue = new IDRValue();
      idrValue->path.push_back(temp);
      while (curr_token.type() == DOT) {
        eat(DOT, "expecting a dot ");
        idrValue->path.push_back(curr_token);
        eat(ID, "expecting an id ");
      }
      simpleTerm.rvalue = idrValue;
    }
  }              
  //negate a value
  else if (curr_token.type() == NEG) {
    eat(NEG, "expecting a neg ");
    NegatedRValue* negatedRValue = new NegatedRValue();
    Expr* exprNeg = new Expr();
    negatedRValue->expr = exprNeg;
    simpleTerm.rvalue = negatedRValue;
    expr(*exprNeg);
  }
  else if (curr_token.type() == POINTER_VAL) {
    PointerValue* pointerValue = new PointerValue();
    pointerValue->pointer = curr_token;
    simpleTerm.rvalue = pointerValue;
    eat(POINTER_VAL, "expecting a pointer val ");
  }
  else if (curr_token.type() == POINTER_TYPE) {
    PointerType* pointerType = new PointerType();
    pointerType->pointer = curr_token;
    simpleTerm.rvalue = pointerType;
    eat(POINTER_TYPE, "expecting a pointer type ");
  }
  else { 
    SimpleRValue* simpleRValue = new SimpleRValue();
    pval(*simpleRValue);
    simpleTerm.rvalue = simpleRValue;
  }
}

//check for data type values
void Parser::pval(SimpleRValue& simpleRValue)
{
  debug("<pval>");
  //get the actual values of a variable
  simpleRValue.value = curr_token;
  if (curr_token.type() == INT_VAL)
    eat(INT_VAL, "expecting an int val ");
  else if (curr_token.type() == DOUBLE_VAL)
    eat(DOUBLE_VAL, "expecting a double val ");
  else if (curr_token.type() == BOOL_VAL)
    eat(BOOL_VAL, "expecting a bool val ");
  else if (curr_token.type() == CHAR_VAL)
    eat(CHAR_VAL, "expecting a char val ");
  else if (curr_token.type() == STRING_VAL){
    eat(STRING_VAL, "expecting a string val ");
  }
  else if (curr_token.type() == POINTER_VAL) {
    eat(POINTER_VAL, "expecting a pointer val");
  }
}

//check for recursive dot and ids
void Parser::idrval()
{
  debug("<idrval>");
  //eat(ID, "expecting an id ");
  while (curr_token.type() == DOT) {
    eat(DOT, "expecting a dot ");
    eat(ID, "expecting an id ");
  }
}

#endif






