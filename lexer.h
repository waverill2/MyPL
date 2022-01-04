//----------------------------------------------------------------------
// NAME: Weston Averill
// FILE: lexer.h
// DATE: 2/4/2021
// DESC: Implement Lexer for MyPL
//----------------------------------------------------------------------

#ifndef LEXER_H
#define LEXER_H

#include <istream>
#include <string>
#include "token.h"
#include "mypl_exception.h"

class Lexer
{
public:

  // construct a new lexer from the input stream
  Lexer(std::istream& input_stream);

  // return the next available token in the input stream (including
  // EOS if at the end of the stream)
  Token next_token();
  
private:

  // input stream, current line, and current column
  std::istream& input_stream;
  int line;
  int column;

  // return a single character from the input stream and advance
  char read();

  // return a single character from the input stream without advancing
  char peek();

  // create and throw a mypl_exception (exits the lexer)
  void error(const std::string& msg, int line, int column) const;
};

Lexer::Lexer(std::istream& input_stream)
  : input_stream(input_stream), line(1), column(1)
{
}

char Lexer::read()
{
  return input_stream.get();
}

char Lexer::peek()
{
  return input_stream.peek();
}


void Lexer::error(const std::string& msg, int line, int column) const
{
  throw MyPLException(LEXER, msg, line, column);
}

