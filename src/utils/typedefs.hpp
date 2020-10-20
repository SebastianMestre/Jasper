#include <memory>

template<typename T>
using Own = std::unique_ptr<T>;
