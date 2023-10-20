#include "bytecode.hpp"

template <typename T>
Writer<T> make_writer(T x) {
	return {{}, std::move(x)};
}

namespace Bytecode {

ErrorReport visit(BasicBlock&, AST::Expr*);

ErrorReport failure() { return {"Failed to generate bytecode"}; }

Writer<Executable> compile(AST::Expr* expr) {
	Executable result;
	BasicBlock main_block;
	ErrorReport status = visit(main_block, expr);
	if (status.ok()) {
	} else {
		return status;
	}

	result.blocks.push_back(std::move(main_block));
	return make_writer(std::move(result));
}

ErrorReport visit(BasicBlock& b, AST::Expr* expr) {
	switch (expr->type()) {
	}
	return failure();
}

int decode(char const* stream, Interpreter::Interpreter& e) {
	return 1024;
}

void execute(Executable const& exe, Interpreter::Interpreter& e) {
	int cursor = 0;
	while (cursor < exe.blocks[0].bytecode.size()) {
		cursor += decode(&exe.blocks[0].bytecode[cursor], e);
	}
}

} // namespace Bytecode
