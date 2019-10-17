
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

float mDeterminant3(std::vector<float> e)
{
	return (e[0] * e[4] * e[8] + e[2] * e[3] * e[7] + e[1] * e[5] * e[6]) -
		(e[2] * e[4] * e[6] + e[0] * e[7] * e[5] + e[1] * e[3] * e[8]);
}

inline float vectorMult2(Vector3 v0, Vector3 v1)
{
	return v0.x*v1.y - v0.y*v1.x;
}

inline bool vectorCollision(Vector3 v11, Vector3 v12, Vector3 v21, Vector3 v22)
{
	Vector3 cut1 = (v12 - v11);
	Vector3 cut2 = (v22 - v21);

	float Z1 = vectorMult2(cut1, (v21 - v11));
	float Z2 = vectorMult2(cut1, (v22 - v11));
	
	if (sgn(Z1) == sgn(Z2) || (Z1 == 0) || (Z2 == 0)) 
	     return false;
	
	float Z1 = vectorMult2(cut2, (v11 - v21));
	float Z2 = vectorMult2(cut2, (v12 - v21));
	
	if (sgn(Z1) == sgn(Z2) || (Z1 == 0) || (Z2 == 0))
		return false;
	return true;
}