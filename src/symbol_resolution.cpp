#include "symbol_resolution.hpp"

#include <iostream>
#include <unordered_map>
#include <vector>

#include "./log/log.hpp"
#include "./utils/error_report.hpp"
#include "./utils/interned_string.hpp"
#include "ast.hpp"
#include "frontend_context.hpp"
#include "symbol_table.hpp"
#include "token.hpp"

#include <cassert>

namespace Frontend {

struct TopLevelDeclTracker {
	AST::Declaration* current() {
		return m_current_decl;
	}

	void enter(AST::Declaration* decl) {
		assert(!m_current_decl);
		m_current_decl = decl;
	}

	void exit() {
		assert(m_current_decl);
		m_current_decl = nullptr;
	}

	AST::Declaration* m_current_decl {nullptr};
};

template<typename T>
struct PtrStack {
	T* current() {
		return m_data.empty() ? nullptr : m_data.back();
	}

	void enter(T* seq_expr) {
		m_data.push_back(seq_expr);
	}

	void exit() {
		m_data.pop_back();
	}

	std::vector<T*> m_data;
};

#define CHECK_AND_RETURN(expr)                                                 \
	{                                                                          \
		auto err = (expr);                                                     \
		if (!err.ok()) {                                                       \
			return err;                                                        \
		}                                                                      \
	}

#define CHECK_AND_WRAP(expr, wrap)                                             \
	{                                                                          \
		auto err = (expr);                                                     \
		if (!err.ok()) {                                                       \
			return {(wrap), {std::move(err)}};                                 \
		}                                                                      \
	}

struct SymbolResolutionCommand {
	SymbolResolutionCommand(Context const& file_context, SymbolTable& symbol_table)
		: file_context {file_context}
		, symbol_table {symbol_table} {}

	ErrorReport handle(AST::Expr* ast) {
		return resolve(ast);
	}

	ErrorReport handle_program(AST::Program* ast) {
		return resolve_program(ast);
	}

private:

	Context const& file_context;
	SymbolTable& symbol_table;
	TopLevelDeclTracker top_level;
	PtrStack<AST::FunctionLiteral> functions;
	PtrStack<AST::SequenceExpression> seq_exprs;

	[[nodiscard]] ErrorReport resolve(AST::Identifier* ast) {

		AST::Declaration* declaration = symbol_table.access(ast->text());

		if (!declaration) {
			// TODO: clean up how we build error reports
			auto token = ast->token();
			SourceLocation token_location = file_context.char_offset_to_location(token->m_start_offset);
			return make_located_error(
				"accessed undeclared identifier '" + ast->text().str() + "'",
				token_location);
		}

		ast->m_declaration = declaration;
		ast->m_surrounding_function = functions.current();

		auto top_level_decl = top_level.current();
		if (top_level_decl)
			top_level_decl->m_references.insert(declaration);

		if (declaration->is_global()) {
			ast->m_origin = AST::Identifier::Origin::Global;
		} else if (declaration->m_surrounding_function != ast->m_surrounding_function) {
			ast->m_origin = AST::Identifier::Origin::Capture;
		} else {
			ast->m_origin = AST::Identifier::Origin::Local;
		}

		// dont capture globals
		if (!declaration->is_global()) {
			for (int i = functions.m_data.size(); i--;) {
				auto* func = functions.m_data[i];
				if (func == declaration->m_surrounding_function)
					break;
				func->m_captures.insert({ast->text(), {declaration}});
			}
		}

		return {};
	}

	[[nodiscard]] ErrorReport resolve(AST::CallExpression* ast) {
		CHECK_AND_RETURN(resolve(ast->m_callee));
		for (auto& arg : ast->m_args)
			CHECK_AND_RETURN(resolve(arg));
		return {};
	}

	[[nodiscard]] ErrorReport resolve(AST::FunctionLiteral* ast) {

		ast->m_surrounding_function = functions.current();

		functions.enter(ast);
		symbol_table.new_nested_scope(); // NOTE: this is nested because of lexical scoping

		for (auto& arg : ast->m_args) {
			if (arg.m_type_hint)
				CHECK_AND_RETURN(resolve(arg.m_type_hint));
			symbol_table.declare(&arg);
		}

		// scan body
		CHECK_AND_RETURN(resolve(ast->m_body));

		symbol_table.end_scope();
		functions.exit();

		return {};
	}

