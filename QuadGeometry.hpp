#pragma once

class QuadGeometry {
public:
	std::vector<std::size_t> indices; //i
	std::vector<float> points;  //xyz
	std::vector<float> tem_mois; //tm
};

class QuadGeometryV {
public:
	std::vector<std::size_t> indices; //i
	std::vector<vec3> points;  //xyz
	std::vector<vec2> tem_mois; //tm

	QuadGeometryV() {};
	QuadGeometryV(QuadGeometry qg)
	{
		indices = qg.indices;
		for (int i = 0; i < qg.points.size() / 3; i++)
			points.push_back(vec3(qg.points[i * 3 + 0], qg.points[i * 3 + 1], qg.points[i * 3 + 2]));

		for (int i = 0; i < qg.tem_mois.size() / 2; i++)
			tem_mois.push_back(vec2(qg.tem_mois[i * 2 + 0], qg.tem_mois[i * 2 + 1]));
	}
};

