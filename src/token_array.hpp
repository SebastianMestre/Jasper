#pragma once

/**
 * Bucket-like token list
 * that avoids reallocations,
 * to preserve references of tokens
 * given to other entities.
 */
struct TokenArray {
	static constexpr int bucket_size = 16;
	std::vector<std::vector<Token>> m_buckets;

	void push_back(Token t) {
		if (m_buckets.empty() || m_buckets.back().size() == bucket_size) {
			m_buckets.push_back(std::vector<Token>{});
			m_buckets.back().reserve(bucket_size);
		}
		m_buckets.back().push_back(std::move(t));
	}

	Token& back() { return m_buckets.back().back(); }

	Token& at(int i) { return m_buckets[i / bucket_size][i % bucket_size]; }

	int size() {
		return m_buckets.empty() ? 0
		                         : (int(m_buckets.size()) - 1) * bucket_size
		        + int(m_buckets.back().size());
	}
};
