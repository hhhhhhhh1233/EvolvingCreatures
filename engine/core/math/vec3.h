#pragma once

#include <cmath>
#include <cassert>

class vec3 {
public:
	float x;
	float y;
	float z;

	vec3() {
		x = 0;
		y = 0;
		z = 0;
	}

	vec3(float X, float Y, float Z)
	{
		x = X;
		y = Y;
		z = Z;
	}

	vec3(const vec3& v) 
	{
		x = v.x;
		y = v.y;
		z = v.z;
	}

	vec3& operator=(const vec3& rhs)
	{
		this->x = rhs.x;
		this->y = rhs.y;
		this->z = rhs.z;
		return *this;
	}

	vec3 operator-() const
	{
		return vec3(-x, -y, -z);
	}

	vec3 operator+(const vec3& rhs) const
	{
		return vec3(this->x + rhs.x, this->y + rhs.y, this->z + rhs.z);
	}

	vec3& operator+=(const vec3& rhs)
	{
		this->x += rhs.x;
		this->y += rhs.y;
		this->z += rhs.z;
		return *this;
	}

	vec3 operator-(const vec3& rhs) const
	{
		return vec3(this->x - rhs.x, this->y - rhs.y, this->z - rhs.z);
	}

	vec3& operator-=(const vec3& rhs)
	{
		this->x -= rhs.x;
		this->y -= rhs.y;
		this->z -= rhs.z;
		return *this;
	}

	vec3& operator*=(const float rhs)
	{
		this->x *= rhs;
		this->y *= rhs;
		this->z *= rhs;
		return *this;
	}

	vec3 operator*(const float rhs) const
	{
		return vec3(x * rhs, y * rhs, z * rhs);
	}

	bool operator==(const vec3& rhs) const
	{
		return (this->x == rhs.x && this->y == rhs.y && this->z == rhs.z);
	}

	bool operator!=(const vec3& rhs) const
	{
		return !(this->x == rhs.x && this->y == rhs.y && this->z == rhs.z);
	}

	float& operator[](const int i)
	{
		assert(i >= 0 && i <= 2);
		return *(&x + i);
	}

	const float& operator[](const int i) const
	{
		assert(i >= 0 && i <= 2);
		return *(&x + i);
	}
};

inline float dot(const vec3& a, const vec3& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline float length(const vec3& v)
{
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

inline vec3 cross(const vec3& a, const vec3& b)
{
	return vec3(a[1]*b[2] - a[2]*b[1], a[2]*b[0] - a[0]*b[2], a[0]*b[1] - a[1]*b[0]);
}

inline vec3 normalize(const vec3& v)
{
	return v * (1/length(v));
}