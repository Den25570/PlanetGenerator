#pragma once

using namespace DirectX;

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
	//inline Vector3& operator = (const Vector3& vector);
	inline bool operator == (const Vector3& vector2);

	//Функции для работы
	static Vector3 normalize(Vector3 v);
	static float length(Vector3 v);
	static Vector3 calcVector(Vector3 v0, Vector3 v1, float length);
	XMFLOAT3 convertToXMFLOAT3();
};

/*struct VectorM
{
	Vector3 start;
	Vector3 end;
	float value;

	//Конструкторы
	inline VectorM(void) {}
	inline VectorM(const Vector3 v1, const Vector3 v2, const float _value);

	//Перегрузка арифметических операторов
	inline VectorM operator + (const VectorM& vector) const;
	inline VectorM operator * (const float num) const;

	//Перегрузка логических операторов
	inline VectorM operator = (const VectorM& vector);
	inline bool operator == (const VectorM& vector2);
};

class Sphere
{
public:
	float radius;
	Vector3 position;
};*/

//Создание стереографической проекции
std::vector<XMFLOAT2> projectSphereOnPlane(const std::vector<Vector3> originPoints);
std::vector<Vector3> reverseProjectSphereOnPlane(const std::vector<XMFLOAT2> originPoints);

//Сравнение чисел с плавающей запятой
bool compareF(const float num_1, const float num_2,const float eps = 0.001);
bool compareD(const double num_1, const double num_2, const double eps = 0.001);



