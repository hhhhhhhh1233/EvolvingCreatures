#pragma once
#include <cassert>

class vec4 {
public:
	float x;
	float y;
	float z;
	float w;

	vec4() 
	{
		x = 0;
		y = 0;
		z = 0;
		w = 0;
	}

	vec4(float X, float Y, float Z, float W)
	{
		x = X;
		y = Y;
		z = Z;
		w = W;
	}

	vec4(const vec4& v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		w = v.w;
	}

	vec4& operator=(const vec4& rhs)
	{
		this->x = rhs.x;
		this->y = rhs.y;
		this->z = rhs.z;
		this->w = rhs.w;
		return *this;
	}

	vec4 operator-() const
	{
		return vec4(-x, -y, -z, -w);
	}

	vec4 operator+(const vec4& rhs) const
	{
		return vec4(this->x + rhs.x, this->y + rhs.y, this->z + rhs.z, this->w + rhs.w);
	}

	vec4& operator+=(const vec4& rhs)
	{
		this->x += rhs.x;
		this->y += rhs.y;
		this->z += rhs.z;
		this->w += rhs.w;
		return *this;
	}

	vec4 operator-(const vec4& rhs) const
	{
		return vec4(this->x - rhs.x, this->y - rhs.y, this->z - rhs.z, this->w - rhs.w);
	}

	vec4& operator-=(const vec4& rhs)
	{
		this->x -= rhs.x;
		this->y -= rhs.y;
		this->z -= rhs.z;
		this->w -= rhs.w;
		return *this;
	}

	vec4& operator*=(const float rhs)
	{
		this->x *= rhs;
		this->y *= rhs;
		this->z *= rhs;
		this->w *= rhs;
		return *this;
	}

	vec4 operator*(const float rhs) const
	{
		return vec4(x * rhs, y * rhs, z * rhs, w * rhs);
	}

	bool operator==(const vec4& rhs) const
	{
		return (this->x == rhs.x && this->y == rhs.y && this->z == rhs.z && this->w == rhs.w);
	}

	bool operator!=(const vec4& rhs)
	{
		return !(this->x == rhs.x && this->y == rhs.y && this->z == rhs.z && this->w == rhs.w);
	}

	float& operator[](const int i)
	{
		assert(i >= 0 && i <= 3);
		return *(&x + i);
	}

	const float& operator[](const int i) const
	{
		assert(i >= 0 && i <= 3);
		return *(&x + i);
	}
};

inline float dot(const vec4& a, const vec4& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

inline float length(const vec4& v)
{
	return sqrt(v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w);
}

inline vec4 normalize(const vec4& v)
{
	return v * (1 / length(v));
}