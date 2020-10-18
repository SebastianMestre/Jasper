#pragma once

namespace Interpreter {
namespace GarbageCollector {

// derive from this class if you want your type to be GC-able
struct Slot {
	bool m_occupied {false};
	bool m_marked {false};

	virtual ~Slot() = default;
};

} // namespace GarbageCollector
} // namespace Intepreter
