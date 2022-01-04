//----------------------------------------------------------------------
// NAME: Weston Averill
// FILE: interpreter.h
// DATE: 3/26/2021
// DESC: interpreter for mypl
//----------------------------------------------------------------------


#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <iostream>
#include <unordered_map>
#include <regex>
#include "ast.h"
#include "symbol_table.h"
#include "data_object.h"
#include "heap.h"


class Interpreter : public Visitor
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
  void visit(PointerType& node);
  void visit(PointerValue& node);

  // return code from calling main
  int return_code() const;
  bool debugFlag = false;
  
private:

  // return exception
  class MyPLReturnException : public std::exception {};
  
  // the symbol table 
  SymbolTable sym_table;

  // holds the previously computed value
  DataObject curr_val;

  // the heap
  Heap heap;

  // the next oid
  size_t next_oid = 0;
  
  // the functions (all within the global environment)
  std::unordered_map<std::string,FunDecl*> functions;
  
  // the user-defined types (all within the global environment)
  std::unordered_map<std::string,TypeDecl*> types;
  std::unordered_map<std::string, tuple<std::string,DataObject>> intAddress;

  // the global environment id
  int global_env_id = 0;
  
  // the program return code
  int ret_code = 0;

  // error message
  void error(const std::string& msg, const Token& token);
  void error(const std::string& msg); 
  void debug(std::string msg);
  std::string currPtr;
};



int Interpreter::return_code() const
{
  return ret_code;
}

void Interpreter::error(const std::string& msg, const Token& token)
{
  throw MyPLException(RUNTIME, msg, token.line(), token.column());
}


void Interpreter::error(const std::string& msg)
{
  throw MyPLException(RUNTIME, msg);
}

void Interpreter::debug(std::string msg)
{
  if (debugFlag)
    std::cout << msg << std::endl;
}


// TODO: finish the visitor functions
void Interpreter::visit(Program& node) 
{
  debug("<program>");
  //push the global 
  sym_table.push_environment();
  
  //store the global environment id
  global_env_id = sym_table.get_environment_id();

  for (Decl* d : node.decls) {
    d->accept(*this);
  }

  //execute the main function
  CallExpr expr;
  expr.function_id = functions["main"]->id;
  expr.accept(*this);

  //pop the global environment
  sym_table.pop_environment();
}

void Interpreter::visit(FunDecl& node) 
{
  debug("<FunDecl>");
  //global_env_id = sym_table.get_environment_id();
  FunDecl* fun = new FunDecl();
  *fun = node;
  functions[node.id.lexeme()] = fun;
}

void Interpreter::visit(TypeDecl& node) 
{
  debug("<TypeDecl>");
  TypeDecl* d = new TypeDecl();
  *d = node;
  types[node.id.lexeme()] = d;
}
  // statements
