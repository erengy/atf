/*
MIT License

Copyright (c) 2015-2016 Eren Okka

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

#include "lexer.h"

namespace atf {

void Lexer::Tokenize(const std::string& input) {
  input_ = input;

  // We begin by expecting a plain-text token, and continue until the EOF.
  for (state_function_t state = LexText(); state != nullptr; ) {
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

    AddToken(kTokenText);

    // Since plain-text is the default token type, this is the main branching
    // point of our state machine.
    switch (c) {
      case '%':
        return std::bind(&Lexer::LexField, this);
      case '$':
        return std::bind(&Lexer::LexFunctionBegin, this);
      case ',':
        return std::bind(&Lexer::LexFunctionDelimiter, this);
      case ')':
        return std::bind(&Lexer::LexFunctionEnd, this);
      case '\'':
        return std::bind(&Lexer::LexRaw, this);
      case '[':
        return std::bind(&Lexer::LexConditionBegin, this);
      case ']':
        return std::bind(&Lexer::LexConditionEnd, this);
    }
  }

  AddToken(kTokenText);

  if (function_level_ != 0) {
    // TODO: Return error
  }

  return nullptr;  // EOF
}

state_function_t Lexer::LexField() {
  AddReservedToken(kTokenFieldBegin);

  pos_ = input_.find('%', pos_);

  if (pos_ == input_.npos) {
    // TODO: Return error
  }

  AddToken(kTokenFieldName);
  AddReservedToken(kTokenFieldEnd);

  return std::bind(&Lexer::LexText, this);
}

state_function_t Lexer::LexFunctionBegin() {
  AddReservedToken(kTokenFunctionSymbol);

  pos_ = input_.find('(', pos_);

  if (pos_ == input_.npos) {
    // TODO: Return error
  }

  AddToken(kTokenFunctionName);
  AddReservedToken(kTokenFunctionBegin);
  function_level_ += 1;

  return std::bind(&Lexer::LexText, this);
}

state_function_t Lexer::LexFunctionDelimiter() {
  AddReservedToken(kTokenFunctionDelimiter);

  return std::bind(&Lexer::LexText, this);
}

state_function_t Lexer::LexFunctionEnd() {
  AddReservedToken(kTokenFunctionEnd);
  function_level_ -= 1;

  return std::bind(&Lexer::LexText, this);
}

state_function_t Lexer::LexRaw() {
  AddReservedToken(kTokenRawBegin);

  pos_ = input_.find('\'', pos_);

  if (pos_ == input_.npos) {
    // TODO: Return error
  }

  if (pos_ > start_) {
    AddToken(kTokenText);
  } else {
    // This is a special case where "''" is evaluated to "'"
    AddToken(kTokenText, "'");
  }
  AddReservedToken(kTokenRawEnd);

  return std::bind(&Lexer::LexText, this);
}

state_function_t Lexer::LexConditionBegin() {
  AddReservedToken(kTokenConditionBegin);

  return std::bind(&Lexer::LexText, this);
}

state_function_t Lexer::LexConditionEnd() {
  AddReservedToken(kTokenConditionEnd);

  return std::bind(&Lexer::LexText, this);
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
