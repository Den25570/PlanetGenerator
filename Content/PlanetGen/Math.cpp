#include "Math.h"
#include <pch.h>

using namespace DirectX;

class Vector3
{
public:
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

	void normalize()
	{
		float length = pow(X, 2) + pow(Y, 2) + pow(Z, 2);
		X /= length;
		Y /= length;
		Z /= length;
	}

	XMFLOAT3 convertToXMFLOAT3()
	{
		return XMFLOAT3(X, Y, Z);
	}
};

struct VectorM
{
	Vector3 start;
	Vector3 end;
	float value;

	inline VectorM(const Vector3 v1, const Vector3 v2, const float _value)
	{
		start = v1;
		end = v2;
		value = value;
	}

	//Перегрузка арифметических операторов
	inline VectorM operator + (const VectorM& vector) const
	{
		return VectorM(start, end + vector.end, value + vector.value);
	}

	inline VectorM operator * (const float num) const
	{
		return VectorM(start, end * num, value * num);
	}

	//Перегрузка логических операторов
	inline VectorM operator = (const VectorM& vector)
	{
		start = vector.start;
		end = vector.end;
		value = vector.value;
	}

	inline bool operator == (const VectorM& vector2)
	{
		return (start == vector2.start) && (end == vector2.end) && (value == vector2.value);
	}
};

//Создание стереографической проекции
std::vector<XMFLOAT2> projectSphereOnPlane(const std::vector<Vector3> originPoints)
{
	std::vector<XMFLOAT2> result(originPoints.size());
	for (auto it = originPoints.cbegin(); it != originPoints.cend(); it++)
		result.push_back(XMFLOAT2(it->X/(1 - it->Z), it->Y / (1 - it->Z)));
	return result;
}

std::vector<Vector3> reverseProjectSphereOnPlane(const std::vector<XMFLOAT2> originPoints)
{
	std::vector<Vector3> result(originPoints.size());
	for (auto it = originPoints.cbegin(); it != originPoints.cend(); it++)
		result.push_back(Vector3(2*it->x/(1 + it->x*it->x + it->y*it->y), 2 * it->y / (1 + it->x*it->x + it->y*it->y),(-1 + it->x*it->x + it->y*it->y) / (1 + it->x*it->x + it->y*it->y)));
	return result;
}

//Сравнение чисел с плавающей запятой
bool compareF(const float num_1, const float num_2, const float eps = 0.0001)
{
	return abs(num_1 - num_2) < eps;
}

bool compareD(const double num_1, const double num_2, const double eps = 0.0001)
{
	return abs(num_1 - num_2) < eps;
}
