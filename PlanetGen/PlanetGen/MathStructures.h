#pragma once

#ifndef MATH_STRUCT
#define MATH_STRUCT

struct Vector3
{
	float X, Y, Z;

	//Конструкторы
	inline Vector3(void) {}
	inline Vector3(const float x, const float y, const float z);

	//Перегрузка арифметических операторов
	inline Vector3 operator + (const Vector3& vector) const;
	inline Vector3 operator + (const float num) const;
	inline Vector3 operator - (const Vector3& vector) const;
	inline Vector3 operator - (const float num) const;
	inline Vector3 operator * (const float num) const;

	//Перегрузка логических операторов
	inline Vector3 operator = (const Vector3& vector);
	inline bool operator == (const Vector3& vector2);
};

#endif

