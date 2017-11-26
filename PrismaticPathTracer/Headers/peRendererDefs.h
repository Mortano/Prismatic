#pragma once

#ifdef PE_RENDERER_EXPORT
#define PE_RENDERER_API __declspec(dllexport)
#else
#define PE_RENDERER_API __declspec(dllimport)
#endif
