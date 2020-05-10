#include "eval.hpp"

#include "ast.hpp"
#include "environment.hpp"
#include "value.hpp"


Type::Value* eval(ASTDeclarationList* ast, Type::Environment& e) {
	for(auto& decl : ast->m_declarations){
		assert(decl->type() == ast_type::Declaration);
		eval(static_cast<ASTDeclaration*>(decl.get()), e);
	}
	return nullptr;
};

Type::Value* eval(ASTDeclaration* ast, Type::Environment& e) {
	// TODO: type and mutable check -> return error
	if (!ast->m_value)
		e.m_scope->declare(ast->m_identifier, e.null());
	else
		e.m_scope->declare(ast->m_identifier, eval(ast->m_value.get(), e));

	return e.null();
};

Type::Value* eval(ASTNumberLiteral* ast, Type::Environment& e) {
	for (char a : ast->m_text)
		if (a == '.')
			return e.new_float(std::stof(ast->m_text));

	return e.new_integer(std::stoi(ast->m_text));
};

Type::Value* eval(ASTStringLiteral* ast, Type::Environment& e) {
	return e.new_string(ast->m_text);
};

Type::Value* eval(ASTObjectLiteral* ast, Type::Environment& e) {
	std::cerr << "WARNING: not implemented action (Creating an object)\n";
	return nullptr;
};

Type::Value* eval(ASTDictionaryLiteral* ast, Type::Environment& e) {
	std::cerr << "WARNING: not implemented action (Creating a dictionary)\n";
	return nullptr;
};

Type::Value* eval(ASTIdentifier* ast, Type::Environment& e) {
	return e.m_scope->access(ast->m_text);
};

Type::Value* eval(ASTBlock* ast, Type::Environment& e) {
	return e.null();
};

Type::Value* eval(ASTReturnStatement* ast, Type::Environment& e) {
	// TODO: proper error handling
	auto* returning = eval(ast->m_value.get(), e);
	assert(returning);

	e.save_return_value(returning);
	return e.null();
};

Type::Value* eval(ASTArgumentList* ast, Type::Environment& e) {
	// TODO: return as list?
	assert(0);
	return e.null();
};

Type::Value* eval(ASTCallExpression* ast, Type::Environment& e) {
	std::vector<Type::Value*> args;

	// TODO: proper error handling

	auto* calleeTypeErased = eval(ast->m_callee.get(), e);
	assert(calleeTypeErased);

	auto* callee = dynamic_cast<Type::Function*>(calleeTypeErased);
	assert(callee);

	auto* arglist = dynamic_cast<ASTArgumentList*>(ast->m_args.get());
	assert(callee->m_def->m_args.size() == arglist->m_args.size());

	e.new_scope();

	for(int i = 0; i < int(callee->m_def->m_args.size()); ++i){
		auto* argdeclTypeErased = callee->m_def->m_args[i].get();
		assert(argdeclTypeErased);

		auto* argdecl = dynamic_cast<ASTDeclaration*>(argdeclTypeErased);
		assert(argdecl);

		auto* argvalue = eval(arglist->m_args[i].get(), e);
		e.declare(argdecl->m_identifier, argvalue);
	}

	auto* body = dynamic_cast<ASTBlock*>(callee->m_def->m_body.get());
	assert(body);

	Type::Value* return_value;
	for(auto &stmt : body->m_body){
		eval(stmt.get(), e);
		if ((return_value = e.fetch_return_value()))
			break;
	}

	e.end_scope();

	return return_value;
};

