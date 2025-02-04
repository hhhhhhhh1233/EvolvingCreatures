#pragma once
#include "vec3.h"

class quat {
public:
	float x;
	float y;
	float z;
	float w;

	quat() 
	{
		x = 0;
		y = 0;
		z = 0;
		w = 0;
	}

	quat(float X, float Y, float Z, float W)
	{
		x = X;
		y = Y;
		z = Z;
		w = W;
	}

	quat(const quat& q)
	{
		x = q.x;
		y = q.y;
		z = q.z;
		w = q.w;
	}

	quat& operator=(const quat& rhs)
	{
		this->x = rhs.x;
		this->y = rhs.y;
		this->z = rhs.z;
		this->w = rhs.w;
		return *this;
	}

	quat& operator+(const quat& rhs)
	{
		this->x = this->x + rhs.x;
		this->y = this->y + rhs.y;
		this->z = this->z + rhs.z;
		this->w = this->w + rhs.w;
		return *this;
	}

	quat& operator-(const quat& rhs)
	{
		this->x = this->x - rhs.x;
		this->y = this->y - rhs.y;
		this->z = this->z - rhs.z;
		this->w = this->w - rhs.w;
		return *this;
	}

	quat& operator*(const quat& rhs)
	{
		vec3 h = cross(vec3(this->x, this->y, this->z), vec3(rhs.x, rhs.y, rhs.z));
		this->x = h.x;
		this->y = h.y;
		this->z = h.z;
		this->w = this->w + rhs.w;
		return *this;
	}


};

