#pragma once

#include <type_traits>
#include "value.hpp"

template <typename ValueType>
struct gc_ptr {
  private:
	ValueType* m_ptr;

  public:
	gc_ptr(gc_ptr const& o)
	    : m_ptr {o.m_ptr} {
		if (m_ptr)
			m_ptr->m_cpp_refcount += 1;
	}

	gc_ptr(gc_ptr&& o)
	    : m_ptr {o.m_ptr} {
		o.m_ptr = nullptr;
	}

	gc_ptr& operator= (gc_ptr&& o) {
		if (m_ptr)
			m_ptr->m_cpp_refcount -= 1;
		m_ptr = o.m_ptr;
		o.m_ptr = nullptr;
		return *this;
	}

	explicit operator bool() const {
		return m_ptr != nullptr;
	}

	template <typename T>
	friend class gc_ptr;

	template <
	    typename T,
	    typename = typename std::enable_if<std::is_convertible<T*, ValueType*>::value>::type>
	gc_ptr(gc_ptr<T>&& o)
	    : m_ptr {o.m_ptr} {
		o.m_ptr = nullptr;
	}

	gc_ptr(ValueType* ptr)
	    : m_ptr {ptr} {
		if (m_ptr)
			m_ptr->m_cpp_refcount += 1;
	}

	~gc_ptr() {
		if (m_ptr)
			m_ptr->m_cpp_refcount -= 1;
	}

	ValueType* get() const {
		return m_ptr;
	}

	ValueType* operator->() const {
		return get();
	}

	Interpreter::Value as_value() {
		return Interpreter::Value {get()};
	}
};
