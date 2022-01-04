//----------------------------------------------------------------------
// NAME: Weston Averill
// FILE: type_checker.h
// DATE: 3/20/2021
// DESC: Type check mypl programs
//----------------------------------------------------------------------


#ifndef TYPE_CHECKER_H
#define TYPE_CHECKER_H

#include <iostream>
#include "ast.h"
#include "symbol_table.h"


class TypeChecker : public Visitor
{
public:

  // top-level
  void visit(Program& node);
  void visit(FunDecl& node);
  void visit(TypeDecl& node);
  // statements
  void visit(VarDeclStmt& node);
  void visit(AssignStmt& node);
  void visit(ReturnStmt& node);
  void visit(IfStmt& node);
  void visit(WhileStmt& node);
  void visit(ForStmt& node);
  // expressions
  void visit(Expr& node);
  void visit(SimpleTerm& node);
  void visit(ComplexTerm& node);
  // rvalues
  void visit(SimpleRValue& node);
  void visit(NewRValue& node);
  void visit(CallExpr& node);
  void visit(IDRValue& node);
  void visit(NegatedRValue& node);
  void visit(PointerValue& node);
  void visit(PointerType& node);

private:

  // the symbol table 
  SymbolTable sym_table;

  // the previously inferred type
  std::string curr_type;

  // helper to add built in functions
  void initialize_built_in_types();

  // error message
  void error(const std::string& msg, const Token& token);
  void error(const std::string& msg); 

};


void TypeChecker::error(const std::string& msg, const Token& token)
{
  throw MyPLException(SEMANTIC, msg, token.line(), token.column());
}


void TypeChecker::error(const std::string& msg)
{
  throw MyPLException(SEMANTIC, msg);
}

void TypeChecker::initialize_built_in_types()
{
  // print function
  sym_table.add_name("print");
  sym_table.set_vec_info("print", StringVec {"string", "nil"});
  // stoi function
  sym_table.add_name("stoi");
  sym_table.set_vec_info("stoi", StringVec {"string", "int"});  

  // TODO: finish the rest of the built-in functions: stod, itos,
  // dtos, get, length, and read
  sym_table.add_name("stod");
  sym_table.set_vec_info("stod", StringVec {"string", "double"});

  sym_table.add_name("itos");
  sym_table.set_vec_info("itos", StringVec {"int", "string"});

  sym_table.add_name("dtos");
  sym_table.set_vec_info("dtos", StringVec {"double", "string"});

  sym_table.add_name("get");
  sym_table.set_vec_info("get", StringVec {"int","string", "char"});

  sym_table.add_name("length");
  sym_table.set_vec_info("length", StringVec {"string","int"});

  sym_table.add_name("read");
  sym_table.set_vec_info("read", StringVec {"string"});
}


void TypeChecker::visit(Program& node)
{
  // push the global environment
  sym_table.push_environment();
  // add built-in functions
  initialize_built_in_types();
  // push 
  for (Decl* d : node.decls) {
    d->accept(*this);
  }
  // check for a main function
  if (sym_table.name_exists("main") && sym_table.has_vec_info("main")) {
    // TODO: finish checking that the main function is defined with
    // the correct signature
    StringVec sv;
    sym_table.get_vec_info("main", sv);
    std::string return_type = sv[sv.size()-1]; 
    //may need to do someting is with return type
    if (return_type != "int")
      error("incorrect return type for main");
  }
  else {
    // NOTE: the only time the 1-argument version of error should be
    // called!
    error("undefined 'main' function");
  }
   // pop the global environment
  sym_table.pop_environment();
}

void TypeChecker::visit(FunDecl& node) 
{
  if (sym_table.name_exists_in_curr_env(node.id.lexeme())) {
    error("function delcared already", node.id);
  }
  //push the paramaters of function to the vector
  StringVec the_type;
  for (FunDecl::FunParam f : node.params) {
    the_type.push_back(f.type.lexeme());
  }
  //now we need to push the return type
  the_type.push_back(node.return_type.lexeme());
  //now we neeed to add function signature to symbol table
  sym_table.add_name(node.id.lexeme());
  sym_table.set_vec_info(node.id.lexeme(), the_type);

  //add a new environment and a special return name
  sym_table.push_environment();
  sym_table.add_name("return");
  sym_table.set_str_info("return", node.return_type.lexeme());

  //add parameters to environment
  //use map to keep track of the names
  StringMap map;
  for (FunDecl::FunParam v : node.params) {
    //check for duplicate parameter names (semantic error)
    if (map.count(v.id.lexeme()) > 0){
      error("duplicate parameter name", v.id);
    }
     //add param name and types to environemnt
    map[v.id.lexeme()] = v.type.lexeme();
    sym_table.add_name(v.id.lexeme());
    sym_table.set_str_info(v.id.lexeme(), v.type.lexeme());
  }

  //finally, check the body
  for (Stmt* s : node.stmts)
    s->accept(*this);
  
  //cleanup
  sym_table.pop_environment();
}

