
#include <pch.h>
#include "Math.h"


using namespace DirectX;

	//Перегрузка арифметических операторов
    Vector3 Vector3::operator + (const Vector3& vector)
	{
		return Vector3(x + vector.x, y + vector.y, z + vector.z);
	}

    Vector3 Vector3::operator - (const Vector3& vector)
	{
		return Vector3(x - vector.x, y - vector.y, z - vector.z);
	}


 Vector3 Vector3::operator * (const float num)
	{
		return Vector3(x * num, y * num, z * num);
	}	

	//Перегрузка логических операторов
   /* Vector3& Vector3::operator = (const Vector3& vector)
	{
		x = vector.x;
		y = vector.y;
		z = vector.z;

		return *this;
	}*/

	bool Vector3::operator == (const Vector3& vector2)
	{
		return (x == vector2.x) && (y == vector2.y) && (z == vector2.z);
	}

    Vector3 Vector3::normalize(Vector3 v)
	{
		float Length = length(v);
		return Vector3(v.x /= Length, v.y /= Length, v.z /= Length);
	}

    float Vector3::length(Vector3 v)
	{
		return sqrt(v.x*v.x + v.y * v.y + v.z * v.z);
	}

	XMFLOAT3 Vector3::convertToXMFLOAT3()
	{
		return XMFLOAT3(x, y, z);
	}

    Vector3 Vector3::calcVector(Vector3 v0, Vector3 v1, float length)
	{
		return  v0 - Vector3::normalize(v0-v1)*length;
	}

/*struct VectorM
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
};*/

//Создание стереографической проекции
std::vector<XMFLOAT2> projectSphereOnPlane(const std::vector<Vector3> originPoints)
{
	std::vector<XMFLOAT2> result(originPoints.size());
	for (auto it = originPoints.cbegin(); it != originPoints.cend(); it++)
		result.push_back(XMFLOAT2(it->x/(1 - it->z), it->y / (1 - it->z)));
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
bool compareF(const float num_1, const float num_2, const float eps)
{
	return abs(num_1 - num_2) < eps;
}

bool compareD(const double num_1, const double num_2, const double eps)
{
	return abs(num_1 - num_2) < eps;
}