Token Lexer::next_token()
{
  std::string lexeme = "";
  char ch = read();
  //check for white space and new lines
  while (isspace(ch))
  {
    if (ch == '\n')
    {
      ch = read();
      line++;
      column = 1;
    }
    else 
    {
      ch = read();
      column++;
    }
  }
  
  //check for comments
  if (ch == '#')
  {
    bool extra = true;
    while (extra)
    {
      while (ch != '\n')
      {
        ch = read();
        column++;
      }
      line++;
      column = 1;
      ch = read();
      
      while (isspace(ch))
      {
        if (ch == '\n')
        {
          ch = read();
          line++;
          column = 1;
        }
        else 
        {
          ch = read();
          column++;
        }
      }
      
      if (ch == '#')
      {
        extra = true;
      }
      else 
      {
        extra = false;
      }
    }
  }
  
  int tempCol = column;
  column++;
 
  //if were at the end of the file, quit
  if (ch == EOF) {
    return Token(EOS, "", line, tempCol);
  }
  
  //check first character if its a letter
  //if it is, check for reserved words or IDs
  else if (isalpha(ch))
  {
    //tempCol = column;
    while (isalpha(ch) || ch == '_' || isdigit(ch))
    {
      lexeme += ch;
      if (isalpha(peek()) || peek() == '_' || isdigit(peek()))
      {
        ch = read();
        column++;
      }
      else
        break;
    }
    if (lexeme == "type")
      return Token(TYPE, lexeme, line, tempCol);
    if (lexeme == "bool")
      return Token(BOOL_TYPE, lexeme, line, tempCol);
    if (lexeme == "int")
      return Token(INT_TYPE, lexeme, line, tempCol);
    if (lexeme == "double")
      return Token(DOUBLE_TYPE, lexeme, line, tempCol);
    if (lexeme == "char")
      return Token(CHAR_TYPE, lexeme, line, tempCol);
    if (lexeme == "string")
      return Token(STRING_TYPE, lexeme, line, tempCol);
    if (lexeme == "and")
      return Token(AND, lexeme, line, tempCol);
    if (lexeme == "or")
      return Token(OR, lexeme, line, tempCol);
    if (lexeme == "not")
      return Token(NOT, lexeme, line, tempCol);
    if (lexeme == "while")
      return Token(WHILE, lexeme, line, tempCol);
    if (lexeme == "for")
      return Token(FOR, lexeme, line, tempCol);
    if (lexeme == "do")
      return Token(DO, lexeme, line, tempCol);
    if (lexeme == "if")
      return Token(IF, lexeme, line, tempCol);
    if (lexeme == "then")
      return Token(THEN, lexeme, line, tempCol);
    if (lexeme == "else")
      return Token(ELSE, lexeme, line, tempCol);
    if (lexeme == "elseif")
      return Token(ELSEIF, lexeme, line, tempCol);
    if (lexeme == "end")
      return Token(END, lexeme, line, tempCol);
    if (lexeme == "fun")
      return Token(FUN, lexeme, line, tempCol);
    if (lexeme == "var")
      return Token(VAR, lexeme, line, tempCol);
    if (lexeme == "to")
      return Token(TO, lexeme, line, tempCol);
    if (lexeme == "return")
      return Token(RETURN, lexeme, line, tempCol);
    if (lexeme == "new")
      return Token(NEW, lexeme, line, tempCol);
    if (lexeme == "nil")
      return Token(NIL, lexeme, line, tempCol);
    if (lexeme == "neg")
      return Token(NEG, lexeme, line, tempCol);
    if (lexeme == "true" || lexeme == "false")
      return Token(BOOL_VAL, lexeme, line, tempCol);
    else
      return Token(ID, lexeme, line, tempCol);
  }

  else if (ch == '~' && (isalpha(peek()) || peek() == '_' || isdigit(peek()))) 
  {
    lexeme+= ch;
    ch = read();
    while (isalpha(ch) || ch == '_' || isdigit(ch)) 
    {
      lexeme += ch;
      if (isalpha(peek()) || peek() == '_' || isdigit(peek())) 
      {
        ch = read();
        column++;
      }
      else 
        break;
    }
    return Token(POINTER_TYPE, lexeme, line, tempCol);
  }

  else if (ch == '&' && (isalpha(peek()) || peek() == '_' || isdigit(peek())))
  {
    lexeme+= ch;
    ch = read();
    while (isalpha(ch) || ch == '_' || isdigit(ch)) 
    {
      lexeme += ch;
      if (isalpha(peek()) || peek() == '_' || isdigit(peek())) 
      {
        ch = read();
        column++;
      }
      else 
        break;
    }
    return Token(POINTER_VAL, lexeme, line, tempCol);
  }
  
  //now check numbers
  //doubles and ints
  else if (isdigit(ch))
  {
    bool oneDot = false;
    bool moreDots = false;
    //get numbers and dots
    tempCol = column;
    while(isdigit(ch) || ch == '.')
    {
      if (ch == '.')
      {
        oneDot = true;
      }
      lexeme += ch;
      //makes sure close comment isn't in output
      if (isdigit(peek()) || peek() == '.') {
        ch = read();
        column++;
      }
      else
        break;
    }
    //if there is a dot, it is a double
    if (oneDot == true)
    {
      return Token(DOUBLE_VAL, lexeme, line, tempCol);
    }
    //it is an integer
    else
    {
      return Token(INT_VAL, lexeme, line, tempCol);
    }
  }
  
  //if double quote, means it is a string
  else if (ch == '"')
  {
    ch = read();
    column++;
    //keep checking string as long it is not second double quote
    while (ch != '"')
    {
      lexeme += ch;
      ch = read();
      column++;
      //if you reach a new line without close, should throw error
      if (ch == '\n')
        error("Error", line, column);
    }
    return Token(STRING_VAL, lexeme, line, tempCol);
  }
  
  //check for char quote
  else if (ch == '\'')
  {
    lexeme = read();
    //check to make sure it is only one character
    if (peek() != '\'')
    {
      error("Error", line, tempCol);
    }
    else
      ch = read();
    return Token(CHAR_VAL, lexeme, line, tempCol);
  }
  
  //not complex and simple symbols
  else
  {
    //check for double or single =
    if (ch == '=')
    {
      if (peek() == '=')
      {
        ch = read();
        column++;
        return Token(EQUAL, "==", line, tempCol);
      }
      else
        return Token(ASSIGN, "=", line, tempCol);
    }  
    //check for greater or greater than
    else if (ch == '>')
    {
      if (peek() == '=')
      {
        ch = read();
        column++;
        return Token(GREATER_EQUAL, ">=", line, tempCol);
      }
      else 
        return Token(GREATER, ">", line, tempCol);
    }
    //check for less or less than
    else if (ch == '<')
    {
      if (peek() == '=')
      {
        ch = read();
        column++;
        return Token(LESS_EQUAL, "<=", line, tempCol);
      }
      else
        return Token(LESS, "<", line, column);
    }
    //check for not equal
    else if (ch == '!')
    {
      if (peek() == '=')
      {
        ch = read();
        column++;
        return Token(NOT_EQUAL, "!=", line, tempCol);
      }
    }
    //the rest are simple symbols
    else if (ch == '+')
    {
      return Token(PLUS, "+", line, tempCol);
    }
    else if (ch == '-')
    {
      return Token(MINUS, "-", line, tempCol);
    }
    else if (ch == '*')
    {
      return Token(MULTIPLY, "*", line, tempCol);
    }
    else if (ch == '/')
    {
      return Token(DIVIDE, "/", line, tempCol);
    }
    else if (ch == '%')
    {
      return Token(MODULO, "%", line, tempCol);
    }
    else if (ch == '(')
    {
      return Token(LPAREN, "(", line, tempCol);
    }
    else if (ch == ')')
    {
      return Token(RPAREN, ")", line, tempCol);
    }
    else if (ch == '.')
    {
      return Token(DOT, ".", line, tempCol);
    }
    else if (ch == ',')
    {
      return Token(COMMA, ",", line, tempCol);
    }
    else if (ch == ':')
    {
      return Token(COLON, ":", line, tempCol);
    }
    //if ch is not any of these, throw an error
    else
      //lexeme = ch;
      //column++;
      error("Error", line, tempCol);
  }
 return Token(EOS, "", line, tempCol);
}


#endif














