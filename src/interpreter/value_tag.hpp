#pragma once

#define VALUE_TAGS                                                             \
	X(Null)                                                                    \
                                                                               \
	X(Integer)                                                                 \
	X(Float)                                                                   \
	X(Boolean)                                                                 \
	X(String)                                                                  \
	X(Error)                                                                   \
                                                                               \
	X(Array)                                                                   \
	X(Record)                                                                  \
	X(Variant)                                                                 \
                                                                               \
	X(Function)                                                                \
	X(NativeFunction)                                                          \
                                                                               \
	X(Reference)                                                               \
                                                                               \
	X(VariantConstructor)                                                      \
	X(RecordConstructor)

#define X(name) #name,
constexpr const char* value_string[] = {VALUE_TAGS};
#undef X

#define X(name) name,
enum class ValueTag { VALUE_TAGS };
#undef X

#undef VALUE_TAGS
