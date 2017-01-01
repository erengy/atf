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

#include <limits>

#include "lexer.h"

namespace atf {

constexpr auto kEOF = nullptr;

void Lexer::Tokenize(const std::string& input) {
  input_ = input;

  // We begin by expecting a plain-text token, and continue until the EOF.
  for (auto state = LexText(); state != kEOF; ) {
    state = state();
  }
}

const token_container_t& Lexer::tokens() const {
  return tokens_;
}

////////////////////////////////////////////////////////////////////////////////

state_function_t Lexer::LexText() {
  for ( ; pos_ < input_.size(); ++pos_) {
    const char c = input_.at(pos_);

    if (!IsReservedCharacter(c))
      continue;

    AddToken(TokenType::Text);

    // Since plain-text is the default token type, this is the main branching
    // point of our state machine.
    switch (c) {
      case '%':  return LexField();
      case '$':  return LexFunctionBegin();
      case ',':  return LexFunctionDelimiter();
      case ')':  return LexFunctionEnd();
      case '\'': return LexRaw();
      case '[':  return LexConditionBegin();
      case ']':  return LexConditionEnd();
    }
  }

  AddToken(TokenType::Text);

  if (function_level_ != 0) {
    // TODO: Return error
  }

  return kEOF;
}

state_function_t Lexer::LexField() {
  AddReservedToken(TokenType::FieldBegin);

  pos_ = input_.find('%', pos_);

  if (pos_ == input_.npos) {
    // TODO: Return error
  }

  AddToken(TokenType::FieldName);
  AddReservedToken(TokenType::FieldEnd);

  return LexText();
}

state_function_t Lexer::LexFunctionBegin() {
  AddReservedToken(TokenType::FunctionSymbol);

  pos_ = input_.find('(', pos_);

  if (pos_ == input_.npos) {
    // TODO: Return error
  }

  AddToken(TokenType::FunctionName);
  AddReservedToken(TokenType::FunctionBegin);

  if (function_level_ < std::numeric_limits<decltype(function_level_)>::max()) {
    function_level_ += 1;
  } else {
    // TODO: Return error
  }

  return LexText();
}

state_function_t Lexer::LexFunctionDelimiter() {
  AddReservedToken(TokenType::FunctionDelimiter);

  return LexText();
}

state_function_t Lexer::LexFunctionEnd() {
  AddReservedToken(TokenType::FunctionEnd);
  function_level_ -= 1;

  return LexText();
}

state_function_t Lexer::LexRaw() {
  AddReservedToken(TokenType::RawBegin);

  pos_ = input_.find('\'', pos_);

  if (pos_ == input_.npos) {
    // TODO: Return error
  }

  if (pos_ > start_) {
    AddToken(TokenType::Text);
  } else {
    // This is a special case where "''" is evaluated to "'"
    AddToken(TokenType::Text, "'");
  }
  AddReservedToken(TokenType::RawEnd);

  return LexText();
}

state_function_t Lexer::LexConditionBegin() {
  AddReservedToken(TokenType::ConditionBegin);

  return LexText();
}

state_function_t Lexer::LexConditionEnd() {
  AddReservedToken(TokenType::ConditionEnd);

  return LexText();
}

////////////////////////////////////////////////////////////////////////////////

void Lexer::AddToken(const TokenType type, const std::string& value) {
  tokens_.push_back({type, value});
}

void Lexer::AddToken(const TokenType type) {
  if (pos_ > start_) {
    tokens_.push_back({type, input_.substr(start_, pos_ - start_)});
    start_ = pos_;
  }
}

void Lexer::AddReservedToken(const TokenType type) {
  ++pos_;  // All reserved characters have the same length
  AddToken(type);
}

bool Lexer::IsReservedCharacter(const char c) const {
  switch (c) {
    case '%':
    case '$':
    case '\'':
    case '[':
    case ']':
      return true;
    case ',':
    case ')':
      return function_level_ > 0;
    default:
      return false;
  }
}

}  // namespace atf
