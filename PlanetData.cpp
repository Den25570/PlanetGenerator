#include "PlanetData.hpp"

void QuadGeometry::setMesh(TriangleMesh * mesh) {
	indices = std::vector<size_t>(3 * mesh->numSides);
	points = std::vector<size_t>(3 * (mesh->numRegions + mesh->numTriangles));
	tem_mois = std::vector<size_t>(2 * (mesh->numRegions + mesh->numTriangles));
}

void QuadGeometry::setMap(TriangleMesh * mesh, Map * map) {
	const float V = 0.95;

	points = map->r_xyz;
	for (auto p : map->t_xyz)
		points.push_back(p);

	// TODO: multiply all the r, t points by the elevation, taking V into account

	int p = 0;
	for (int r = 0; r < mesh->numRegions; r++) {
		tem_mois[p++] = map->r_elevation[r];
		tem_mois[p++] = map->r_moisture[r];
	}
	for (int t = 0; t < mesh->numTriangles; t++) {
		tem_mois[p++] = map->t_elevation[t];
		tem_mois[p++] = map->t_moisture[t];
	}

	int i = 0;
	int count_valley = 0;
	int count_ridge = 0;

	for (int s = 0; s < mesh->numSides; s++) {
		int opposite_s = mesh->s_opposite_s(s),
			r1 = mesh->s_begin_r(s),
			r2 = mesh->s_begin_r(opposite_s),
			t1 = mesh->s_inner_t(s),
			t2 = mesh->s_inner_t(opposite_s);

		// Each quadrilateral is turned into two triangles, so each
		// half-edge gets turned into one. There are two ways to fold
		// a quadrilateral. This is usually a nuisance but in this
		// case it's a feature. See the explanation here
		// https://www.redblobgames.com/x/1725-procedural-elevation/#rendering
		int coast = map->r_elevation[r1] < 0.0 || map->r_elevation[r2] < 0.0;
		if (coast || map->s_flow[s] > 0 || map->s_flow[opposite_s] > 0) {
			// It's a coastal or river edge, forming a valley
			indices[i++] = r1;
			indices[i++] = mesh->numRegions + t2;
			indices[i++] = mesh->numRegions + t1;
			count_valley++;
		}
		else {
			// It's a ridge
			indices[i++] = r1;
			indices[i++] = r2;
			indices[i++] = mesh->numRegions + t1;
			count_ridge++;
		}
	}
}