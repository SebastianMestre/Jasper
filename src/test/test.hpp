#pragma once

#include <memory>

#include "test_report.hpp"

namespace Test {

struct Test {
	struct Interface {
		virtual TestReport execute() = 0;
		virtual ~Interface() = default;
	};

	template <typename T>
	struct Impl : Interface {
		T x;

		Impl(T x_)
		    : x {std::move(x_)} {}

		TestReport execute() override {
			return x.execute();
		}
	};

	std::unique_ptr<Interface> m_data;

	template <typename T>
	Test(T data)
	    : m_data {std::make_unique<Impl<T>>(std::move(data))} {}

	TestReport execute() {
		return m_data->execute();
	}
};

}