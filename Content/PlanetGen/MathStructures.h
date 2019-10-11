#pragma once

class Vector3
{
public:
	float X, Y, Z;

	//������������
	inline Vector3(void) {}
	inline Vector3(const float x, const float y, const float z);

	//���������� �������������� ����������
	inline Vector3 operator + (const Vector3& vector) const;
	inline Vector3 operator + (const float num) const;
	inline Vector3 operator - (const Vector3& vector) const;
	inline Vector3 operator - (const float num) const;
	inline Vector3 operator * (const float num) const;

	//���������� ���������� ����������
	inline Vector3 operator = (const Vector3& vector);
	inline bool operator == (const Vector3& vector2);

    void normalize();
};

struct VectorM
{
	Vector3 start;
	Vector3 end;
	float value;

	//������������
	inline VectorM(void) {}
	inline VectorM(const Vector3 v1, const Vector3 v2, const float _value);

	//���������� �������������� ����������
	inline VectorM operator + (const VectorM& vector) const;
	inline VectorM operator * (const float num) const;

	//���������� ���������� ����������
	inline VectorM operator = (const VectorM& vector);
	inline bool operator == (const VectorM& vector2);
};

struct Sphere
{
	float radius;
	Vector3 position;
};



