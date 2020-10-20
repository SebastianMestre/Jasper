#pragma once

#include <vector>

/**
 * Chunked array. Preserves reference stability.
 */
template <typename T>
struct ChunkedArray {
	static constexpr int bucket_size = 16;
	std::vector<std::vector<T>> m_buckets;

	void push_back(T t) {
		if (m_buckets.empty() || m_buckets.back().size() == bucket_size) {
			m_buckets.push_back(std::vector<T> {});
			m_buckets.back().reserve(bucket_size);
		}
		m_buckets.back().push_back(std::move(t));
	}

	T& back() {
		return m_buckets.back().back();
	}

	T& at(int i) {
		return m_buckets[i / bucket_size][i % bucket_size];
	}

	int size() {
		return m_buckets.empty() ? 0
		                         : (int(m_buckets.size()) - 1) * bucket_size +
		                               int(m_buckets.back().size());
	}
};
