
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
	return  v0 - Vector3::normalize(v0 - v1)*length;
}

//Создание стереографической проекции
std::vector<Vector3> projectSphereOnPlane(const std::vector<Vector3> originPoints)
{
	std::vector<Vector3> result(originPoints.size());
	for (auto it = originPoints.cbegin(); it != originPoints.cend(); it++)
		result.push_back(Vector3(it->x / (1 - it->z), it->y / (1 - it->z), 0));
	return result;
}

std::vector<Vector3> reverseProjectSphereOnPlane(const std::vector<Vector3> originPoints)
{
	std::vector<Vector3> result(originPoints.size());
	for (auto it = originPoints.cbegin(); it != originPoints.cend(); it++)
		result.push_back(Vector3(2 * it->x / (1 + it->x*it->x + it->y*it->y), 2 * it->y / (1 + it->x*it->x + it->y*it->y), (-1 + it->x*it->x + it->y*it->y) / (1 + it->x*it->x + it->y*it->y)));
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
