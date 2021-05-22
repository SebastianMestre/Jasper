#include <cstdint>

static constexpr int state_count = 200;
using State = uint8_t;

struct Range {
	unsigned char start;
	unsigned char end;
};

struct Automaton {
	int offset[state_count] {};
	State transition[state_count][256] {};

	constexpr State go(State s, unsigned char c) const {
		return transition[s][c];
	}
};

struct AutomatonBuilder {
	int state_counter {state_count};
	Automaton automaton {};

	struct StateProxy {
		AutomatonBuilder& builder;
		State state;

		constexpr StateProxy& transition(unsigned char c, State dst) {
			builder.transition(state, c, dst);
			return *this;
		}

		constexpr StateProxy& transition(Range r, State dst) {
			builder.transition(state, r, dst);
			return *this;
		}

		constexpr StateProxy& default_transition(State dst) {
			builder.default_transition(state, dst);
			return *this;
		}

		constexpr StateProxy from_state(State src) {
			return {builder, src};
		}
	};

	constexpr AutomatonBuilder& transition(State src, unsigned char c, State dst) {
		automaton.transition[src][c] = dst;
		return *this;
	}

	constexpr AutomatonBuilder& transition(State src, Range r, State dst) {
		for (auto c = r.start;; ++c)  {
			transition(src, c, dst);
			if (c == r.end) break;
		}
		return *this;
	}

	constexpr AutomatonBuilder& default_transition(State src, State dst) {
		return transition(src, Range {0, 255}, dst);
	}

	constexpr StateProxy from_state(State src) {
		return {*this, src};
	}

	constexpr int new_state() {
		return --state_counter;
	}
};