void Interpreter::visit(VarDeclStmt& node) 
{
  debug("<VarDeclStmt>");
  if (node.expr != nullptr) {
    node.expr->accept(*this);
  }
  sym_table.add_name(node.id.lexeme());
  //cout << curr_val.to_string() << endl;
  sym_table.set_val_info(node.id.lexeme(), curr_val);
  DataObject test;
  sym_table.get_val_info(node.id.lexeme(), test);
  //cout << node.id.lexeme() << " is " << test.to_string() << endl;
  if (node.pointer) {
    intAddress.insert({node.id.lexeme(), make_tuple(currPtr, curr_val)});
  }
}
void Interpreter::visit(AssignStmt& node) 
{
  debug("<AssignStmt>");
  //get rhs
  if (node.expr != nullptr) {
    node.expr->accept(*this);
  }
  DataObject rhs = curr_val;
  //check if path is size 1
  if (node.lvalue_list.size() == 1) {
    sym_table.set_val_info(node.lvalue_list.front().lexeme(), rhs);
  }

  //this means path is greater than 1
  else { 
    HeapObject obj;
    std::list<Token>::iterator it = node.lvalue_list.begin();
    sym_table.get_val_info(it->lexeme(), curr_val);
    ++it;
    while (it != node.lvalue_list.end()) {
      size_t oid;
      curr_val.value(oid);
      if (heap.has_obj(oid)) {
        heap.get_obj(oid, obj);
        obj.get_val(it->lexeme(), curr_val);
      }
      else if (it != node.lvalue_list.end()) {
        error("no attribute name", *it);
      }
      obj.set_att(it->lexeme(), curr_val);
      it++;
    }
    obj.set_att(it->lexeme(), rhs);
  }
  
  unordered_map<std::string, tuple<std::string, DataObject>>::iterator itr = intAddress.begin();
  while (itr != intAddress.end()) {
    if (get<0>(itr->second) == node.lvalue_list.front().lexeme()) {
      get<1>(itr->second) = rhs;
    }
    itr++;
  }

  //i think the problem is with the environemnt
  if (intAddress.count(node.lvalue_list.front().lexeme()) > 0) {
    //intAddress[node.lvalue_list.front().lexeme()] = make_tuple(currPtr, rhs);
    intAddress[node.lvalue_list.front().lexeme()] = make_tuple(get<0>(intAddress[node.lvalue_list.front().lexeme()]), rhs);
    //sym_table.set_val_info(currPtr, rhs);
    sym_table.set_val_info(get<0>(intAddress[node.lvalue_list.front().lexeme()]), rhs);
  }
}
void Interpreter::visit(ReturnStmt& node) 
{
  debug("<ReturnStmt>");
  //evaluate the expression

  //check this first
  if (node.expr != nullptr) {
    node.expr->accept(*this);
  }
  //cout << " here in return statmtnju";
  //throw the return exception
  throw new MyPLReturnException;
}
void Interpreter::visit(IfStmt& node) 
{
  debug("<IfStmt>");
  node.if_part->expr->accept(*this);
  bool cond;
  //bool found = false;
  curr_val.value(cond);
  if (cond) {
    for (Stmt* s : node.if_part->stmts) {
      s->accept(*this);
    }
  }
  else if (!node.else_ifs.empty()) {
    for (BasicIf* b : node.else_ifs) {
      if (!cond) {
        b->expr->accept(*this);
        curr_val.value(cond);
        if (cond) {
          for (Stmt* s : b->stmts) {
            s->accept(*this);
          }
        }
      }
    }
  }
  if (!cond) {
    for (Stmt* s : node.body_stmts) {
      s->accept(*this);
    }
  }
}

void Interpreter::visit(WhileStmt& node) 
{
  debug("<WhileStmt>");
  if (node.expr != nullptr) {
    node.expr->accept(*this);
  }
  bool cond = false;
  curr_val.value(cond);
  while (cond) {
    for (Stmt* s : node.stmts) {
      s->accept(*this);
    }
    node.expr->accept(*this);
    curr_val.value(cond);
  }
}

void Interpreter::visit(ForStmt& node) 
{
  debug("<ForStmt>");
  //push environment
  sym_table.push_environment();

  sym_table.add_name(node.var_id.lexeme());

  //get the curr_val of start expr
  if (node.start != nullptr) {
    node.start->accept(*this);
  }
  int start_val;
  curr_val.value(start_val);

  //get the curr_val of the end expr
  if (node.end != nullptr) {
    node.end->accept(*this);
  }
  int rest_val;
  curr_val.value(rest_val);

  DataObject loop = new DataObject(start_val);
  sym_table.set_val_info(node.var_id.lexeme(), loop);

  //keep looping if the expr is true
  //loop.value(start_val);
  while (start_val < rest_val) {
    sym_table.push_environment();
    for (Stmt* s : node.stmts) {
      s->accept(*this);
    }
    sym_table.pop_environment();
    //update the curr_val
    sym_table.get_val_info(node.var_id.lexeme(), loop);
    loop.value(start_val);
    start_val++;
    loop.set(start_val);
    sym_table.set_val_info(node.var_id.lexeme(), loop);
  }
  sym_table.pop_environment();
}
  // expressions
