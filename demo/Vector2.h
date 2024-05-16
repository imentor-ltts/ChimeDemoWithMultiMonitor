#pragma once
#include <corecrt_math.h>
class Vector2 {
public:
    float x, y;

    // Constructors
    Vector2() : x(0), y(0) {}
    Vector2(float x, float y) : x(x), y(y) {}

    // Basic arithmetic operations
    Vector2 operator+(const Vector2& other) const {
        return Vector2(x + other.x, y + other.y);
    }

    Vector2 operator-(const Vector2& other) const {
        return Vector2(x - other.x, y - other.y);
    }

    Vector2 operator*(float scalar) const {
        return Vector2(x * scalar, y * scalar);
    }

    Vector2 operator/(float scalar) const {
        if (scalar != 0)
            return Vector2(x / scalar, y / scalar);
        else
            throw ("Division by zero");
    }

    // Dot product
    float dot(const Vector2& other) const {
        return x * other.x + y * other.y;
    }

    // Magnitude (length) of the vector
    float magnitude() const {
        return sqrt(x * x + y * y);
    }

    // Normalize the vector
    Vector2 normalized() const {
        float mag = magnitude();
        if (mag != 0)
            return *this / mag;
        else
            return *this;
    }
};
