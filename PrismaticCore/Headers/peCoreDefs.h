#pragma once

#ifdef PE_CORE_EXPORT
#define	PE_CORE_API __declspec(dllexport)
#else
#define PE_CORE_API __declspec(dllimport)
#endif