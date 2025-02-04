#pragma once
#include <cmath>
#include <iostream>
#include "vec4.h"
#include "vec3.h"

class mat4 {
public:
	vec4 m[4];

	mat4()
	{
		m[0] = vec4(1, 0, 0, 0);
		m[1] = vec4(0, 1, 0, 0);
		m[2] = vec4(0, 0, 1, 0);
		m[3] = vec4(0, 0, 0, 1);
	}

	mat4(const vec4& r0, const vec4& r1, const vec4& r2, const vec4& r3)
	{
		m[0] = r0;
		m[1] = r1;
		m[2] = r2;
		m[3] = r3;
	}

	mat4(const mat4& M)
	{
		for (int i = 0; i < 4; i++)
			m[i] = M[i];
	}

	mat4& operator+(const mat4& rhs) const
	{
		mat4 mat;
		for (int i = 0; i < 4; i++)
			mat[i] = rhs[i] + m[i];
		return mat;
	}

	mat4& operator=(const mat4& rhs)
	{
		for (int i = 0; i < 4; i++)
			m[i] = rhs[i];
		return *this;
	}

	mat4 operator*(const float scalar) const
	{
		mat4 mat;
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				mat[i][j] = m[i][j] * scalar;
		return mat;
	}

	mat4 operator*(const mat4& rhs) const
	{
		mat4 mat;
		float tempSum{ 0.0f };
		for (int k = 0; k < 4; k++)
		{
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					tempSum += m[j][k] * rhs[i][j];
				}
				mat[i][k] = tempSum;
				tempSum = 0;
			}

		}
		return mat;
	}
	
	vec4 operator*(const vec4& rhs) const
	{
		return vec4(rhs.x * m[0].x + rhs.y * m[1].x + rhs.z * m[2].x + rhs.w * m[3].x, 
					rhs.x * m[0].y + rhs.y * m[1].y + rhs.z * m[2].y + rhs.w * m[3].y,
					rhs.x * m[0].z + rhs.y * m[1].z + rhs.z * m[2].z + rhs.w * m[3].z, 
					rhs.x * m[0].w + rhs.y * m[1].w + rhs.z * m[2].w + rhs.w * m[3].w);
	}

	bool operator==(const mat4& rhs)
	{
		for (int i = 0; i < 4; i++)
			if (m[i] != rhs[i])
				return false;
		return true;
	}
	
	bool operator!=(const mat4& rhs)
	{
		for (int i = 0; i < 4; i++)
			if (m[i] != rhs[i])
				return true;
		return false;
	}

	vec4& operator[](const int i) 
	{
		return m[i];
	}

	const vec4& operator[](const int i) const
	{
		return m[i];
	}
};

inline float determinant(const mat4& m)
{
	return m[0][0] * (m[1][1] * m[2][2] * m[3][3] + m[2][1] * m[3][2] * m[1][3] + m[3][1] * m[1][2] * m[2][3] - m[1][3] * m[2][2] * m[3][1] - m[2][3] * m[3][2] * m[1][1] - m[3][3] * m[1][2] * m[2][1])
		 - m[1][0] * (m[0][1] * m[2][2] * m[3][3] + m[2][1] * m[3][2] * m[0][3] + m[3][1] * m[0][2] * m[2][3] - m[0][3] * m[2][2] * m[3][1] - m[2][3] * m[3][2] * m[0][1] - m[3][3] * m[0][2] * m[2][1])
		 + m[2][0] * (m[0][1] * m[1][2] * m[3][3] + m[1][1] * m[3][2] * m[0][3] + m[3][1] * m[0][2] * m[1][3] - m[0][3] * m[1][2] * m[3][1] - m[1][3] * m[3][2] * m[0][1] - m[3][3] * m[0][2] * m[1][1])
		 - m[3][0] * (m[0][1] * m[1][2] * m[2][3] + m[1][1] * m[2][2] * m[0][3] + m[2][1] * m[0][2] * m[1][3] - m[0][3] * m[1][2] * m[2][1] - m[1][3] * m[2][2] * m[0][1] - m[2][3] * m[0][2] * m[1][1]);
}
inline mat4 transpose(const mat4& m)
{
	mat4 mat;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			mat[i][j] = m[j][i];
		}
	}
	return mat;
}