void TypeChecker::visit(TypeDecl& node) 
{
  StringMap map;
  sym_table.add_name(node.id.lexeme());
  sym_table.push_environment();
  for (VarDeclStmt* v : node.vdecls) {
    v->accept(*this);
    std::string temp = curr_type;
    if (v->type != nullptr) {
      map[v->id.lexeme()] = v->type->lexeme();
    }
    else {
      map[v->id.lexeme()] = temp;
    }
  }
  sym_table.pop_environment();
  sym_table.set_map_info(node.id.lexeme(), map);
}

// statements
void TypeChecker::visit(VarDeclStmt& node) 
{
  node.expr->accept(*this);
  std::string exp_type = curr_type;
  std::string var_name = node.id.lexeme();

  //check for shadowing
  if (sym_table.name_exists_in_curr_env(var_name)) {
    error("redefinition of variable", node.id);
  }
  //check the variable type
  if (node.type != nullptr && node.type->lexeme() != exp_type && exp_type != "nil") {
    error("mismatch type in variable declaration", node.id);
  }
  //etc..
  //add to symbol tables
  if (node.type != nullptr) {
    sym_table.add_name(var_name);
    sym_table.set_str_info(var_name, node.type->lexeme());
  }
  else {
    sym_table.add_name(var_name);
    sym_table.set_str_info(var_name, curr_type);
  }
  
}

void TypeChecker::visit(AssignStmt& node) 
{
  //infer rhs type
  node.expr->accept(*this);
  std::string rhs_type = curr_type;
  //cout << rhs_type << "   ";
  //infer lhs type
  std::string lhs_type;
  std::list<Token>::iterator it = node.lvalue_list.begin();
  if (!sym_table.name_exists(it->lexeme()))
    error("use before defition", *it);
  sym_table.get_str_info(it->lexeme(), curr_type);
  ++it;
  for (; it != node.lvalue_list.end(); ++it) {
    StringMap info;
    sym_table.get_map_info(curr_type, info);
    StringMap::iterator prop = info.find(it->lexeme());
    if (prop == info.end()) {
      error("no member in type", *it);
    }
    curr_type = info[it->lexeme()];
  }
  lhs_type = curr_type;

  //check types: error if the rhs and lhs types don't match
  if (rhs_type != "nil" && lhs_type != rhs_type) {
    //cout << lhs_type << " " << rhs_type;
    std::string msg = "mismatched types in assignment";
    error(msg, node.lvalue_list.front());
  }

}

void TypeChecker::visit(ReturnStmt& node) 
{
  node.expr->accept(*this);
  std::string rt = curr_type;
  std::string funReturn;
  //this gets the functions return type
  sym_table.get_str_info("return", funReturn);
  //if the function doesn't return the correct type, 
  //we need to trhow an error
  if (funReturn != rt && rt != "nil") {
    error("mismatch in return types", node.expr->first_token());
  }
}

void TypeChecker::visit(IfStmt& node) 
{
  sym_table.push_environment();
  node.if_part->expr->accept(*this);          
  std::string ifStmtType = curr_type;
  //check the if statement expression
  if (ifStmtType != "bool") {
    //maybe we can throw error that shows line and column
    error("Non-boolean expression in if statement", node.if_part->expr->first_token());
  }
  //now lets call accept on the if statements statments
  sym_table.push_environment();
  for (Stmt* s : node.if_part->stmts) {
    s->accept(*this);
  }
  sym_table.pop_environment();
  for (BasicIf* i : node.else_ifs) {
    i->expr->accept(*this);
    ifStmtType = curr_type;
    if (ifStmtType != "bool") {
      //try to throw better error with line and column
      error("Non-boolean expression in if statement", i->expr->first_token());
    }
    sym_table.push_environment();
    for (Stmt* s : i->stmts) {
      s->accept(*this);
    }
    sym_table.pop_environment();
  }
  sym_table.push_environment();
  for (Stmt* s : node.body_stmts) {
    s->accept(*this);
  }
  sym_table.pop_environment();
  sym_table.pop_environment();
}

