#pragma once

#ifdef PE_MATH_EXPORT
#define	PE_MATH_API __declspec(dllexport)
#else
#define PE_MATH_API __declspec(dllimport)
#endif

#define USE_SIMD