	[[nodiscard]] ErrorReport resolve(AST::ArrayLiteral* ast) {
		for (auto& element : ast->m_elements)
			CHECK_AND_RETURN(resolve(element));

		return {};
	}


	[[nodiscard]] ErrorReport resolve_stmt(AST::Declaration* ast) {

		ast->m_surrounding_function = functions.current();
		ast->m_surrounding_seq_expr = seq_exprs.current();
		symbol_table.declare(ast);

		if (ast->m_type_hint)
			CHECK_AND_RETURN(resolve(ast->m_type_hint));

		if (ast->m_value) {
			CHECK_AND_WRAP(
			    resolve(ast->m_value),
			    "While scanning declaration '" + ast->identifier_text().str() + "'");
		}

		return {};
	}

	[[nodiscard]] ErrorReport resolve_stmt(AST::Block* ast) {
		symbol_table.new_nested_scope();
		for (auto& child : ast->m_body)
			CHECK_AND_RETURN(resolve_stmt(child));
		symbol_table.end_scope();
		return {};
	}

	[[nodiscard]] ErrorReport resolve_stmt(AST::IfElseStatement* ast) {
		CHECK_AND_RETURN(resolve(ast->m_condition));
		CHECK_AND_RETURN(resolve_stmt(ast->m_body));

		if (ast->m_else_body)
			CHECK_AND_RETURN(resolve_stmt(ast->m_else_body));
		return {};
	}

	[[nodiscard]] ErrorReport resolve_stmt(AST::WhileStatement* ast) {
		symbol_table.new_nested_scope();

		CHECK_AND_RETURN(resolve(ast->m_condition));
		CHECK_AND_RETURN(resolve_stmt(ast->m_body));

		symbol_table.end_scope();
		return {};
	}

	[[nodiscard]] ErrorReport resolve_stmt(AST::ReturnStatement* ast) {
		ast->m_surrounding_seq_expr = seq_exprs.current();
		return resolve(ast->m_value);
	}

	[[nodiscard]] ErrorReport resolve_stmt(AST::ExpressionStatement* ast) {
		CHECK_AND_RETURN(resolve(ast->m_expression));
		return {};
	}


	[[nodiscard]] ErrorReport resolve(AST::IndexExpression* ast) {
		CHECK_AND_RETURN(resolve(ast->m_callee));
		CHECK_AND_RETURN(resolve(ast->m_index));
		return {};
	}

	[[nodiscard]] ErrorReport resolve(AST::TernaryExpression* ast) {
		CHECK_AND_RETURN(resolve(ast->m_condition));
		CHECK_AND_RETURN(resolve(ast->m_then_expr));
		CHECK_AND_RETURN(resolve(ast->m_else_expr));
		return {};
	}

	[[nodiscard]] ErrorReport resolve(AST::AccessExpression* ast) {
		return resolve(ast->m_target);
	}

	[[nodiscard]] ErrorReport resolve(AST::MatchExpression* ast) {

		CHECK_AND_RETURN(resolve(&ast->m_target));

		if (ast->m_type_hint)
			CHECK_AND_RETURN(resolve(ast->m_type_hint));

		for (auto& kv : ast->m_cases) {
			auto& case_data = kv.second;

			symbol_table.new_nested_scope();

			CHECK_AND_RETURN(resolve_stmt(&case_data.m_declaration));
			CHECK_AND_RETURN(resolve(case_data.m_expression));

			symbol_table.end_scope();
		}

		return {};
	}

	[[nodiscard]] ErrorReport resolve(AST::ConstructorExpression* ast) {
		CHECK_AND_RETURN(resolve(ast->m_constructor));

		for (auto& arg : ast->m_args)
			CHECK_AND_RETURN(resolve(arg));

		return {};
	}

	[[nodiscard]] ErrorReport resolve(AST::SequenceExpression* ast) {
		seq_exprs.enter(ast);
		CHECK_AND_RETURN(resolve_stmt(ast->m_body));
		seq_exprs.exit();
		return {};
	}