inline mat4 inverse(const mat4& m)
{
	if (determinant(m) == 0)
		return mat4();
	vec4 r0 = vec4((m[1][1] * m[2][2] * m[3][3] + m[2][1] * m[3][2] * m[1][3] + m[3][1] * m[1][2] * m[2][3] - m[1][3] * m[2][2] * m[3][1] - m[2][3] * m[3][2] * m[1][1] - m[3][3] * m[1][2] * m[2][1]),
		   		  -(m[1][0] * m[2][2] * m[3][3] + m[2][0] * m[3][2] * m[1][3] + m[3][0] * m[1][2] * m[2][3] - m[1][3] * m[2][2] * m[3][0] - m[2][3] * m[3][2] * m[1][0] - m[3][3] * m[1][2] * m[2][0]),
				   (m[1][0] * m[2][1] * m[3][3] + m[2][0] * m[3][1] * m[1][3] + m[3][0] * m[1][1] * m[2][3] - m[1][3] * m[2][1] * m[3][0] - m[2][3] * m[3][1] * m[1][0] - m[3][3] * m[1][1] * m[2][0]),
		   		  -(m[1][0] * m[2][1] * m[3][2] + m[2][0] * m[3][1] * m[1][2] + m[3][0] * m[1][1] * m[2][2] - m[1][2] * m[2][1] * m[3][0] - m[2][2] * m[3][1] * m[1][0] - m[3][2] * m[1][1] * m[2][0]));
	vec4 r1 = vec4(-(m[0][1] * m[2][2] * m[3][3] + m[2][1] * m[3][2] * m[0][3] + m[3][1] * m[0][2] * m[2][3] - m[0][3] * m[2][2] * m[3][1] - m[2][3] * m[3][2] * m[0][1] - m[3][3] * m[0][2] * m[2][1]),
				    (m[0][0] * m[2][2] * m[3][3] + m[2][0] * m[3][2] * m[0][3] + m[3][0] * m[0][2] * m[2][3] - m[0][3] * m[2][2] * m[3][0] - m[2][3] * m[3][2] * m[0][0] - m[3][3] * m[0][2] * m[2][0]),
				   -(m[0][0] * m[2][1] * m[3][3] + m[2][0] * m[3][1] * m[0][3] + m[3][0] * m[0][1] * m[2][3] - m[0][3] * m[2][1] * m[3][0] - m[2][3] * m[3][1] * m[0][0] - m[3][3] * m[0][1] * m[2][0]),
				    (m[0][0] * m[2][1] * m[3][2] + m[2][0] * m[3][1] * m[0][2] + m[3][0] * m[0][1] * m[2][2] - m[0][2] * m[2][1] * m[3][0] - m[2][2] * m[3][1] * m[0][0] - m[3][2] * m[0][1] * m[2][0]));
	vec4 r2 = vec4((m[0][1] * m[1][2] * m[3][3] + m[1][1] * m[3][2] * m[0][3] + m[3][1] * m[0][2] * m[1][3] - m[0][3] * m[1][2] * m[3][1] - m[1][3] * m[3][2] * m[0][1] - m[3][3] * m[0][2] * m[1][1]),
				  -(m[0][0] * m[1][2] * m[3][3] + m[1][0] * m[3][2] * m[0][3] + m[3][0] * m[0][2] * m[1][3] - m[0][3] * m[1][2] * m[3][0] - m[1][3] * m[3][2] * m[0][0] - m[3][3] * m[0][2] * m[1][0]),
				   (m[0][0] * m[1][1] * m[3][3] + m[1][0] * m[3][1] * m[0][3] + m[3][0] * m[0][1] * m[1][3] - m[0][3] * m[1][1] * m[3][0] - m[1][3] * m[3][1] * m[0][0] - m[3][3] * m[0][1] * m[1][0]),
				  -(m[0][0] * m[1][1] * m[3][2] + m[1][0] * m[3][1] * m[0][2] + m[3][0] * m[0][1] * m[1][2] - m[0][2] * m[1][1] * m[3][0] - m[1][2] * m[3][1] * m[0][0] - m[3][2] * m[0][1] * m[1][0]));
	vec4 r3 = vec4(-(m[0][1] * m[1][2] * m[2][3] + m[1][1] * m[2][2] * m[0][3] + m[2][1] * m[0][2] * m[1][3] - m[0][3] * m[1][2] * m[2][1] - m[1][3] * m[2][2] * m[0][1] - m[2][3] * m[0][2] * m[1][1]),
				    (m[0][0] * m[1][2] * m[2][3] + m[1][0] * m[2][2] * m[0][3] + m[2][0] * m[0][2] * m[1][3] - m[0][3] * m[1][2] * m[2][0] - m[1][3] * m[2][2] * m[0][0] - m[2][3] * m[0][2] * m[1][0]),
				   -(m[0][0] * m[1][1] * m[2][3] + m[1][0] * m[2][1] * m[0][3] + m[2][0] * m[0][1] * m[1][3] - m[0][3] * m[1][1] * m[2][0] - m[1][3] * m[2][1] * m[0][0] - m[2][3] * m[0][1] * m[1][0]),
				    (m[0][0] * m[1][1] * m[2][2] + m[1][0] * m[2][1] * m[0][2] + m[2][0] * m[0][1] * m[1][2] - m[0][2] * m[1][1] * m[2][0] - m[1][2] * m[2][1] * m[0][0] - m[2][2] * m[0][1] * m[1][0]));
	mat4 mat(r0,r1,r2,r3);
	return transpose(mat)* (1 / determinant(m));
}