void TypeChecker::visit(WhileStmt& node) 
{
  sym_table.push_environment();
  node.expr->accept(*this);
  std::string whileStmtType = curr_type;
  //check to see if its a bool expression
  if (whileStmtType != "bool") {
    //throw error if not
    error("Non-boolean expression in while statement", node.expr->first_token());
  }

  //now check each stmt in while loop
  sym_table.push_environment();
  for (Stmt* s : node.stmts) {
    s->accept(*this);
  }
  sym_table.pop_environment();
  sym_table.pop_environment();
}

void TypeChecker::visit(ForStmt& node) 
{
  sym_table.push_environment();

  sym_table.add_name(node.var_id.lexeme());
  if (node.start != nullptr) {
    node.start->accept(*this);
  }
  std::string strt = curr_type;
  sym_table.set_str_info(node.var_id.lexeme(), strt);
  if (node.end != nullptr) {
    node.end->accept(*this);
  }
  std::string nd = curr_type;

  if (strt != nd) {
    error("mismatch types in for statement", node.var_id);
  }
  sym_table.push_environment();
  //now check the body of the for stmt
  for (Stmt* s : node.stmts) {
    s->accept(*this);
  }
  sym_table.pop_environment();
  sym_table.pop_environment();
}

  // expressions
void TypeChecker::visit(Expr& node) 
{
  //check the first exists
  if (node.first != nullptr)
    node.first->accept(*this);
  std::string firstType = curr_type;
  std::string restType;
  //check if not an operator
  if(node.op == nullptr) {
    //rule 23 i think
    if (node.negated && firstType != "bool") {
      error("expecting a boolean expression", node.first_token());
    }
  }
  //now check if there is an operator,
  //if there is we have a complex expression
  if (node.op != nullptr) {
    if (node.rest == nullptr) {
      error("no finish to expression", *node.op);
    }
    node.rest->accept(*this);
    restType = curr_type;
    //rule 2
    if (node.op->lexeme() == "+" || node.op->lexeme() == "-" ||
    node.op->lexeme() == "*" || node.op->lexeme() == "/") {
      if ((firstType == "int" && restType == "double")
      || (firstType == "double" && restType == "int")) {
        error("can't do operations on ints and doubles", *node.op);
      }
      else if (firstType == restType && firstType == "int") {
        curr_type = "int";
      }
      else if (firstType == restType && firstType == "double") {
        curr_type = "double";
      }
      else if (firstType == "int" && (restType == "char" || restType == "string" || restType == "bool")) {
        error("mismatch in types in " + node.op->lexeme(), node.first_token());
      }
      else if (firstType == "double" && (restType == "char" || restType == "string" || restType == "bool")) {
        error("mismatch in types in " + node.op->lexeme(), node.first_token());
      }
      else if (firstType == "char" && (restType == "double" || restType == "bool" || restType == "int")) {
        error("mismatch in types in " + node.op->lexeme(), node.first_token());
      }
      else if (firstType == "string" && (restType == "double" || restType == "bool" || restType == "int")) {
        error("mismatch in types in " + node.op->lexeme(), node.first_token());
      }
    }
    //rule 4
    if (node.op->lexeme() == "%" && (firstType != "int" || restType != "int")) {
      error("use of modulo without int", *node.op);
    }
    //rule5
    else if (node.op->lexeme() == "+" && ((firstType == "string" && restType == "char") 
    || (firstType == "char" && restType == "string"))) {
      curr_type = "string";
    }
    else if (node.op->lexeme() == "+" && firstType == restType && 
    (firstType == "char" || firstType == "string")) {
      curr_type = "string";
    }
    //rule 18 & 19
    else if ((firstType == "nil" && restType != "nil") || (firstType != "nil" && restType == "nil") 
    && (node.op->lexeme() == "==" || node.op->lexeme() == "!=")) {
      curr_type = "bool";
    }
    //rule 17
    else if (firstType == restType && (node.op->lexeme() == "!=" || node.op->lexeme() == "==")) {
      curr_type = "bool";
    }
    else if (firstType != restType && (node.op->lexeme() == "!=" || node.op->lexeme() == "==")) {
      error("mismatch types", node.first_token());
    }
    //rule 20
    else if (firstType == restType && firstType != "bool" && firstType != "nil"
    && (node.op->lexeme() == "<" || node.op->lexeme() == ">"
    || node.op->lexeme() == "<=" || node.op->lexeme() == ">=")){
      curr_type = "bool";
    }
    else if (firstType != restType && firstType != "nil" && restType != "nil" &&
    firstType != "bool" && restType != "bool" && (node.op->lexeme() == "<" || 
    node.op->lexeme() == ">" || node.op->lexeme() == "<=" || node.op->lexeme() == ">=")){
      error("mismatched types", node.first_token());
    }
    //rule 24
    else if ((node.op->lexeme() == "and" || node.op->lexeme() == "or") && 
    (firstType != "bool" || restType != "bool")) {
      error("expecting a boolean", node.first_token());
    }
  }
}

