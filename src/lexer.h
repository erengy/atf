/*
MIT License

Copyright (c) 2015-2017 Eren Okka

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <functional>
#include <string>
#include <vector>

namespace atf {

// This class is a wrapper around std::function, which allows us to make a
// recursive type definition for state functions.
//
// For a general-purpose class that uses variadic templates, see:
// http://stackoverflow.com/a/23742130
class State {
public:
  using function_t = std::function<State()>;

  State(function_t function) : function_(function) {}
  operator function_t() { return function_; }

private:
  function_t function_;
};

// Each state function returns the next state, which is also a state function.
// The idea comes from Golang's "text/template/parse" package and the "Lexical
// Scanning in Go" talk by Rob Pike: https://youtu.be/HxaD_trXwRE
//
// Note that "using T = std::function<T()>" and its typedef equivalent are not
// allowed, which is why we use the wrapper class.
using state_function_t = State::function_t;

enum class TokenType {
  Text,               // Plain text (default type)
  FieldBegin,         // "%"
  FieldName,          // e.g. "title"
  FieldEnd,           // "%"
  FunctionSymbol,     // "$"
  FunctionName,       // e.g. "if"
  FunctionBegin,      // "("
  FunctionDelimiter,  // ","
  FunctionEnd,        // ")"
  RawBegin,           // "'"
  RawEnd,             // "'"
  ConditionBegin,     // "["
  ConditionEnd,       // "]"
};

struct Token {
  TokenType type;
  std::string value;
};

using token_container_t = std::vector<Token>;
using token_iterator_t = token_container_t::const_iterator;
using token_range_t = std::pair<token_iterator_t, token_iterator_t>;

class Lexer {
public:
  void Tokenize(const std::string& input);
  const token_container_t& tokens() const;

private:
  state_function_t LexText();
  state_function_t LexField();
  state_function_t LexFunctionBegin();
  state_function_t LexFunctionDelimiter();
  state_function_t LexFunctionEnd();
  state_function_t LexRaw();
  state_function_t LexConditionBegin();
  state_function_t LexConditionEnd();

  void AddToken(const TokenType type, const std::string& value);
  void AddToken(const TokenType type);
  void AddReservedToken(const TokenType type);
  bool IsReservedCharacter(const char c) const;

  unsigned short function_level_ = 0;
  std::string input_;
  size_t pos_ = 0;
  size_t start_ = 0;
  token_container_t tokens_;
};

}  // namespace atf
