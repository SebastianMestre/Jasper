#include "parse.hpp"

#include "ast.hpp"
#include "lexer.hpp"
#include "parser.hpp"

Writer<AST::AST*> parse_program(std::string const& source, TokenArray& ta, AST::Allocator& allocator) {
	std::vector<char> v;
	for (char c : source)
		v.push_back(c);

	Lexer l = {std::move(v), ta};

	Parser p;
	p.m_lexer = &l;
	p.m_ast_allocator = &allocator;

	return p.parse_top_level();
}

Writer<AST::AST*> parse_expression(std::string const& source, TokenArray& ta, AST::Allocator& allocator) {
	std::vector<char> v;
	for (char c : source)
		v.push_back(c);

	Lexer l = {std::move(v), ta};

	Parser p;
	p.m_lexer = &l;
	p.m_ast_allocator = &allocator;

	return p.parse_expression();
}
