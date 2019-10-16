#include "pch.h"
#include "../Content/Delanuay.h"

void Triangulation::generateTriangulation_2D(Triangulation *triangulation, std::vector<Vector3> _points)
{
	triangulation->InitializePoints(_points);
	triangulation->SortPointsByX(0, triangulation->points.size()-1);
}

void Triangulation::InitializePoints(std::vector<Vector3> _points)
{
	points.reserve(_points.size());
	for (auto it = _points.cbegin(); it != _points.cend(); it++)
		points.push_back(*it);
}

void Triangulation::SortPointsByX(int start, int end)
{
	if (start < end)
	{
		int pivot = points[end].position.x;
		int P_index = start;

		for (int i = start; i < end; i++)
		{
			if (points[i].position.x <= pivot)
			{
				auto temp = points[i];
				points[i] = points[P_index];
				points[P_index] = temp;
				P_index++;
			}
		}
		auto temp = points[end];
		points[end] = points[P_index];
		points[P_index] = temp;

		SortPointsByX(start, P_index - 1);
		SortPointsByX(P_index + 1, end);
	}
}


