#pragma once

using namespace DirectX;

//���� ����� ������ �� ��������� ��������� ���� z = 0 

class Vector3
{
public:
	float x, y, z;

	//������������
	inline Vector3(void) {}
	inline Vector3(const float _x, const float _y, const float _z)
	{
		x = _x; y = _y; z = _z;
	}

	//���������� �������������� ����������
	inline Vector3 operator + (const Vector3& vector) ;
	inline Vector3 operator - (const Vector3& vector) ;
	inline Vector3 operator * (const float num) ;

	//���������� ���������� ����������
	inline bool operator == (const Vector3& vector2);

	//������� ��� ������
	static Vector3 normalize(Vector3 v);
	static float length(Vector3 v);
	static Vector3 calcVector(Vector3 v0, Vector3 v1, float length);
	XMFLOAT3 convertToXMFLOAT3();
};

//�������� ����������������� ��������
std::vector<Vector3> projectSphereOnPlane(const std::vector<Vector3> originPoints);
std::vector<Vector3> reverseProjectSphereOnPlane(const std::vector<Vector3> originPoints);

//��������� ����� � ��������� �������
bool compareF(const float num_1, const float num_2,const float eps = 0.001);
bool compareD(const double num_1, const double num_2, const double eps = 0.001);



