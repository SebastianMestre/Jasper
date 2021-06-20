#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../interpreter/exit_status_tag.hpp"
#include "test_status_tag.hpp"

namespace Interpreter {
struct Interpreter;
}

namespace Frontend {
struct SymbolTable;
}

namespace Test {

struct ITestSet {
	virtual TestReport execute() = 0;
	virtual ~ITestSet() = default;
};

using Interpret = ExitStatus (*)(Interpreter::Interpreter&, Frontend::SymbolTable&);

struct InterpreterTestSet : public ITestSet {

	std::string m_source_file;
	std::vector<Interpret> m_testers;
	bool m_dump = false;

	InterpreterTestSet(std::string);
	InterpreterTestSet(std::string, Interpret);
	InterpreterTestSet(std::string, std::vector<Interpret>);

	TestReport execute() override;
};

struct NormalTestSet : public ITestSet {
	using TestFunction = TestReport (*)();

	NormalTestSet();
	NormalTestSet(TestFunction);
	NormalTestSet(std::vector<TestFunction>);

	std::vector<TestFunction> m_testers;

	TestReport execute() override;
};

struct TestSet {
	std::unique_ptr<ITestSet> m_data;

	template <typename T>
	TestSet(T data)
	    : m_data {std::make_unique<T>(std::move(data))} {}

	bool operator==(TestSet const& o) const {
		return m_data == o.m_data;
	}

	TestReport execute() {
		return m_data->execute();
	}
};

} // namespace Test
