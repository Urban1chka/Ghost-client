#include "Math.hpp"
#include <Windows.h>
#include "lib/imgui/imgui.h"

void Vector2::Abs() {
	this->x = abs(this->x);
	this->y = abs(this->y);
}

float Vector2::Length() {
	return sqrt((this->x * this->x) + (this->y * this->y));
}

float Vector2::Distance(Vector2 vInput) {
	return (*this - vInput).Length();
}

float Vector2::Distance2D(Vector2 vInput) {
	return sqrt(powf(this->x - vInput.x, 2) + powf(this->y - vInput.y, 2));
}

Vector2 Vector2::Normalized() {
	return Vector2(this->x / Length(), this->y / Length());
}

bool Vector2::Empty() {
	return this->x == 0.f && this->y == 0.f;
}

void Vector3::Abs() {
	this->x = abs(this->x);
	this->y = abs(this->y);
	this->z = abs(this->z);
}

float Vector3::LengthSqr() {
	return (this->x * this->x) + (this->y * this->y) + (this->z * this->z);
}

float Vector3::Length() {
	return sqrt(LengthSqr());
}

float Vector3::Length2D() {
	return sqrt((this->x * this->x) + (this->y * this->y));
}

float Vector3::Distance(Vector3 vInput) {
	return (*this - vInput).Length();
}

float Vector3::Distance2D(Vector3 vInput) {
	return (*this - vInput).Length2D();
}

float Vector3::Dot(Vector3 vInput) {
	return this->x * vInput.x + this->y * vInput.y + this->z * vInput.z;
}

Vector3 Vector3::Normalized() {
	return Vector3(this->x / Length(), this->y / Length(), this->z / Length());
}

bool Vector3::Empty() {
	return this->x == 0.f && this->y == 0.f && this->z == 0.f;
}

Vector3 Vector3::Angles(Vector3 start, Vector3 end) {
	Vector3 delta = end - start;
	float hypotenuse = sqrtf(delta.x * delta.x + delta.z * delta.z);

	Vector3 angles;

	angles.x = -atan2f(delta.y, hypotenuse) * (180.0f / 3.1415926535f);
	angles.y = atan2f(delta.x, delta.z) * (180.0f / 3.1415926535f);

	angles.z = 0.0f;

	return angles;
}

Vector3& Vector3::ClampAngles() {
	if (this->x < -89.0f) this->x = -89.0f;
	if (this->x > 89.0f)  this->x = 89.0f;

	while (this->y < -180.0f) this->y += 360.0f;
	while (this->y > 180.0f)  this->y -= 360.0f;

	this->z = 0.0f;
	return *this;
}

float Vector3::Magnitude() {
	return sqrt(this->x * this->x + this->y * this->y + this->z * this->z);
}
float Vector3::dot_product(Vector3 input) const {
	return (x * input.x) + (y * input.y) + (z * input.z);
}

bool Vector3::world_to_screen(Vector2& out, const float matrix[4][4]) const {
	float w = matrix[0][3] * x + matrix[1][3] * y + matrix[2][3] * z + matrix[3][3];
	if (w < 0.001f) return false;

	float x_coord = matrix[0][0] * x + matrix[1][0] * y + matrix[2][0] * z + matrix[3][0];
	float y_coord = matrix[0][1] * x + matrix[1][1] * y + matrix[2][1] * z + matrix[3][1];

	float invW = 1.0f / w;
	x_coord *= invW;
	y_coord *= invW;

	static float screen_width = (float)GetSystemMetrics(SM_CXSCREEN);
	static float screen_height = (float)GetSystemMetrics(SM_CYSCREEN);

	out.x = (screen_width * 0.5f) + (x_coord * screen_width * 0.5f);
	out.y = (screen_height * 0.5f) - (y_coord * screen_height * 0.5f);
	return true;
}
