#ifndef MACRO_H
#define MACRO_H
#include"util.h"
#include"Log.h"

#if defined __GNUC__ || defined __llvm__

#define LIKELY(x) __builtin_expect(!!(x),1)

#define UNLIKELY(x) __builtin_expect(!!(x),0)
#else
#define LIKELY(x) (x)
#define UNLIKELY(x) (x)
#endif


#endif // !MACRO_H
