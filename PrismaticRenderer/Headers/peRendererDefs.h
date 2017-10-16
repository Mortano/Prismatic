#pragma once

#ifdef PE_RENDERER_EXPORT
#define	PE_RENDERER_API __declspec(dllexport)
#else
#define PE_RENDERER_API __declspec(dllimport)
#endif

#ifdef _DEBUG
#define GL_CHECK(call) call; do { GLenum glErr = glGetError(); if(glErr != GL_NO_ERROR) { \
    PrismaticEngine.GetLogging()->LogError("%s threw OpenGL error %i\n", #call, glErr); \
    PE_FAIL("OpenGL assert failed!"); } } while (0)
#else
#define GL_CHECK(call) call
#endif