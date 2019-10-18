#include "pch.h"
#include "../Content/Delanuay.h"
#include <stack>

void Triangulation::generateTriangulation2(std::vector<Vector3> _points)
{
	std::vector<DEdge*> minConvexHull;
	std::stack<DEdge*> obsEdges;

	InitializePoints(_points);
	SortPointsByX(&points, 0, points.size() - 1);
	
	//Создание первого треугольника и рёбер в МВО
	DEdge* e1 = new DEdge(&points[0], &points[1]);
	DEdge* e2 = new DEdge(&points[1], &points[2]);
	DEdge* e3 = new DEdge(&points[2], &points[0]);

	minConvexHull.push_back(e1);
	minConvexHull.push_back(e2);
	minConvexHull.push_back(e3);

	DTriangle t = DTriangle(e1, e2, e3);
	t.calcCircum();
	triangles.push_back(t);

	//Добавление новых точек
	int last_add_index = 2;
	std::vector<int> positions(2);

	for (int i = 3; i < points.size(); i++)
	{
		positions = getObsEdges(&minConvexHull, &obsEdges, i, last_add_index);

		for (DEdge* curr_edge = obsEdges.top(); !obsEdges.empty(); obsEdges.pop(), curr_edge = obsEdges.top())
		{
			auto t = curr_edge->triangles.front();
			float radius = t->radius_c;
			float dist = Vector3::length(points[i].position - t->circle_pos);

			if (dist < radius)
			{
				for (int j = 0; j < 3; j++)
					if (t->edges[j] != curr_edge)
						obsEdges.push(t->edges[j]);
			}
		}


	}
}

std::vector<int> Triangulation::getObsEdges(std::vector<DEdge *> *minConvexHull, std::stack<DEdge *> *obsEdges, int index, int last_add_index)
{
	bool visible = true;
	//Поиск видимых рёбер справа
	int end_pos_r = last_add_index;
	for (int j = last_add_index; j < minConvexHull->size(), visible; j++, end_pos_r++)
	{
		for (auto it = minConvexHull->begin(); it != minConvexHull->end(); it++)
		{
			if (*it == (*minConvexHull)[j])
				continue;
			visible &= vectorCollision((*it)->nodes[0]->position, (*it)->nodes[1]->position, points[index].position, (*minConvexHull)[j]->nodes[0]->position);
			visible &= vectorCollision((*it)->nodes[0]->position, (*it)->nodes[1]->position, points[index].position, (*minConvexHull)[j]->nodes[1]->position);
			if (!visible)
				break;
		}
		if (visible)
		{
			obsEdges->push((*minConvexHull)[j]);
			end_pos_r++;
		}
	}
	//Поиск видимых рёбер слева
	visible = true;
	int end_pos_l = last_add_index - 1;
	for (int j = last_add_index - 1; j >= 0, visible; j--)
	{
		for (auto it = minConvexHull->begin(); it != minConvexHull->end(); it++)
		{
			if (*it == (*minConvexHull)[j])
				continue;
			visible &= vectorCollision((*it)->nodes[0]->position, (*it)->nodes[1]->position, points[index].position, (*minConvexHull)[j]->nodes[0]->position);
			visible &= vectorCollision((*it)->nodes[0]->position, (*it)->nodes[1]->position, points[index].position, (*minConvexHull)[j]->nodes[1]->position);
			if (!visible)
				break;
		}
		if (visible)
		{
			obsEdges->push((*minConvexHull)[j]);
			end_pos_l--;
		}
	}
	return std::vector<int>{end_pos_l, end_pos_r};
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
	
	circle_pos.x = b / (2 * a);
	circle_pos.y = -c / (2 * a);
	circle_pos.z = 0;
  //radius_c = sqrtf((b*b+c*c-4*a*d)/(4*a*a));
	radius_c = Vector3::length(circle_pos - Vector3(x1,y1,0));
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

int DEdge::getTriangleIndex(DTriangle* t)
{
	if (triangles[0] == t)
		return 0;
	else
		return 1;
}


