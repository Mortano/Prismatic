#pragma once

#include <cmath>

namespace pe
{

    //! 2D vector
    template<typename T>
    struct peVector2
    {

        union
        {
            struct
            {
                T x, y;
            };
            T data[2];
        };

#pragma region Constructors

        peVector2() :
            x(0),
            y(0)
        {
        }

        peVector2(T xx, T yy) :
            x(xx),
            y(yy)
        {
        }

        peVector2(const peVector2<T>& other)
        {
            x = other.x;
            y = other.y;
        }

#pragma endregion

#pragma region Methods

        //! Returns the length of this vector
        T Length() const
        {
            return sqrt((x*x) + (y*y));
        }

        //! Returns the squared length of this vector
        T LengthSqr() const
        {
            return (x*x) + (y*y);
        }

        //! Returns a normalized version of this vector
        peVector2<T> Normalized() const
        {
            T length = Length();
            return peVector2<T>(x / length, y / length);
        }

        //! Normalizes this vector
        peVector2<T>& NormalizeSelf()
        {
            T length = Length();
            x /= length;
            y /= length;
            return *this;
        }

#pragma endregion

#pragma region Operators

        peVector2<T>& operator=(const peVector2<T>& other)
        {
            x = other.x;
            y = other.y;
            return *this;
        }

        peVector2<T>& operator+=(const peVector2<T>& other)
        {
            x += other.x;
            y += other.y;
            return *this;
        }

        peVector2<T>& operator-=(const peVector2<T>& other)
        {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        peVector2<T>& operator*=(T factor)
        {
            x *= factor;
            y *= factor;
            return *this;
        }

        peVector2<T>& operator/=(T factor)
        {
            x /= factor;
            y /= factor;
            return *this;
        }

        peVector2<T> operator+(const peVector2<T>& rhs)
        {
            return peVector2<T>(x + rhs.x, y + rhs.y);
        }

        peVector2<T> operator-(const peVector2<T>& rhs)
        {
            return peVector2<T>(x - rhs.x, y - rhs.y);
        }

        peVector2<T> operator*(T rhs)
        {
            return peVector2<T>(x * rhs, y * rhs);
        }

        peVector2<T> operator/(T rhs)
        {
            return peVector2<T>(x / rhs, y / rhs);
        }

#pragma endregion

    };

    typedef peVector2<float>    peVector2f;
    typedef peVector2<double>   peVector2d;

}