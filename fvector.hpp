#include <cmath>
#include <iostream>


class Vector3f
{
public:
    float content[3];

public:
    Vector3f() : content{0., 0., 0.} {}
    Vector3f(float x, float y, float z) : content{x, y, z} {}
    Vector3f(const Vector3f& v) {
        content[0] = v.content[0];
        content[1] = v.content[1];
        content[2] = v.content[2];
    }
    ~Vector3f() {}

    float x() const { return content[0]; }
    float y() const { return content[1]; }
    float z() const { return content[2]; }

    float length() const {
        return sqrt(pow(content[0], 2) + pow(content[1], 2) + pow(content[2], 2));
    }

    Vector3f operator - () const {
            return Vector3f(- content[0], - content[1], - content[2]);
        }
    float operator [] (int i) const { return content[i]; }
    float& operator [] (int i) { return content[i]; }
    
    Vector3f& operator += (const Vector3f &v) {
        content[0] += v.content[0];
        content[1] += v.content[1];
        content[2] += v.content[2];
        return *this;
    }

    Vector3f& operator -= (const Vector3f &v) {
        content[0] -= v.content[0];
        content[1] -= v.content[1];
        content[2] -= v.content[2];
        return *this;
    }

    Vector3f& operator *= (const float v) {
        content[0] *= v;
        content[1] *= v;
        content[2] *= v;
        return *this;
    }

    Vector3f& operator /= (const float v) {
        return *this *= 1 / v;
    }

    Vector3f normalize() {
        return *this / length(); 
    }

    float dot(const Vector3f v) {
        float r1 = content[0] * v.content[0];
        float r2 = content[1] * v.content[1];
        float r3 = content[2] * v.content[2];
        return r1 + r2 + r3;
    }

    Vector3f cross(const Vector3f v) {
        float r1 = content[1] * v.content[2] - content[2] * v.content[1];
        float r2 = content[2] * v.content[0] - content[0] * v.content[2];
        float r3 = content[0] * v.content[1] - content[1] * v.content[0];
        return Vector3f(r1, r2, r3);
    }

    friend std::ostream& operator << (std::ostream &out, const Vector3f v) {
        return out << "Vector3f(" << v.content[0] << ", " << v.content[1] << ", " << v.content[2] << ")";
    }

    friend Vector3f operator + (const Vector3f v1, const Vector3f v2) {
        return Vector3f(
            v1.content[0] + v2.content[0], 
            v1.content[1] + v2.content[1],
            v1.content[2] + v2.content[2]
        );
    }

    friend Vector3f operator - (const Vector3f v1, const Vector3f v2) {
        return Vector3f(
            v1.content[0] - v2.content[0], 
            v1.content[1] - v2.content[1],
            v1.content[2] - v2.content[2]
        );
    }

    friend Vector3f operator * (const Vector3f v1, const Vector3f v2) {
        return Vector3f(
            v1.content[0] * v2.content[0], 
            v1.content[1] * v2.content[1],
            v1.content[2] * v2.content[2]
        );
    }

    friend Vector3f operator * (const float v1, const Vector3f v2) {
        return Vector3f(
            v1 * v2.content[0], 
            v1 * v2.content[1],
            v1 * v2.content[2]
        );
    }

    friend Vector3f operator * (const Vector3f v1, const float v2) {
        return v2 * v1;
    }

    friend Vector3f operator / (const Vector3f v1, const float v2) {
        return (1 / v2) * v1;
    }
};



class Vector4f
{
public:
    float content[4];

public:
    Vector4f() : content{0., 0., 0., 0.} {}
    Vector4f(float x, float y, float z, float w) : content{x, y, z, w} {}
    Vector4f(const Vector4f& v) {
        content[0] = v.content[0];
        content[1] = v.content[1];
        content[2] = v.content[2];
        content[3] = v.content[3];
    }
    ~Vector4f() {}

    float x() const { return content[0]; }
    float y() const { return content[1]; }
    float z() const { return content[2]; }
    float w() const { return content[3]; }

    float length() const {
        return sqrt(pow(content[0], 2) + pow(content[1], 2) + pow(content[2], 2) + pow(content[3], 2));
    }

    Vector4f operator - () const {
            return Vector4f(- content[0], - content[1], - content[2], - content[3]);
        }
    float operator [] (int i) const { return content[i]; }
    float& operator [] (int i) { return content[i]; }
    
    Vector4f& operator += (const Vector4f &v) {
        content[0] += v.content[0];
        content[1] += v.content[1];
        content[2] += v.content[2];
        content[3] += v.content[3];
        return *this;
    }

    Vector4f& operator -= (const Vector4f &v) {
        content[0] -= v.content[0];
        content[1] -= v.content[1];
        content[2] -= v.content[2];
        content[3] -= v.content[3];
        return *this;
    }

    Vector4f& operator *= (const float v) {
        content[0] *= v;
        content[1] *= v;
        content[2] *= v;
        content[3] *= v;
        return *this;
    }

    Vector4f& operator /= (const float v) {
        return *this *= 1 / v;
    }

    Vector4f normalize(Vector4f v ) {
        return v / v.length(); 
    }

    float dot(const Vector4f v) {
        float r1 = content[0] * v.content[0];
        float r2 = content[1] * v.content[1];
        float r3 = content[2] * v.content[2];
        float r4 = content[3] * v.content[3];
        return r1 + r2 + r3 + r4;
    }

    Vector4f cross(const Vector4f v) {
        float r1 = content[1] * v.content[2] - content[2] * v.content[1];
        float r2 = content[2] * v.content[0] - content[0] * v.content[2];
        float r3 = content[0] * v.content[1] - content[1] * v.content[0];
        return Vector4f(r1, r2, r3, 1.);
    }

    friend std::ostream& operator << (std::ostream &out, const Vector4f v) {
        return out << "Vector3f(" << v.content[0] << ", " << v.content[1] << ", " << v.content[2] << ", " << v.content[3] << ")";
    }

    friend Vector4f operator + (const Vector4f v1, const Vector4f v2) {
        return Vector4f(
            v1.content[0] + v2.content[0], 
            v1.content[1] + v2.content[1],
            v1.content[2] + v2.content[2],
            v1.content[3] + v2.content[3]
        );
    }

    friend Vector4f operator - (const Vector4f v1, const Vector4f v2) {
        return Vector4f(
            v1.content[0] - v2.content[0], 
            v1.content[1] - v2.content[1],
            v1.content[2] - v2.content[2],
            v1.content[3] - v2.content[3]
        );
    }

    friend Vector4f operator * (const Vector4f v1, const Vector4f v2) {
        return Vector4f(
            v1.content[0] * v2.content[0], 
            v1.content[1] * v2.content[1],
            v1.content[2] * v2.content[2],
            v1.content[3] * v2.content[3]
        );
    }

    friend Vector4f operator * (const float v1, const Vector4f v2) {
        return Vector4f(
            v1 * v2.content[0], 
            v1 * v2.content[1],
            v1 * v2.content[2],
            v1 * v2.content[3]
        );
    }

    friend Vector4f operator * (const Vector4f v1, const float v2) {
        return v2 * v1;
    }

    friend Vector4f operator / (const Vector4f v1, const float v2) {
        return (1 / v2) * v1;
    }
};
