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

float vectorMult2(Vector3 v0, Vector3 v1)
{
	return v0.x*v1.y - v0.y*v1.x;
}

float calcAngl(Vector3 v10, Vector3 v11, Vector3 v20, Vector3 v21)
{
	Vector3 v1, v2;
	v1 = v11 - v10;
	v2 = v21 - v20;

	return (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z) / (sqrtf(v1.x*v1.x + v1.y*v1.y + v1.z*v1.z)*sqrtf(v2.x*v2.x + v2.y*v2.y + v2.z*v2.z));
}

float Vector3::getPolarAngle()
{
	float angle;
	if (x > 0 && y >= 0)
		angle = atan(y / x);
	else if (x > 0 && y < 0)
		angle = atan(y / x) + 2 * M_PI;
	else if (x < 0)
		angle = atan(y / x) + M_PI;
	else if (x = 0 && y > 0)
		angle = M_PI / 2;
	else if (x = 0 && y < 0)
		angle = 3 * M_PI / 2;
	else
		angle = 0;
	return angle;
}

bool vectorCollision(Vector3 v11, Vector3 v12, Vector3 v21, Vector3 v22)
{
	Vector3 cut1 = (v12 - v11);
	Vector3 cut2 = (v22 - v21);

	float Z1 = vectorMult2(cut1, (v21 - v11));
	float Z2 = vectorMult2(cut1, (v22 - v11));

	if (sgn(Z1) == sgn(Z2) || (Z1 == 0) || (Z2 == 0))
		return false;

	Z1 = vectorMult2(cut2, (v11 - v21));
	Z2 = vectorMult2(cut2, (v12 - v21));

	if (sgn(Z1) == sgn(Z2) || (Z1 == 0) || (Z2 == 0))
		return false;
	return true;
}