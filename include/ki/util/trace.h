#pragma once
#include <iostream>

#if defined(NDEBUG)
#define TRACE if (0) std::cout
#else
#define TRACE if (1) std::cout << "[TRACE] "
#endif