Type::Value* eval(ASTBinaryExpression* ast, Type::Environment& e) {

	auto* lhs = eval(ast->m_lhs.get(), e);
	auto* rhs = eval(ast->m_rhs.get(), e);

	switch (ast->m_op) {
	case token_type::ADD: {
		
		// TODO: proper error handling

		Type::Integer* lhs_i = dynamic_cast<Type::Integer*>(lhs);
		if (lhs_i) {
			Type::Integer* rhs_i = dynamic_cast<Type::Integer*>(rhs);
			assert(rhs_i);
			return e.new_integer(lhs_i->m_value + rhs_i->m_value);
		}

		Type::Float* lhs_f = dynamic_cast<Type::Float*>(lhs);
		if (lhs_f) {	
			Type::Float* rhs_f = dynamic_cast<Type::Float*>(rhs);
			assert(rhs_f);
			return e.new_float(lhs_f->m_value + rhs_f->m_value);
		}

		Type::String* lhs_s = dynamic_cast<Type::String*>(lhs);
		Type::String* rhs_s = dynamic_cast<Type::String*>(rhs);
		assert(lhs_s);
		assert(rhs_s);
		return e.new_string(lhs_s->m_value + rhs_s->m_value);
		break;
	}
	case token_type::SUB: {
		
		// TODO: proper error handling

		Type::Integer* lhs_i = dynamic_cast<Type::Integer*>(lhs);
		if (lhs_i) {
			Type::Integer* rhs_i = dynamic_cast<Type::Integer*>(rhs);
			assert(rhs_i);
			return e.new_integer(lhs_i->m_value - rhs_i->m_value);
		}

		Type::Float* lhs_f = dynamic_cast<Type::Float*>(lhs);
		Type::Float* rhs_f = dynamic_cast<Type::Float*>(rhs);
		assert(lhs_f);
		assert(rhs_f);
		return e.new_float(lhs_f->m_value - rhs_f->m_value);
		break;
	}
	case token_type::MUL: {
		
		// TODO: proper error handling

		Type::Integer* lhs_i = dynamic_cast<Type::Integer*>(lhs);
		if (lhs_i) {
			Type::Integer* rhs_i = dynamic_cast<Type::Integer*>(rhs);
			assert(rhs_i);
			return e.new_integer(lhs_i->m_value * rhs_i->m_value);
		}

		Type::Float* lhs_f = dynamic_cast<Type::Float*>(lhs);
		Type::Float* rhs_f = dynamic_cast<Type::Float*>(rhs);
		assert(lhs_f);
		assert(rhs_f);
		return e.new_float(lhs_f->m_value * rhs_f->m_value);
		break;
	}
	case token_type::DIV: {
		
		// TODO: proper error handling

		Type::Integer* lhs_i = dynamic_cast<Type::Integer*>(lhs);
		if (lhs_i) {
			Type::Integer* rhs_i = dynamic_cast<Type::Integer*>(rhs);
			assert(rhs_i);
			return e.new_integer(lhs_i->m_value / rhs_i->m_value);
		}

		Type::Float* lhs_f = dynamic_cast<Type::Float*>(lhs);
		Type::Float* rhs_f = dynamic_cast<Type::Float*>(rhs);
		assert(lhs_f);
		assert(rhs_f);
		return e.new_float(lhs_f->m_value / rhs_f->m_value);
		break;
	}
	case token_type::AND: {
		
		// TODO: proper error handling
		Type::Boolean* lhs_b = dynamic_cast<Type::Boolean*>(lhs);
		Type::Boolean* rhs_b = dynamic_cast<Type::Boolean*>(rhs);
		assert(lhs_b);
		assert(rhs_b);
		return e.new_boolean(lhs_b->m_value and rhs_b->m_value);
		break;
	}
	case token_type::IOR: {
		
		// TODO: proper error handling
		Type::Boolean* lhs_b = dynamic_cast<Type::Boolean*>(lhs);
		Type::Boolean* rhs_b = dynamic_cast<Type::Boolean*>(rhs);
		assert(lhs_b);
		assert(rhs_b);
		return e.new_boolean(lhs_b->m_value or rhs_b->m_value);
		break;
	}
	case token_type::XOR: {
		
		// TODO: proper error handling
		Type::Boolean* lhs_b = dynamic_cast<Type::Boolean*>(lhs);
		Type::Boolean* rhs_b = dynamic_cast<Type::Boolean*>(rhs);
		assert(lhs_b);
		assert(rhs_b);
		return e.new_boolean(lhs_b->m_value xor rhs_b->m_value);
		break;
	}
	default:
		std::cerr << "WARNING: not implemented action"
		             "(Evaluating binary expression)\n";
	}

	return nullptr;
}

Type::Value* eval(ASTFunctionLiteral* ast, Type::Environment& e) {
	return e.new_function(ast, e.m_scope);
};

Type::Value* eval(AST* ast, Type::Environment& e) {

	switch (ast->type()) {
	case ast_type::NumberLiteral:
		return eval(static_cast<ASTNumberLiteral*>(ast), e);
	case ast_type::StringLiteral:
		return eval(static_cast<ASTStringLiteral*>(ast), e);
	case ast_type::ObjectLiteral:
		return eval(static_cast<ASTObjectLiteral*>(ast), e);
	case ast_type::DictionaryLiteral:
		return eval(static_cast<ASTDictionaryLiteral*>(ast), e);
	case ast_type::FunctionLiteral:
		return eval(static_cast<ASTFunctionLiteral*>(ast), e);
	case ast_type::DeclarationList:
		return eval(static_cast<ASTDeclarationList*>(ast), e);
	case ast_type::Declaration:
		return eval(static_cast<ASTDeclaration*>(ast), e);
	case ast_type::Identifier:
		return eval(static_cast<ASTIdentifier*>(ast), e);
	case ast_type::BinaryExpression:
		return eval(static_cast<ASTBinaryExpression*>(ast), e);
	case ast_type::CallExpression:
		return eval(static_cast<ASTCallExpression*>(ast), e);
	case ast_type::ArgumentList:
		return eval(static_cast<ASTArgumentList*>(ast), e);
	case ast_type::Block:
		return eval(static_cast<ASTBlock*>(ast), e);
	case ast_type::ReturnStatement:
		return eval(static_cast<ASTReturnStatement*>(ast), e);
	}

	return nullptr;
}