void Interpreter::visit(Expr& node) 
{
  debug("<Expr>");
  if (node.negated) {
    node.first->accept(*this);
    bool val;
    curr_val.value(val);
    curr_val.set(!val);
  }
  else {
    node.first->accept(*this);
    if (node.op) {
      DataObject lhs_val = curr_val;
      node.rest->accept(*this);
      DataObject rhs_val = curr_val;
      TokenType op = node.op->type();
      //cout << node.first_token().to_string() << " at first " << endl;
      //start checking various cases (there are many)

      //be sure to set the computed value in curr_val

      //need to go throughmath operators (+, -, *, /, %)
      if (op == PLUS) {
        if (lhs_val.is_nil()) {
          error("cant do operation on nil value", node.first_token());
        }
        if (rhs_val.is_nil()) {
          error("cant do operation on nil value", node.first_token());
        }
        //add two ints
        if (lhs_val.is_integer()) {
          //cout << "here " << endl;
          int l_val = 0;
          lhs_val.value(l_val);
          int r_val = 0;
          rhs_val.value(r_val);
          curr_val.set(l_val + r_val);
        }
        //adding two doubles
        else if (lhs_val.is_double()) {
          double l_val = 0.0;
          lhs_val.value(l_val);
          double r_val = 0.0;
          rhs_val.value(r_val);
          curr_val.set(l_val + r_val);
        }
        //adding two chars, need to make a string
        else if (lhs_val.is_char() && rhs_val.is_char()) {
          std::string str = "";
          char l_val;
          lhs_val.value(l_val);
          char r_val;
          rhs_val.value(r_val);
          str += l_val;
          str += r_val;
          curr_val.set(str);
        }
        //adding two strings together
        else if (lhs_val.is_string() && rhs_val.is_string()) {
          std::string l_val = "";
          std::string r_val = "";
          lhs_val.value(l_val);
          rhs_val.value(r_val);
          curr_val.set(l_val+r_val);
        }
        //lhs is string, rhs is char
        else if (lhs_val.is_string() && rhs_val.is_char()) {
          std::string l_val = "";
          char r_val;
          lhs_val.value(l_val);
          rhs_val.value(r_val);
          l_val += r_val;
          curr_val.set(l_val);
        }
        //lhs is a char, rhs is a string
        else if (lhs_val.is_char() && rhs_val.is_string()) {
          char l_val;
          std::string r_val = "";
          lhs_val.value(l_val);
          rhs_val.value(r_val);
          curr_val.set(l_val+r_val);
        }
      }
      else if (op == MINUS) {
        if (lhs_val.is_nil() || rhs_val.is_nil()) {
          error("cant do minus operation on a nil value", node.first_token());
        }
        //subtraction of integers
        if (lhs_val.is_integer()) {
          int l_val;
          int r_val;
          lhs_val.value(l_val);
          rhs_val.value(r_val);
          curr_val.set(l_val - r_val);
        }
        //subtraction of doubles
        else if (lhs_val.is_double()) {
          double l_val;
          double r_val;
          lhs_val.value(l_val);
          rhs_val.value(r_val);
          curr_val.set(l_val - r_val);
        }
      }
      else if (op == MULTIPLY) {
        if (lhs_val.is_nil() || rhs_val.is_nil()) {
          error("cant do multiplication operation on a nil value", node.first_token());
        }
        //multiplication of integers
        if (lhs_val.is_integer()) {
          int l_val;
          int r_val;
          lhs_val.value(l_val);
          rhs_val.value(r_val);
          curr_val.set(l_val * r_val);
        }
        //multiplication of doubles
        else if (lhs_val.is_double()) {
          double l_val;
          double r_val;
          lhs_val.value(l_val);
          rhs_val.value(r_val);
          curr_val.set(l_val * r_val);
        }
      }
      else if (op == DIVIDE) {
        if (lhs_val.is_nil() || rhs_val.is_nil()) {
          error("cant do division operation on a nil value", node.first_token());
        }
        //division of integers
        if (lhs_val.is_integer()) {
          int l_val;
          int r_val;
          lhs_val.value(l_val);
          rhs_val.value(r_val);
          curr_val.set(l_val / r_val);
        }
        //division of doubles
        else if (lhs_val.is_double()) {
          double l_val;
          double r_val;
          lhs_val.value(l_val);
          rhs_val.value(r_val);
          curr_val.set(l_val / r_val);
        }
      }
      else if (op == MODULO) {
        if (lhs_val.is_nil() || rhs_val.is_nil()) {
          error("cant do modulo operation on a nil value", node.first_token());
        }
        //modulo of integers
        if (lhs_val.is_integer()) {
          int l_val;
          int r_val;
          lhs_val.value(l_val);
          rhs_val.value(r_val);
          curr_val.set(l_val % r_val);
        }
      }
      //and operation
      else if (op == AND) {
        if (lhs_val.is_nil() || rhs_val.is_nil()) {
          error("can't use AND with a nil", node.first_token());
        }
        bool l_val;
        bool r_val;
        lhs_val.value(l_val);
        rhs_val.value(r_val);
        curr_val.set(l_val && r_val);
      }
      else if (op == OR) {
        if (lhs_val.is_nil() || rhs_val.is_nil()) {
          error("can't use OR with a nil", node.first_token());
        }
        bool l_val;
        bool r_val;
        lhs_val.value(l_val);
        rhs_val.value(r_val);
        curr_val.set(l_val || r_val);
      }
      //now we need to look at relational operators (=, !=, <, >, <=, >=)
      
      //operator equal to
      else if (op == EQUAL) {
        if (lhs_val.is_nil() ^ rhs_val.is_nil()) {
          curr_val.set(false);
        }
        else if (lhs_val.is_nil() && rhs_val.is_nil()) {
          curr_val.set(true);
        }
        //now check they are equal
        else {
          if (lhs_val.to_string() == rhs_val.to_string()) {
            curr_val.set(true);
          }
          else {
            curr_val.set(false);
          }
        }
      }
      //operator not equal
      else if (op == NOT_EQUAL) {
        if (lhs_val.is_nil() ^ rhs_val.is_nil()) {
          curr_val.set(true);
        }
        else if (lhs_val.is_nil() && rhs_val.is_nil()) {
          curr_val.set(false);
        }
        //now check they are equal
        else {
          if (lhs_val.to_string() == rhs_val.to_string()) {
            curr_val.set(false);
          }
          else {
            curr_val.set(true);
          }
        }
      }
      //operator less than
      else if (op == LESS) {
        if (lhs_val.is_nil() || rhs_val.is_nil()) {
          error("cant do operation on nil", node.first_token());
        }
        else if (lhs_val.is_integer()) {
          int l_val;
          int r_val;
          lhs_val.value(l_val);
          rhs_val.value(r_val);
          if (l_val < r_val) {
            curr_val.set(true);
          }
          else {
            curr_val.set(false);
          }
        }
        else if (lhs_val.is_double()) {
          double l_val;
          double r_val;
          lhs_val.value(l_val);
          rhs_val.value(r_val);
          if (l_val < r_val) {
            curr_val.set(true);
          }
          else {
            curr_val.set(false);
          }
        }
        else if (lhs_val.is_char()) {
          char l_val;
          char r_val;
          lhs_val.value(l_val);
          rhs_val.value(r_val);
          if (l_val < r_val) {
            curr_val.set(true);
          }
          else {
            curr_val.set(false);
          }
        }
        else if (lhs_val.is_string()) {
          std::string l_val;
          std::string r_val;
          lhs_val.value(l_val);
          rhs_val.value(r_val);
          if (l_val < r_val) {
            curr_val.set(true);
          }
          else {
            curr_val.set(false);
          }
        }
      }
      //operator less than or equal
      else if (op == LESS_EQUAL) {
        if (lhs_val.is_nil() || rhs_val.is_nil()) {
          error("cant do operation on nil", node.first_token());
        }
        else if (lhs_val.is_integer()) {
          int l_val;
          int r_val;
          lhs_val.value(l_val);
          rhs_val.value(r_val);
          if (l_val <= r_val) {
            curr_val.set(true);
          }
          else {
            curr_val.set(false);
          }
        }
        else if (lhs_val.is_double()) {
          double l_val;
          double r_val;
          lhs_val.value(l_val);
          rhs_val.value(r_val);
          if (l_val <= r_val) {
            curr_val.set(true);
          }
          else {
            curr_val.set(false);
          }
        }
        else if (lhs_val.is_char()) {
          char l_val;
          char r_val;
          lhs_val.value(l_val);
          rhs_val.value(r_val);
          if (l_val <= r_val) {
            curr_val.set(true);
          }
          else {
            curr_val.set(false);
          }
        }
        else if (lhs_val.is_string()) {
          std::string l_val;
          std::string r_val;
          lhs_val.value(l_val);
          rhs_val.value(r_val);
          if (l_val <= r_val) {
            curr_val.set(true);
          }
          else {
            curr_val.set(false);
          }
        }
      }
      //operator greater than
      else if (op == GREATER) {
        if (lhs_val.is_nil() || rhs_val.is_nil()) {
          error("cant do operation on nil", node.first_token());
        }
        else if (lhs_val.is_integer()) {
          int l_val;
          int r_val;
          lhs_val.value(l_val);
          rhs_val.value(r_val);
          if (l_val > r_val) {
            curr_val.set(true);
          }
          else {
            //cout << "hrer";
            curr_val.set(false);
          }
        }
        else if (lhs_val.is_double()) {
          double l_val;
          double r_val;
          lhs_val.value(l_val);
          rhs_val.value(r_val);
          if (l_val > r_val) {
            curr_val.set(true);
          }
          else {
            curr_val.set(false);
          }
        }
        else if (lhs_val.is_char()) {
          char l_val;
          char r_val;
          lhs_val.value(l_val);
          rhs_val.value(r_val);
          if (l_val > r_val) {
            curr_val.set(true);
          }
          else {
            curr_val.set(false);
          }
        }
        else if (lhs_val.is_string()) {
          std::string l_val;
          std::string r_val;
          lhs_val.value(l_val);
          rhs_val.value(r_val);
          if (l_val > r_val) {
            curr_val.set(true);
          }
          else {
            curr_val.set(false);
          }
        }
      }
      //operator greater than or equal
      else if (op == GREATER_EQUAL) {
        if (lhs_val.is_nil() || rhs_val.is_nil()) {
          error("cant do operation on nil", node.first_token());
        }
        else if (lhs_val.is_integer()) {
          int l_val;
          int r_val;
          lhs_val.value(l_val);
          rhs_val.value(r_val);
          if (l_val >= r_val) {
            curr_val.set(true);
          }
          else {
            curr_val.set(false);
          }
        }
        else if (lhs_val.is_double()) {
          double l_val;
          double r_val;
          lhs_val.value(l_val);
          rhs_val.value(r_val);
          if (l_val >= r_val) {
            curr_val.set(true);
          }
          else {
            curr_val.set(false);
          }
        }
        else if (lhs_val.is_char()) {
          char l_val;
          char r_val;
          lhs_val.value(l_val);
          rhs_val.value(r_val);
          if (l_val >= r_val) {
            curr_val.set(true);
          }
          else {
            curr_val.set(false);
          }
        }
        else if (lhs_val.is_string()) {
          std::string l_val;
          std::string r_val;
          lhs_val.value(l_val);
          rhs_val.value(r_val);
          if (l_val >= r_val) {
            curr_val.set(true);
          }
          else {
            curr_val.set(false);
          }
        }
      }
    }
  }
}
void Interpreter::visit(SimpleTerm& node) 
{
  debug("<SimpleTerm>");
  if (node.rvalue != nullptr) {
    node.rvalue->accept(*this);
  }
}
void Interpreter::visit(ComplexTerm& node) 
{
  debug("<ComplexTerm>");
  if (node.expr != nullptr) {
    node.expr->accept(*this);
  }
}
  // rvalues
