#pragma once
#include "peMathDefs.h"
#include "PEVector3.h"

#include <xmmintrin.h>
#include <string>

namespace pe
{
    
enum class DoNotInitialize
{
    NoInit
};

//! 4x4 matrix in column-major order
struct peMatrix4f
{
#pragma warning(push)
#pragma warning(disable: 4201) //Unnamed union
    union
    {
        struct
        {
            // column-major
           float m00, m10, m20, m30,
              m01, m11, m21, m31,
              m02, m12, m22, m32,
              m03, m13, m23, m33;
        };
        float data[16];
#ifdef USE_SIMD
        struct
        {
            __m128 v0, v1, v2, v3;
        };
        static_assert(sizeof(__m128) == (4 * sizeof(float)), "Can't use SIMD for matrix because float is not 4 bytes!");
#endif
    };
#pragma warning(pop)

#pragma region Constructors
    //! Constructs a new identity matrix
    peMatrix4f()
    {
        for (size_t i = 0; i < 16; i++)
        {
            if (i % 5 == 0)
            {
                data[i] = 1.f;
            }
            else
            {
                data[i] = 0.f;
            }
        }
    }

    //! Constructs a new matrix without setting the values
    peMatrix4f(DoNotInitialize noInit) {}

    //! Copy constructors
    peMatrix4f(const peMatrix4f& other)
    {
        memcpy(data, other.data, 16 * sizeof(float));
    }
#pragma endregion

#pragma region Methods

    //! Transforms the given direction vector
    peVector3f TransformDirection(const peVector3f& vec) const
    {
#ifdef USE_SIMD
        __m128 vx = _mm_load1_ps(&vec.x);
        __m128 vy = _mm_load1_ps(&vec.y);
        __m128 vz = _mm_load1_ps(&vec.z);
        __m128 xy = _mm_add_ps(_mm_mul_ps(vx, v0), _mm_mul_ps(vy, v1));
        peVector3f ret;
        ret.aligned = _mm_add_ps(xy, _mm_mul_ps(vz, v2));;
        return ret;
#else
        peVector3f tmp;
        tmp.x = vec.x * data[0] + vec.y * data[4] + vec.z * data[8];
        tmp.y = vec.x * data[1] + vec.y * data[5] + vec.z * data[9];
        tmp.z = vec.x * data[2] + vec.y * data[6] + vec.z * data[10];
        return tmp;
#endif
    }

    peVector3f TransformPosition(const peVector3f& vec) const
    {
#ifdef USE_SIMD
        __m128 vx = _mm_load1_ps(&vec.x);
        __m128 vy = _mm_load1_ps(&vec.y);
        __m128 vz = _mm_load1_ps(&vec.z);
        __m128 xy = _mm_add_ps(_mm_mul_ps(vx, v0), _mm_mul_ps(vy, v1));
        peVector3f ret;
        ret.aligned = _mm_add_ps(xy, _mm_mul_ps(vz, v2));;
        return ret;
#else
        peVector3f tmp;
        tmp.x = vec.x * data[0] + vec.y * data[4] + vec.z * data[8] + data[12];
        tmp.y = vec.x * data[1] + vec.y * data[5] + vec.z * data[9] + data[13];
        tmp.z = vec.x * data[2] + vec.y * data[6] + vec.z * data[10] + data[14];
        return tmp;
#endif
    }

#pragma endregion

#pragma region Operators

    //! Assignment operator
    peMatrix4f& operator=(const peMatrix4f& other)
    {
        memcpy(data, other.data, 16 * sizeof(float));
        return *this;
    }

    //! Adds the given matrix to this matrix
    peMatrix4f& operator+=(const peMatrix4f& other)
    {
#ifdef USE_SIMD
        v0 = _mm_add_ps(v0, other.v0);
        v1 = _mm_add_ps(v1, other.v1);
        v2 = _mm_add_ps(v2, other.v2);
        v3 = _mm_add_ps(v3, other.v3);
#else
        for(size_t i = 0; i < 16; i++)
        {
            data[i] += other.data[i];
        }
#endif
        return *this;
    }

    peMatrix4f& operator-=(const peMatrix4f& other)
    {
#ifdef USE_SIMD
        v0 = _mm_sub_ps(v0, other.v0);
        v1 = _mm_sub_ps(v1, other.v1);
        v2 = _mm_sub_ps(v2, other.v2);
        v3 = _mm_sub_ps(v3, other.v3);
#else
        for (size_t i = 0; i < 16; i++)
        {
            data[i] -= other.data[i];
        }
#endif
        return *this;
    }

    peMatrix4f& operator*=(const peMatrix4f& other)
    {
#ifdef USE_SIMD
        //TODO SIMD multiplication
        for (size_t i = 0; i < 4; i++)
        {
            data[i * 4] = this->data[0] * other.data[i * 4] + this->data[4] * other.data[i * 4 + 1] + this->data[8] * other.data[i * 4 + 2] + this->data[12] * other.data[i * 4 + 3];
            data[i * 4 + 1] = this->data[1] * other.data[i * 4] + this->data[5] * other.data[i * 4 + 1] + this->data[9] * other.data[i * 4 + 2] + this->data[13] * other.data[i * 4 + 3];
            data[i * 4 + 2] = this->data[2] * other.data[i * 4] + this->data[6] * other.data[i * 4 + 1] + this->data[10] * other.data[i * 4 + 2] + this->data[14] * other.data[i * 4 + 3];
            data[i * 4 + 3] = this->data[3] * other.data[i * 4] + this->data[7] * other.data[i * 4 + 1] + this->data[11] * other.data[i * 4 + 2] + this->data[15] * other.data[i * 4 + 3];
        }
#else
        for (size_t i = 0; i < 4; i++)
        {
            data[i * 4] = this->data[0] * other.data[i * 4] + this->data[4] * other.data[i * 4 + 1] + this->data[8] * other.data[i * 4 + 2] + this->data[12] * other.data[i * 4 + 3];
            data[i * 4 + 1] = this->data[1] * other.data[i * 4] + this->data[5] * other.data[i * 4 + 1] + this->data[9] * other.data[i * 4 + 2] + this->data[13] * other.data[i * 4 + 3];
            data[i * 4 + 2] = this->data[2] * other.data[i * 4] + this->data[6] * other.data[i * 4 + 1] + this->data[10] * other.data[i * 4 + 2] + this->data[14] * other.data[i * 4 + 3];
            data[i * 4 + 3] = this->data[3] * other.data[i * 4] + this->data[7] * other.data[i * 4 + 1] + this->data[11] * other.data[i * 4 + 2] + this->data[15] * other.data[i * 4 + 3];
        }
#endif
        return *this;
    }

#pragma endregion

};

}