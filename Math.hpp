#pragma once
#include <cmath>

namespace Math {
    inline float my_atan2(float y, float x) { return atan2f(y, x); }
    inline float my_cos(float x) { return cosf(x); }
    inline float my_sqrt(float x) { return sqrtf(x); }
    inline float my_sin(float x) { return sinf(x); }
}

struct matrix4x4 {
	union {
		struct {
			float _11, _12, _13, _14;
			float _21, _22, _23, _24;
			float _31, _32, _33, _34;
			float _41, _42, _43, _44;
		};
		float matrix[4][4];
	};
};

class Vector4 {
public:
	float x, y, z, w;
	Vector4() : x(0), y(0), z(0), w(0) {}
	Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
};

class Vector2 {
public:
	float x, y;

	Vector2() {
		x = y = 0.f;
	}

	Vector2(float newX, float newY) {
		x = newX;
		y = newY;
	}

	Vector2(int newX, int newY) { /* lets stop getting warnings :p */
		x = (float)newX;
		y = (float)newY;
	}

	Vector2 operator+(float fInput) {
		return Vector2{ x + fInput, y + fInput };
	}

	Vector2 operator-(float fInput) {
		return Vector2{ x - fInput, y - fInput };
	}

	Vector2 operator/(float fInput) {
		return Vector2{ x / fInput, y / fInput };
	}

	Vector2 operator*(float fInput) {
		return Vector2{ x * fInput, y * fInput };
	}

	Vector2 operator+(Vector2 vInput) {
		return Vector2{ x + vInput.x, y + vInput.y };
	}

	Vector2 operator-(Vector2 vInput) {
		return Vector2{ x - vInput.x, y - vInput.y };
	}

	Vector2 operator*(Vector2 vInput) {
		return Vector2{ x * vInput.x, y * vInput.y };
	}

	Vector2 operator/(Vector2 vInput) {
		return Vector2{ x / vInput.x, y / vInput.y };
	}

	Vector2& operator-=(Vector2 vInput) {
		x -= vInput.x;
		y -= vInput.y;

		return *this;
	}

	Vector2& operator+=(Vector2 vInput) {
		x += vInput.x;
		y += vInput.y;

		return *this;
	}

	Vector2& operator*=(Vector2 vInput) {
		x *= vInput.x;
		y *= vInput.y;

		return *this;
	}

	Vector2& operator/=(Vector2 vInput) {
		x /= vInput.x;
		y /= vInput.y;

		return *this;
	}

	Vector2& operator/=(float fInput) {
		x /= fInput;
		y /= fInput;

		return *this;
	}

	Vector2& operator*=(float fInput) {
		x *= fInput;
		y *= fInput;

		return *this;
	}

	Vector2& operator-=(float fInput) {
		x -= fInput;
		y -= fInput;

		return *this;
	}

	Vector2& operator+=(float fInput) {
		x += fInput;
		y += fInput;

		return *this;
	}

	bool operator==(Vector2& vInput) {
		return x == vInput.x && y == vInput.y;
	}

	bool operator!=(Vector2& vInput) {
		if (x != vInput.x || y != vInput.y)
			return true;

		return false;
	}

	void Abs();
	float Length();
	float Distance(Vector2 vInput);
	float Distance2D(Vector2 vInput);
	Vector2 Normalized();
	bool Empty();
};
class Vector3 {
public:
	float x, y, z;

	Vector3() {
		x = y = z = 0.f;
	}

	Vector3(float newX, float newY, float newZ) {
		x = newX;
		y = newY;
		z = newZ;
	}

	Vector3 operator+(float fInput) const {
		return Vector3{ x + fInput, y + fInput, z + fInput };
	}

	Vector3 operator-(float fInput) const {
		return Vector3{ x - fInput, y - fInput, z - fInput };
	}

	Vector3 operator/(float fInput) const {
		return Vector3{ x / fInput, y / fInput, z / fInput };
	}

	Vector3 operator*(float fInput) const {
		return Vector3{ x * fInput, y * fInput, z * fInput };
	}

	Vector3 operator+(Vector3 vInput) const {
		return Vector3{ x + vInput.x, y + vInput.y, z + vInput.z };
	}

	Vector3 operator-(Vector3 vInput) const {
		return Vector3{ x - vInput.x, y - vInput.y, z - vInput.z };
	}

	Vector3 operator*(Vector3 vInput) const {
		return Vector3{ x * vInput.x, y * vInput.y, z * vInput.z };
	}

	Vector3 operator/(Vector3 vInput) const {
		return Vector3{ x / vInput.x, y / vInput.y, z / vInput.z };
	}

	Vector3& operator-=(Vector3 vInput) {
		x -= vInput.x;
		y -= vInput.y;
		z -= vInput.z;

		return *this;
	}

	Vector3& operator+=(Vector3 vInput) {
		x += vInput.x;
		y += vInput.y;
		z += vInput.z;
		return *this;
	}

	Vector3& operator*=(Vector3 vInput) {
		x *= vInput.x;
		y *= vInput.y;
		z *= vInput.z;
		return *this;
	}

	Vector3& operator/=(Vector3 vInput) {
		x /= vInput.x;
		y /= vInput.y;
		z /= vInput.z;
		return *this;
	}

	Vector3& operator/=(float fInput) {
		x /= fInput;
		y /= fInput;
		z /= fInput;
		return *this;
	}

	Vector3& operator*=(float fInput) {
		x *= fInput;
		y *= fInput;
		z *= fInput;
		return *this;
	}

	Vector3& operator-=(float fInput) {
		x -= fInput;
		y -= fInput;
		z -= fInput;
		return *this;
	}

	Vector3& operator+=(float fInput) {
		x += fInput;
		y += fInput;
		z += fInput;
		return *this;
	}

	bool operator==(Vector3 vInput) const {
		return x == vInput.x && y == vInput.y && z == vInput.z;
	}

	bool operator!=(Vector3 vInput) const {
		if (x != vInput.x || y != vInput.y || z != vInput.z)
			return true;

		return false;
	}

	void Abs();
	float LengthSqr();
	float Length();
	float Length2D();
	float Distance(Vector3 vInput);
	float Distance2D(Vector3 vInput);
	float Dot(Vector3 vInput);
	Vector3 Normalized();
	bool Empty();
	static Vector3 Angles(Vector3 start, Vector3 end);
	Vector3& ClampAngles();
	bool world_to_screen(Vector2& out, const float matrix[4][4]) const;

	float Magnitude();
	float dot_product(Vector3 input) const;

	static Vector3 MoveTowards(Vector3 currentPos, Vector3 targetPos, float stepSize) {
		return currentPos + ((targetPos - currentPos).Normalized() * stepSize);
	}
};

inline bool m_strcmp(char* a, const char* b) {
	if ((uintptr_t)a == 0x00000000ffffffff || (uintptr_t)b == 0x00000000ffffffff)
		return false;
	if ((uintptr_t)a == 0x000000000000007d || (uintptr_t)b == 0x000000000000007d)
		return false;

	if (!a || !b) return !a && !b;

	int ret = 0;
	unsigned char* p1 = (unsigned char*)a;
	unsigned char* p2 = (unsigned char*)b;
	while (!(ret = *p1 - *p2) && *p2)
		++p1, ++p2;

	return ret == 0;
}