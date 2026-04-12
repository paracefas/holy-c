#pragma once

#include "./ast.hpp"
#include "./combinators.hpp"


bool is_keyword (std::string s);
Result<Token> lexInt (std::string in);
Result<Token> lexDouble (std::string in);
Result<Token> lexID (std::string in);
Result<Token> lexSymbol (std::string in);
Result<Token> lexString (std::string in);
Result<Token> lexer (std::string in);