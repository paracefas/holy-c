#pragma once

#include "ast.hpp"

#include <functional>

using ParserFunc = std::function<Result<Token>(std::string)>;

ParserFunc match (token_t expected);
ParserFunc keyword (std::string word);
Result<int> parseInt (std::string in);
Result<double> parseDouble (std::string in);
Result<std::string> parseStr (std::string in);
Result<char> parsePlus (std::string in);
Result<char> parseMinus (std::string in);
Result<char> parseMult (std::string in);
Result<char> parseDiv (std::string in);
Result<char> parseOp (std::string in);
Result<Expr> parseBinaryOp (std::string in);
Result<Expr> parseExpr (std::string in);
Result<std::string> parseID (std::string in);
Result<std::monostate> parseCurlyOpn (std::string in);
Result<std::monostate> parseCurlyCls (std::string in);
Result<std::monostate> parseParOpn (std::string in);
Result<std::monostate> parseParCls (std::string in);
Result<Stmt> parseReturn (std::string in);
Result<std::monostate> parseComma (std::string in);
Result<FunctionArg> parseSingleArg (std::string in);
Result<std::vector<FunctionArg>> parseArgs (std::string in);
Result<char> parseAssignation (std::string in);
Result<char> parseColon (std::string in);
Result<token_t> parseType (std::string in);
Result<Stmt> parseLet (std::string in);
Result<Stmt> parseStmt (std::string in);
Result<std::vector<Stmt>> parseBlock (std::string in);
Result<Stmt> parseFunc1 (std::string in);
Result<Stmt> parseFunc2 (std::string in);
Result<Stmt> parseFunc (std::string in);
Result<std::vector<Stmt>> parse (std::string in);