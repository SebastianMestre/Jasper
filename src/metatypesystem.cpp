#include "metatypesystem.hpp"

#include <cassert>
#include <utility>

#define DEBUG 1

#if DEBUG
#include <iostream>
#endif

namespace TypeChecker {

MetaTypeId MetaTypeSystem::find(MetaTypeId i) {
	if (m_metatype_header[i].tag != MetaTypeTag::Var)
		return i;

	if (m_metatype_header[i].equals == i)
		return i;

	return m_metatype_header[i].equals = find(m_metatype_header[i].equals);
}

void MetaTypeSystem::unify(MetaTypeId i, MetaTypeId j) {
	i = find(i);
	j = find(j);

	if (i == j)
		return;

	if (m_metatype_header[j].tag == MetaTypeTag::Var)
		std::swap(i, j);

	if (m_metatype_header[i].tag == MetaTypeTag::Var) {
		m_metatype_header[i].equals = j;
	} else {

#if DEBUG
		{
			char const* empty = "(no data)";

			char const* data_i = m_metatype_header[i].debug_data
			                         ? m_metatype_header[i].debug_data
			                         : empty;

			char const* data_j = m_metatype_header[j].debug_data
			                         ? m_metatype_header[j].debug_data
			                         : empty;

			std::cerr << "Tried to unify different metatypes.\n"
			          << "Debug data:\n"
			          << "i: " << data_i << '\n'
			          << "j: " << data_j << '\n';
		}
#endif

		assert(0 && "unified two different metatypes");
	}
}

MetaTypeId MetaTypeSystem::new_var(char const* debug_data) {
	int result = m_metatype_header.size();
	m_metatype_header.push_back(
	    {MetaTypeTag::Var, result, debug_data ? debug_data : "var"});
	return result;
}

MetaTypeId MetaTypeSystem::new_meta(char const* debug_data) {
	int result = m_metatype_header.size();
	m_metatype_header.push_back(
	    {MetaTypeTag::Var, result, debug_data ? debug_data : "meta"});
	return result;
}

} // namespace TypeChecker