	[[nodiscard]] ErrorReport resolve(AST::UnionExpression* ast) {

		for (auto& type : ast->m_types)
			CHECK_AND_RETURN(resolve(type));

		return {};
	}

	[[nodiscard]] ErrorReport resolve(AST::StructExpression* ast) {

		for (auto& type : ast->m_types)
			CHECK_AND_RETURN(resolve(type));

		return {};
	}

	[[nodiscard]] ErrorReport resolve(AST::TypeTerm* ast) {
		CHECK_AND_RETURN(resolve(ast->m_callee));
		for (auto& arg : ast->m_args)
			CHECK_AND_RETURN(resolve(arg));
		return {};
	}

	[[nodiscard]] ErrorReport resolve_stmt(AST::Stmt* ast) {
#define DISPATCH(type)                                                         \
	case ASTStmtTag::type:                                                     \
		return resolve_stmt(static_cast<AST::type*>(ast));

#define DO_NOTHING(type)                                                       \
	case ASTStmtTag::type:                                                     \
		return {};

		switch (ast->tag()) {
			DISPATCH(Block);
			DISPATCH(WhileStatement);
			DISPATCH(IfElseStatement);
			DISPATCH(ReturnStatement);
			DISPATCH(ExpressionStatement);
			DISPATCH(Declaration);
		}

#undef DO_NOTHING
#undef DISPATCH
		Log::fatal() << "(internal) Unhandled case in resolve_stmt '" << ast_stmt_string[int(ast->tag())] << "'";
	}

	[[nodiscard]] ErrorReport resolve(AST::Expr* ast) {
#define DISPATCH(type)                                                         \
	case ASTExprTag::type:                                                     \
		return resolve(static_cast<AST::type*>(ast));

#define DO_NOTHING(type)                                                       \
	case ASTExprTag::type:                                                     \
		return {};

		switch (ast->type()) {
			DO_NOTHING(NumberLiteral);
			DO_NOTHING(IntegerLiteral);
			DO_NOTHING(StringLiteral);
			DO_NOTHING(BooleanLiteral);
			DO_NOTHING(NullLiteral);
			DISPATCH(ArrayLiteral);
			DISPATCH(FunctionLiteral);

			DISPATCH(Identifier);
			DISPATCH(IndexExpression);
			DISPATCH(CallExpression);
			DISPATCH(TernaryExpression);
			DISPATCH(AccessExpression);
			DISPATCH(MatchExpression);
			DISPATCH(ConstructorExpression);
			DISPATCH(SequenceExpression);

			DISPATCH(UnionExpression);
			DISPATCH(StructExpression);
			DISPATCH(TypeTerm);
		}

#undef DO_NOTHING
#undef DISPATCH
		Log::fatal() << "(internal) Unhandled case in resolve '" << ast_expr_string[int(ast->type())] << "'";
	}

	[[nodiscard]] ErrorReport resolve_program(AST::Program* ast) {
		for (auto& decl : ast->m_declarations) {
			symbol_table.declare(&decl);
			decl.m_surrounding_function = functions.current();
		}

		for (auto& decl : ast->m_declarations) {
			top_level.enter(&decl);

			if (decl.m_type_hint)
				CHECK_AND_RETURN(resolve(decl.m_type_hint));

			if (decl.m_value)
				CHECK_AND_WRAP(
					resolve(decl.m_value),
					"While scanning top level declaration '" +
						decl.identifier_text().str() + "'");

			top_level.exit();
		}

		return {};
	}
};


#undef CHECK_AND_RETURN

[[nodiscard]] ErrorReport resolve_symbols(AST::Expr* ast, Context const& file_context, SymbolTable& env) {
	auto command = SymbolResolutionCommand {file_context, env};
	return command.handle(ast);
}

[[nodiscard]] ErrorReport resolve_symbols_program(AST::Program* ast, Context const& file_context, SymbolTable& env) {
	auto command = SymbolResolutionCommand {file_context, env};
	return command.handle_program(ast);
}

} // namespace Frontend