void Interpreter::visit(SimpleRValue& node) 
{
  debug("<SimpleRValue>");
  if (node.value.type() == CHAR_VAL)
    curr_val.set(node.value.lexeme().at(0));
  else if (node.value.type() == STRING_VAL) 
    curr_val.set(node.value.lexeme());
  else if (node.value.type() == INT_VAL) {
    try {
      curr_val.set(std::stoi(node.value.lexeme()));
    }
    catch (const std::invalid_argument& e) {
      error("internal error", node.value);
    }
    catch (const std::out_of_range& e) {
      error("int out of range", node.value);
    }
  }
  else if (node.value.type() == DOUBLE_VAL) {
    try {
      curr_val.set(std::stod(node.value.lexeme()));
    }
    catch (const std::invalid_argument& e) {
      error("internal error", node.value);
    }
    catch (const std::out_of_range& e) {
      error("double out of range", node.value);
    }
  }
  else if (node.value.type() == BOOL_VAL) {
    if (node.value.lexeme() == "true")
      curr_val.set(true);
    else 
      curr_val.set(false);
  }
  else if (node.value.type() == NIL) 
    curr_val.set_nil();
}

void Interpreter::visit(NewRValue& node) 
{
  debug("<NewRValue>");
  HeapObject h;
  //build the heap object
  //look up in types array
  size_t oid = next_oid;
  TypeDecl *t = types[node.type_id.lexeme()];
  for (VarDeclStmt* s : t->vdecls) {
    if (s->expr != nullptr) {
      s->expr->accept(*this);
    }
    else {
      curr_val = nullptr;
    }
    h.set_att(s->id.lexeme(), curr_val);
  }
  heap.set_obj(oid, h);
  curr_val.set(oid);
  next_oid++;
  sym_table.add_name(node.type_id.lexeme());
  sym_table.set_val_info(node.type_id.lexeme(), curr_val);
}