void TypeChecker::visit(SimpleTerm& node) 
{
  if (node.rvalue != nullptr) 
    node.rvalue->accept(*this);
}

void TypeChecker::visit(ComplexTerm& node) 
{
  if (node.expr != nullptr)
    node.expr->accept(*this);
}
  // rvalues
void TypeChecker::visit(SimpleRValue& node) 
{
  // infer type based on token type
  if (node.value.type() == CHAR_VAL)
    curr_type = "char" ;
  else if (node.value.type() == STRING_VAL)
    curr_type = "string" ;
  else if (node.value.type() == INT_VAL)
    curr_type = "int" ;
  else if (node.value.type() == DOUBLE_VAL)
    curr_type = "double" ;
  else if (node.value.type() == BOOL_VAL)
    curr_type = "bool" ;
  else if (node.value.type() == NIL)
    curr_type = "nil" ;

}
void TypeChecker::visit(NewRValue& node) 
{
  if (!sym_table.name_exists(node.type_id.lexeme())) {
    error("no matching types", node.type_id);
  }
  curr_type = node.type_id.lexeme();
}

void TypeChecker::visit(CallExpr& node) 
{
  //check to make sure the function exists,
  //if its doesn't throw error
  if (!sym_table.has_vec_info(node.function_id.lexeme())) {
    error("no function defined", node.function_id);
  }
  //now we have to get the function signature
  std::string fun_name = node.function_id.lexeme();
  StringVec fun_type;
  sym_table.get_vec_info(fun_name, fun_type);

  //check thre are enough args
  if (fun_type.size()-1 > node.arg_list.size()) {
    error("not enough args", node.function_id);
  }
  else if (fun_type.size()-1 < node.arg_list.size()) {
    error("too many args given", node.function_id);
  }

  std::string temp;
  int i = 0;
  //go through each argument
  for (Expr* e : node.arg_list) {
    e->accept(*this);
    temp = curr_type;
    //cout << node.function_id.lexeme();
    //cout << temp;
    //now check to make sure the types align
    if (temp != "nil" && temp != fun_type[i]) { 
      //cout << fun_type[i];
      error("parameter types do not match for function call", node.function_id);
    }
    i++;
  }
  curr_type = fun_type[fun_type.size()-1];
}

void TypeChecker::visit(IDRValue& node) 
{
  std::list<Token>::iterator it = node.path.begin();
  if (!sym_table.name_exists(it->lexeme()))
    error("use before defition", *it);
  sym_table.get_str_info(it->lexeme(), curr_type);
  ++it;
  for (; it != node.path.end(); ++it) {
    StringMap info;
    sym_table.get_map_info(curr_type, info);
    StringMap::iterator temp = info.find(it->lexeme());
    if (temp == info.end()) {
      error("no member in type", *it);
    }
    curr_type = info[it->lexeme()];
  }
}

void TypeChecker::visit(NegatedRValue& node) 
{
  if (node.expr != nullptr)
    node.expr->accept(*this);
  //rule3
  if (curr_type != "int" && curr_type != "double")
    error("can't negate", node.expr->first_token());
  
}

void TypeChecker::visit(PointerType& node) 
{
  std::string str = node.pointer.lexeme();
  if (!sym_table.name_exists(str)) {
    error("this has not been declared yet ", node.first_token());
  }
  sym_table.get_str_info(str, curr_type);
}

void TypeChecker::visit(PointerValue& node)
{
  std::string str = node.pointer.lexeme();
  str = str.substr(1);
  if (!sym_table.name_exists_in_curr_env(str)) {
    error("this variable has not been declared yet ", node.first_token());
  }
  sym_table.get_str_info(str, curr_type);
}

#endif
