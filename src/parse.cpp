#include "parse.hpp"

#include "ast.hpp"
#include "lexer.hpp"
#include "parser.hpp"

Writer<std::pair<AST::AST*, AST::Allocator>> parse_program(std::string const& source, TokenArray& ta) {
	std::vector<char> v;
	for (char c : source)
		v.push_back(c);

	Lexer l = {std::move(v), ta};

	std::pair<AST::AST*, AST::Allocator> graph {nullptr, {}};

	Parser p;
	p.m_lexer = &l;
	p.m_ast_allocator = &graph.second;

	auto result = p.parse_top_level();
	return {result.m_error, graph};
}

Writer<std::pair<AST::AST*, AST::Allocator>> parse_expression(std::string const& source, TokenArray& ta) {
	std::vector<char> v;
	for (char c : source)
		v.push_back(c);

	Lexer l = {std::move(v), ta};

	std::pair<AST::AST*, AST::Allocator> graph {nullptr, {}};

	Parser p;
	p.m_lexer = &l;
	p.m_ast_allocator = &graph.second;

	auto result = p.parse_expression();
	return {result.m_error, graph};
}