void Interpreter::visit(CallExpr& node) 
{
  debug("<CallExpr>");
  std::string fun_name = node.function_id.lexeme();
  // check for built - in functions
  if (fun_name == "print") {
    node.arg_list.front()->accept(*this);
    std::string s = curr_val.to_string();
    s = std::regex_replace(s, std::regex("\\\\n"), "\n");
    s = std::regex_replace(s, std::regex("\\\\t"), "\t");
    std::cout << s;
  }
 
  else if (fun_name == "stoi") {
    node.arg_list.front()->accept(*this);
    try {
      curr_val.set(stoi(curr_val.to_string()));
    }
    catch (const std::invalid_argument& e) {
      error ("internal error" , node.function_id);
    }
    catch (const std::out_of_range& e) {
      error ("int out of range", node.function_id);
    }
  }
  else if (fun_name == "stod") {
    node.arg_list.front()->accept(*this);
    try {
      curr_val.set(stod(curr_val.to_string()));
    }
    catch (const std::invalid_argument& e) {
      error ("internal error" , node.function_id);
    }
    catch (const std::out_of_range& e) {
      error ("int out of range", node.function_id);
    }
  }
  else if (fun_name == "itos") {
    node.arg_list.front()->accept(*this);
    int val;
    curr_val.value(val);
    curr_val.set(to_string(val));
  }
  else if (fun_name == "dtos") {
    node.arg_list.front()->accept(*this);
    double val;
    curr_val.value(val);
    curr_val.set(to_string(val));
  }
  else if (fun_name == "get") {
    node.arg_list.front()->accept(*this);
    int i;
    curr_val.value(i);
    node.arg_list.back()->accept(*this);
    std::string str = "";
    curr_val.value(str);
    std::string c;
    try {
      c = str[i];
      curr_val.set(c);
    }
    catch (const std::invalid_argument& e) {
      error("internal error", node.function_id);
    }
    catch (const std::out_of_range& e) {
      error("int out of range", node.function_id);
    }
  }
  else if (fun_name == "length") {
    node.arg_list.front()->accept(*this);
    std::string s = curr_val.to_string();
    int size = s.length();
    curr_val.set(size);
  }
  else if (fun_name == "read") {
    //node.arg_list.front()->accept(*this);
    std::string str;
    cin >> str;
    curr_val.set(str);
  }

  else {
    //call the function
    // 1. evaluate the args and save
    // 2. save the current environment
    // 3. go to the gobal environment
    // 4. push a new environment
    // 5. add param values ( from 1)
    // 6. eval each statement
    // 7. catch a return exception
    // 8. pop environment
    // 9. return to saved environment

    FunDecl* fun_node = functions[fun_name];
    //...
    //define a map first
    unordered_map<std::string, DataObject> map;
    std::list<FunDecl::FunParam>::iterator it = fun_node->params.begin();
    for (Expr* e : node.arg_list) {
      e->accept(*this);
      //build the map
      map[it->id.lexeme()] = curr_val;
      it++;
    }
    int curr_env = sym_table.get_environment_id();
    sym_table.set_environment_id(global_env_id);
    sym_table.push_environment();
    //what to do with (5)
    //put map values in symbol table
    //add name and set value in symbol table
    for (FunDecl::FunParam f : fun_node->params) {
      sym_table.add_name(f.id.lexeme());
      sym_table.set_val_info(f.id.lexeme(), map[f.id.lexeme()]);
    }
    try {
      for (Stmt* s : fun_node->stmts) {
        s->accept(*this);
      }
    }
    catch (MyPLReturnException* e) {
      
    }
    sym_table.pop_environment();
    sym_table.set_environment_id(curr_env);
  }
}

