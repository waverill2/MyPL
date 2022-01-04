//----------------------------------------------------------------------
// NAME: Weston Averill
// FILE: printer.h
// DATE: 2/19/21
// DESC: Printer class to pretty print
//----------------------------------------------------------------------

#ifndef PRINTER_H
#define PRINTER_H

#include <iostream>
#include "ast.h"
//using namespace std;
class Printer : public Visitor
{
public:
  // constructor
  Printer(std::ostream& output_stream) : out(output_stream) {}

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

private:
  std::ostream& out;
  int indent = 0;

  void inc_indent() {indent += 3;}
  void dec_indent() {indent -= 3;}
  std::string get_indent() {return std::string(indent, ' ');}

};

// TODO: Implement the visitor functions 
  void Printer::visit(Program& node)
  {
    for (Decl* decl : node.decls) {
      decl->accept(*this);
      std::cout << std::endl;
    }
  }
  
  //Besically creating function headers,
  //getting the parameters, and filling the 
  //functions up with statements
  void Printer::visit(FunDecl& node)
  {
    std::cout << "fun "; 
    if (node.return_type.type() == INT_TYPE || node.return_type.type() == DOUBLE_TYPE ||
        node.return_type.type() == BOOL_TYPE || node.return_type.type() == CHAR_TYPE ||
        node.return_type.type() == STRING_TYPE || node.return_type.type() == ID){
      std::cout << node.return_type.lexeme() << " ";
    }
    else
      std::cout << "nil ";
    std::cout << node.id.lexeme();
    std::cout << "(";
    int parameterCount = node.params.size();
    for (FunDecl::FunParam v : node.params) {
      if (parameterCount > 1) {
        std::cout << v.id.lexeme() << ": " << v.type.lexeme() << ", ";
      }
      else
        std::cout << v.id.lexeme() << ": " << v.type.lexeme();
      parameterCount--;
    }
    std::cout << ")" << std::endl;

    for (Stmt* s : node.stmts) {
      inc_indent();
      std::cout << get_indent();
      s->accept(*this);
      std::cout << std::endl;
      dec_indent();
    }
    std::cout << "end" << std::endl;
  }

  void Printer::visit(TypeDecl& node)
  {
    //output structs pretty much
    std::cout << "type ";
    std::cout << node.id.lexeme() << std::endl;
    for(VarDeclStmt* v : node.vdecls) {
      inc_indent();
      std::cout << get_indent();
      v->accept(*this);
      std::cout << std::endl;
      dec_indent();
    }
    std::cout << "end" << std::endl;
  }

  // statements
  void Printer::visit(VarDeclStmt& node)
  {
    //need to output the variable declaration
    std::cout << "var " << node.id.lexeme();
    if (node.type != nullptr) {
      std::cout << ": " << node.type->lexeme();
    }  
    std::cout << " = ";
    node.expr->accept(*this);
  }

  void Printer::visit(AssignStmt& node)
  {
    //outut an assignment statement
    //need to check for dots
    int idCount = node.lvalue_list.size();
    for (Token t : node.lvalue_list) {
      if (idCount > 1)
        std::cout <<  t.lexeme() << "."; 
      else
        std::cout << t.lexeme();
      idCount--;
    }
    std::cout << " = ";
    node.expr->accept(*this);
  }

  void Printer::visit(ReturnStmt& node)
  {
    //out the return statement
    std::cout << "return ";
    node.expr->accept(*this);
  }

  void Printer::visit(IfStmt& node)
  {
    //output if statements, elseifs, and elses
    //need to be careful about indentations
    std::cout << "if ";
    node.if_part->expr->accept(*this);
    std::cout << " then " << std::endl;
    for (Stmt* s : node.if_part->stmts) {
      inc_indent();
      std::cout << get_indent();
      s->accept(*this);
      std::cout << std::endl;
      dec_indent();
    }
    //elseifs
    if (!node.else_ifs.empty()) {
      for (BasicIf* b : node.else_ifs) {
        std::cout << get_indent();
        std::cout << "elseif ";
        b->expr->accept(*this);
        std::cout << " then " << std::endl;
        for (Stmt* s : b->stmts) {
          inc_indent();
          std::cout << get_indent();
          s->accept(*this);
          std::cout << std::endl;
          dec_indent();
        }
      }
    }
    //esles
    if (!node.body_stmts.empty()){
      std::cout << get_indent();
      std::cout << "else" << std::endl;
      for (Stmt* s : node.body_stmts) {
        inc_indent();
        std::cout << get_indent();
        s->accept(*this);
        std::cout << std::endl;
        dec_indent();
      }
    }
    std::cout << get_indent();
    std::cout << "end";
  }

  void Printer::visit(WhileStmt& node)
  {
    //output the whiole statement
    //also need to check indentations
    std::cout << "while ";
    node.expr->accept(*this);
    std::cout << " do" << std::endl;
    for (Stmt* s : node.stmts) {
      inc_indent();
      std::cout << get_indent();
      s->accept(*this);
      std::cout << std::endl;
      dec_indent();
    }
    std::cout << get_indent();
    std::cout << "end";
  }

  void Printer::visit(ForStmt& node)
  {
    //need to output for statement
    //also check for indentations
    std::cout << "for " << node.var_id.lexeme() << " = ";
    node.start->accept(*this);
    std::cout << " to ";
    node.end->accept(*this);
    std::cout << " do" << std::endl;
    for (Stmt* s : node.stmts) {
      inc_indent();
      std::cout << get_indent();
      s->accept(*this);
      std::cout << std::endl;
      dec_indent();
    }
    std::cout << get_indent();
    std::cout << "end";
  }

  // expressions
  void Printer::visit(Expr& node)
  {
    //output the expressions now
    //this part is a little tricky
    if (node.negated) {
      std::cout << "not ";
    }
    //we have a complex expr
    if (node.op != nullptr) {
      std::cout << "(";
      node.first->accept(*this);
      if (node.op != nullptr) {
        std::cout << " " << node.op->lexeme() << " ";
        node.rest->accept(*this);
      }
      std::cout << ")";
    }
    //simple value
    else {
       node.first->accept(*this);
    }
  }

  void Printer::visit(SimpleTerm& node)
  {
    //call the function to print rvalue
    if(node.rvalue != nullptr) {
      node.rvalue->accept(*this);
    }
  }

  void Printer::visit(ComplexTerm& node)
  {
    //call funciton to print the complexterm
    if (node.expr != nullptr) {
      node.expr->accept(*this);
    }
  }

  // rvalues
  void Printer::visit(SimpleRValue& node)
  {
    //output the simple r values
    //special case for strings
    if (node.value.type() == STRING_VAL) {
      std::cout << "\"" << node.value.lexeme() << "\"";
    }
    else if (node.value.type() == CHAR_VAL) {
      std::cout << "\'" << node.value.lexeme() << "\'";
    }
    else
      std::cout << node.value.lexeme(); 
  }

  void Printer::visit(NewRValue& node)
  {
    //output the new rvalue
    std::cout << "new " << node.type_id.lexeme();// << " ";
  }

  void Printer::visit(CallExpr& node)
  {
    //also another tricky expr function
    //create function signature and parameters
    //need to check the commas
    std::cout << node.function_id.lexeme();
    std::cout << "(";
    int callCount = node.arg_list.size();
    for (Expr* e : node.arg_list) {
      if (callCount > 1) {
        e->accept(*this);
        std::cout << ", ";
      }
      else {
        e->accept(*this);
      }
      callCount--;
    }
    std::cout << ")"; 
  }

  void Printer::visit(IDRValue& node)
  {
    //output the id r value now
    //keep track of how many there are
    int idCount = node.path.size();
    for (Token t : node.path) {
      if (idCount > 1)
        std::cout << t.lexeme() << ".";
      else 
        std::cout << t.lexeme();
      idCount--;
    }
  }

  void Printer::visit(NegatedRValue& node)
  {
    //out put the negated r value now
    std::cout << "neg ";
    if (node.expr != nullptr) {
      node.expr->accept(*this);
    }
  }



#endif
