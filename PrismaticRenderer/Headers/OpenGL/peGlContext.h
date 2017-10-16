#pragma once
#include "Exceptions\peExceptions.h"

#include "GL\freeglut.h"

namespace pe
{

    //! Stores function pointers for OpenGL functions depending on the
    //! current driver version. All OpenGL calls should happen through this
    //! object!
    class peGlContext
    {
    public:
        static peGlContext&     GetInstance();

        void                    Init();

#pragma region BasicFunctions
        inline void             glViewport(GLint x, GLint y, GLsizei width, GLsizei height) { _glViewportProc(x, y, width, height); }
        inline void             glFlush() { _glFlushProc(); }
        inline void             glClearColor(GLclampf r, GLclampf g, GLclampf b, GLclampf a) { _glClearColorProc(r, g, b, a); }
        inline void             glClear(GLbitfield mask) { _glClearProc(mask); }
#pragma endregion

    private:
                                peGlContext();
                                
#pragma region BasicFunctionsPtrs
        typedef void(*ViewportProc)(GLint x, GLint y, GLsizei width, GLsizei height);
        typedef void(*FlushProc)();
        typedef void(*ClearColorProc)(GLclampf r, GLclampf g, GLclampf b, GLclampf a);
        typedef void(*ClearProc)(GLbitfield mask);

        ViewportProc            _glViewportProc;
        FlushProc               _glFlushProc;
        ClearColorProc          _glClearColorProc;
        ClearProc               _glClearProc;
#pragma endregion
    };

#define GL_CONTEXT (peGlContext::GetInstance())
#undef CALL_GL_PROC

}