void Interpreter::visit(IDRValue& node) 
{
  debug("<IDRValue>");
  std::list<Token>::iterator it = node.path.begin();
  sym_table.get_val_info(it->lexeme(), curr_val);
  it++;
  //cout << " in idrvalue";
  for (; it != node.path.end(); ++it) {
    HeapObject obj;
    size_t oid = 20;
    curr_val.value(oid);
    /*
    if (heap.get_obj(oid, obj)) {
      obj.get_val(it->lexeme(), curr_val);
    }
    */
    if (heap.has_obj(oid)) {
      heap.get_obj(oid, obj);
      obj.get_val(it->lexeme(), curr_val);
    }
    else {
      error("no attribute name ", *it);
    }
  }
  //cout << "end of idrvalue" << endl;
}

void Interpreter::visit(NegatedRValue& node) 
{
  debug("<NegatedRValue>");
  if (node.expr != nullptr) {
    node.expr->accept(*this);
  }
  if (curr_val.is_integer()) {
    int c_val;
    curr_val.value(c_val);
    curr_val.set(c_val*-1);
  }
  else if (curr_val.is_double()) {
    double c_val;
    curr_val.value(c_val);
    curr_val.set(c_val*-1.0);
  }
}

void Interpreter::visit(PointerType& node)
{
  //std::string str = node.pointer.lexeme();
  //sym_table.get_val_info(str, curr_val);
  //cout << "here " << curr_val.to_string() << endl;
  unordered_map<std::string, tuple<std::string, DataObject>>::iterator itr;
  itr = intAddress.find(node.pointer.lexeme());
  curr_val = get<1>(itr->second);
}

void Interpreter::visit(PointerValue& node)
{
  std::string str = node.pointer.lexeme();
  str = str.substr(1);
 // cout << str << endl;
  sym_table.get_val_info(str, curr_val);
  currPtr = str;
  //cout << str + " " << curr_val.to_string() << endl;
}

#endif
