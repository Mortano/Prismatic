#pragma once
#include "peMathDefs.h"

#include <xmmintrin.h>

//! 3-component vector. The fourth component is not used and exists
//! only for SIMD alignment.
struct PE_MATH_API peVector3f
{
#pragma warning(push)
#pragma warning(disable: 4201) //Unnamed union
    union
    {
        struct
        {
            float x, y, z, w;
        };
        __m128 aligned;
        float data[4];
    };
#pragma warning(pop)

#pragma region Constructors

    //! Creates a new empty vector and sets all values to zero
    peVector3f() :
        x(0),
        y(0),
        z(0),
        w(0)
    {}

    //! Creates a new vector with the given values
    peVector3f(float xx, float yy, float zz) :
        x(xx),
        y(yy),
        z(zz),
        w(0)
    {}

    //! Copies and existing vector
    peVector3f(const peVector3f& other) :
        w(0)
    {
        for (size_t i = 0; i < 4; i++) data[i] = other.data[i];
    }

    peVector3f(peVector3f&& other) :
        w(0)
    {
        for (size_t i = 0; i < 4; i++)
        {
            data[i] = other.data[i];
            other.data[i] = 0;
        }
    }

#pragma endregion

#pragma region Operators

    //! Assignment operator
    inline peVector3f& operator=(const peVector3f& other)
    {
        for (size_t i = 0; i < 4; i++) data[i] = other.data[i];
        return (*this);
    }

    //! Move assignment operator
    inline peVector3f& operator=(peVector3f&& other)
    {
        for (size_t i = 0; i < 4; i++)
        {
            data[i] = other.data[i];
            other.data[i] = 0;
        }
        return (*this);
    }

#pragma endregion

};