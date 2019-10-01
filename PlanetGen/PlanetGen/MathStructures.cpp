#include "MathStructures.h"

struct Vector3
{
	float X, Y, Z;

	//Конструкторы
	inline Vector3(void) {}
	inline Vector3(const float x, const float y, const float z)
	{
		X = x; Y = y; Z = z;
	}

	//Перегрузка арифметических операторов
	inline Vector3 operator + (const Vector3& vector) const
	{
		return Vector3(X + vector.X, Y + vector.Y, Z + vector.Z);
	}

	inline Vector3 operator + (const float num) const
	{
		return Vector3(X + num, Y + num, Z + num);
	}

	inline Vector3 operator - (const Vector3& vector) const
	{
		return Vector3(X - vector.X, Y - vector.Y, Z - vector.Z);
	}

	inline Vector3 operator - (const float num) const
	{
		return Vector3(X - num, Y - num, Z - num);
	}

	inline Vector3 operator * (const float num) const
	{
		return Vector3(X * num, Y * num, Z * num);
	}

	//Перегрузка логических операторов
	inline Vector3 operator = (const Vector3& vector)
	{
		X = vector.X;
		Y = vector.Y;
		Z = vector.Z;

		return *this;
	}

	inline bool operator == (const Vector3& vector2)
	{
		return (X == vector2.X) && (Y == vector2.Y) && (Z == vector2.Z);
	}
};