inline mat4 rotationx(const float rad)
{
	mat4 mat(vec4(1, 0, 0, 0), vec4(0, std::cos(rad), std::sin(rad), 0), vec4(0, -std::sin(rad), std::cos(rad), 0), vec4(0, 0, 0, 1));
	return mat;
}

inline mat4 rotationy(const float rad)
{
	mat4 mat(vec4(std::cos(rad), 0, -std::sin(rad), 0), vec4(0, 1,0, 0), vec4(std::sin(rad), 0, std::cos(rad), 0), vec4(0, 0, 0, 1));
	return mat;
}

inline mat4 rotationz(const float rad)
{
	mat4 mat(vec4(std::cos(rad), std::sin(rad), 0, 0), vec4(-std::sin(rad), std::cos(rad), 0, 0), vec4(0, 0, 1, 0), vec4(0, 0, 0, 1));
	return mat;
}

inline mat4 rotationaxis(const vec3& v, const float rad)
{
	return mat4(vec4((1 - std::cos(rad))*v.x*v.x + std::cos(rad), (1 - std::cos(rad)) * v.x * v.y + std::sin(rad)*v.z, (1 - std::cos(rad))*v.x*v.z - std::sin(rad)*v.y, 0), 
				vec4((1 - std::cos(rad))*v.x*v.y - std::sin(rad) * v.z, (1 - std::cos(rad))*v.y*v.y + std::cos(rad), (1 - std::cos(rad))*v.y*v.z + std::sin(rad)*v.x, 0), 
				vec4((1 - std::cos(rad))*v.x*v.z + std::sin(rad) * v.y, (1 - std::cos(rad))*v.y*v.z - std::sin(rad)*v.x, (1 - std::cos(rad))*v.z*v.z + std::cos(rad), 0),
				vec4(0,0,0,1));
}

inline mat4 translate(vec3 v)
{
	return mat4(vec4(1, 0, 0, 0),
				vec4(0, 1, 0, 0),
				vec4(0, 0, 1, 0),
				vec4(v.x, v.y, v.z, 1));
}

// Generates a perspective matrix based on certain parameters 
inline mat4 perspective(const float fovy, const float aspect, const float near, const float far)
{
	//return mat4(vec4(1, 0, 0, 0),vec4(0, 1, 0, 0),vec4(0, 0, 1, 1),vec4(0, 0, 0, 0));
	float yScale = 1.0 / std::tan(fovy / 2);
	float xScale = yScale / aspect;
	float nearmfar = near - far;
	return mat4(vec4(xScale, 0,0,0), 
				vec4(0, yScale, 0,0), 
				vec4(0,0, (far + near)/nearmfar, -1), 
				vec4(0,0,2*far*near/nearmfar, 0));

}

// Generates a matrix that rotates an object to look at a target
inline mat4 lookat(const vec3& eye, const vec3& at, const vec3& up)
{
	vec3 zAxis = normalize(eye - at);
	vec3 xAxis = normalize(cross(up, zAxis));
	vec3 yAxis = cross(zAxis, xAxis);

	return mat4(vec4(xAxis.x, yAxis.x, zAxis.x, 0),
				vec4(xAxis.y, yAxis.y, zAxis.y, 0),
				vec4(xAxis.z, yAxis.z, zAxis.z, 0),
				vec4(dot(xAxis, -eye), dot(yAxis, -eye), dot(zAxis, -eye), 1));
}

// Generates a matrix that scales an object
inline mat4 scale(float scaleX, float scaleY, float scaleZ)
{
	return mat4(vec4(scaleX, 0, 0, 0),
				vec4(0, scaleY, 0, 0),
				vec4(0, 0, scaleZ, 0),
				vec4(0, 0, 0, 1));
}

// Generates a matrix that scales an object but only takes one argument
inline mat4 scale(float scale)
{
	return mat4(vec4(scale, 0, 0, 0),
		vec4(0, scale, 0, 0),
		vec4(0, 0, scale, 0),
		vec4(0, 0, 0, 1));
}