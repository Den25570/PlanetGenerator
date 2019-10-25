#pragma once

#define _USE_MATH_DEFINES
#include <math.h>

using namespace DirectX;

//Если нужен вектор на плоскости оставлять поле z = 0 

class Vector3
{
public:
	float x, y, z;

	//Конструкторы
	inline Vector3(void) {}
	inline Vector3(const float _x, const float _y, const float _z)
	{
		x = _x; y = _y; z = _z;
	}

	//Перегрузка арифметических операторов
	inline Vector3 operator + (const Vector3& vector) ;
	inline Vector3 operator - (const Vector3& vector) ;
	inline Vector3 operator * (const float num) ;

	//Перегрузка логических операторов
	inline bool operator == (const Vector3& vector2);

	//Функции для работы
	static Vector3 normalize(Vector3 v);
	static float length(Vector3 v);
	static Vector3 calcVector(Vector3 v0, Vector3 v1, float length);
	float getPolarAngle();
	XMFLOAT3 convertToXMFLOAT3();
};

//Создание стереографической проекции
std::vector<Vector3> projectSphereOnPlane(const std::vector<Vector3> originPoints);
std::vector<Vector3> reverseProjectSphereOnPlane(const std::vector<Vector3> originPoints);

//Сравнение чисел с плавающей запятой
bool compareF(const float num_1, const float num_2,const float eps = 0.001);
bool compareD(const double num_1, const double num_2, const double eps = 0.001);

//Детерминант матрицы 3 на 3
float mDeterminant3(std::vector<float> e);

//Векторное произведение векторов на плоскости, возвращает значение z
float vectorMult2(Vector3 v0, Vector3 v1);

float calcAngl(Vector3 v10, Vector3 v11, Vector3 v20, Vector3 v21);

//Пересечение векторов v0-v1 и v2-v3
bool vectorCollision(Vector3 v0, Vector3 v1, Vector3 v2, Vector3 v3);

//Определение знака численного типа
template <typename T> int sgn(T val) {
	return (T(0) < val) - (val < T(0));
}

