#include "pch.h"
#include "../Content/Delanuay.h"
#include <stack>

void Triangulation::generateTriangulation2(Triangulation *triangulation, std::vector<Vector3> _points)
{
	std::vector<DEdge*> minConvexHull;
	std::stack<DEdge*> obsEdges;

	triangulation->InitializePoints(_points);
	triangulation->SortPointsByX(&(triangulation->points), 0, triangulation->points.size() - 1);
	
	//Создание первого треугольника и рёбер в МВО
	DEdge* e1 = new DEdge(&(triangulation->points[0]), &(triangulation->points[1]));
	DEdge* e2 = new DEdge(&(triangulation->points[1]), &(triangulation->points[2]));
	DEdge* e3 = new DEdge(&(triangulation->points[2]), &(triangulation->points[0]));

	minConvexHull.push_back(e1);
	minConvexHull.push_back(e2);
	minConvexHull.push_back(e3);

	DTriangle t = DTriangle(e1, e2, e3);
	t.calcCircum();
	triangulation->triangles.push_back(t);


	//Добавление новых точек
	int last_add_index = 2;
	for (int i = 3; i < triangulation->points.size(); i++)
	{
		int start, end;
		bool visible = true;
		//Поиск видимых рёбер справа
		for (int j = last_add_index; j < minConvexHull.size(), visible; j++)
		{
			for (auto it = minConvexHull.begin(); it != minConvexHull.end(); it++)
			{
				if (*it == minConvexHull[j])
					continue;
				visible &= vectorCollision((*it)->nodes[0]->position, (*it)->nodes[1]->position, triangulation->points[i].position, minConvexHull[j]->nodes[0]->position);
				visible &= vectorCollision((*it)->nodes[0]->position, (*it)->nodes[1]->position, triangulation->points[i].position, minConvexHull[j]->nodes[1]->position);	
				if (!visible)
					break;
			}
			if (visible)
				obsEdges.push(minConvexHull[j]);
		}
		//Поиск видимых рёбер слева
		visible = true;
		for (int j = last_add_index-1; j >= 0, visible; j++)
		{
			for (auto it = minConvexHull.begin(); it != minConvexHull.end(); it++)
			{
				if (*it == minConvexHull[j])
					continue;
				visible &= vectorCollision((*it)->nodes[0]->position, (*it)->nodes[1]->position, triangulation->points[i].position, minConvexHull[j]->nodes[0]->position);
				visible &= vectorCollision((*it)->nodes[0]->position, (*it)->nodes[1]->position, triangulation->points[i].position, minConvexHull[j]->nodes[1]->position);
				if (!visible)
					break;
			}
			if (visible)
				obsEdges.push(minConvexHull[j]);
		}
	
	}
}

void Triangulation::InitializePoints(std::vector<Vector3> _points)
{
	points.reserve(_points.size());
	for (auto it = _points.cbegin(); it != _points.cend(); it++)
		points.push_back(DNode(*it));
}

void Triangulation::SortPointsByX(std::vector<DNode>* nodes, int start, int end)
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

		SortPointsByX(nodes, start, P_index - 1);
		SortPointsByX(nodes, P_index + 1, end);
	}
}

void DTriangle::calcCircum()
{
	
	float x1 = (*edges[0]->nodes[0]).position.x; float y1 = (*edges[0]->nodes[0]).position.y;
	float x2 = (*edges[1]->nodes[0]).position.x; float y2 = (*edges[1]->nodes[0]).position.y;
	float x3 = (*edges[2]->nodes[0]).position.x; float y3 = (*edges[2]->nodes[0]).position.y;

	float a = mDeterminant3( {x1,y1,1,x2,y2,1,x3,y3,1});
	float b = mDeterminant3({ x1*x1 + y1 * y1,y1,1,x2*x2 + y2 * y2,y2,1 ,x3*x3 + y3 * y3,y3,1 });
	float c = mDeterminant3({ x1*x1 + y1 * y1,x1,1,x2*x2 + y2 * y2,x2,1 ,x3*x3 + y3 * y3,x3,1 });
  //float d = mDeterminant3({ x1*x1 + y1 * y1,x1,y1,x2*x2 + y2 * y2,x2,y2 ,x3*x3 + y3 * y3,x3,y3 });
	
	x_c = b / (2 * a);
	y_c = -c / (2 * a);
  //radius_c = sqrtf((b*b+c*c-4*a*d)/(4*a*a));
	radius_sqr = (x_c - x1)*(x_c - x1) + (y_c - y1)*(y_c - y1);
}

std::vector<std::vector<DNode*>>* Triangulation::DevideNodes()
{
	float s = 0.1;
	float min_x = D3D12_FLOAT32_MAX;
	float max_x = -D3D12_FLOAT32_MAX;
	float min_y = D3D12_FLOAT32_MAX;
	float max_y = -D3D12_FLOAT32_MAX;

	for (auto it = points.cbegin(); it != points.cend(); it++)
	{
		if (it->position.x < min_x)
			min_x = it->position.x;
		else if (it->position.x > max_x)
			max_x = it->position.x;
		if (it->position.y < min_y)
			min_y = it->position.y;
		else if (it->position.y > max_y)
			max_y = it->position.y;
	}
	float a = max_x - min_x;
	float b = max_y - min_y;
	int m = sqrt(s * a/b * points.size());  

	std::vector<std::vector<DNode*>> result(m);

	for (auto it = points.begin(); it != points.end(); it++)
		result[(it->position.x - min_x) / (a / m)].push_back(&(*it));

	return &result;
}


