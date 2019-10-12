#pragma once

class Vector3
{
public:
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

    void normalize();
	DirectX::XMFLOAT3 convertToXMFLOAT3();
};

struct VectorM
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

struct Sphere
{
	float radius;
	Vector3 position;
};

bool compareF(const float num_1, const float num_2,const float eps = 0.0001);
bool compareD(const double num_1, const double num_2, const double eps = 0.0001);



