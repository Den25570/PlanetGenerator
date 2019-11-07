#include "Delanuay.h"

void Triangulation::generateTriangulation2(std::vector<Vector3> _points)
{
	std::vector<DEdge*> minConvexHull;
	std::vector<DEdge*> potentialMCH;
	std::stack<DEdge*> obsEdges;

	InitializePoints(_points);
	SortPointsByX(0, points.size() - 1);
	
	//Создание первого треугольника и рёбер в МВО
	DEdge* e1 = new DEdge(&points[0], &points[1]);
	DEdge* e2 = new DEdge(&points[1], &points[2]);
	DEdge* e3 = new DEdge(&points[2], &points[0]);

	minConvexHull.push_back(e1);
	minConvexHull.push_back(e2);
	minConvexHull.push_back(e3);

	AddTriangle(e1, e2, e3);

	//Добавление новых точек
	int last_add_index = 2;

	for (int i = 3; i < points.size(); i++)
	{
		getObsEdges(&minConvexHull, &obsEdges, i, last_add_index);

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
				delete t;

				//Удаление из МВО
				auto search_res = std::find(minConvexHull.begin(), minConvexHull.end(), e3);
				if (search_res != minConvexHull.end())
					minConvexHull.erase(search_res);

				delete curr_edge;
			}
			else
			{				
				e1 = AddEdge(&points[0], &points[i]);
				e2 = AddEdge(&points[i], &points[1]);
				e3 = curr_edge;

				auto search_res = std::find(potentialMCH.begin(), potentialMCH.end(), e1);
				if (search_res == potentialMCH.end())
					potentialMCH.push_back(e1);
				else
					potentialMCH.erase(search_res);

				search_res = std::find(potentialMCH.begin(), potentialMCH.end(), e2);
				if (search_res == potentialMCH.end())
					potentialMCH.push_back(e2);
				else
					potentialMCH.erase(search_res);

				//Удаление из МВО
				search_res = std::find(minConvexHull.begin(), minConvexHull.end(), e3);
				if (search_res != minConvexHull.end())
					minConvexHull.erase(search_res);

				AddTriangle(AddEdge(&points[0], &points[i]), AddEdge(&points[i], &points[1]), curr_edge);
			}										
		}
		minConvexHull.push_back(potentialMCH[0]);
		minConvexHull.push_back(potentialMCH[1]);
		potentialMCH.clear();
	}
}

DEdge* Triangulation::AddEdge(DNode* p1, DNode* p2)
{
	DEdge* edge = nodesConnected(p1, p2);
	if (edge == NULL)
		edge = new DEdge(p1, p2);
	return edge;
}

DTriangle* Triangulation::AddTriangle(DEdge * e1, DEdge * e2, DEdge * e3)
{
	DTriangle* t = new DTriangle(e1, e2, e3);
	t->calcCircum();
	triangles.push_back(t);
	return t;
}

void Triangulation::getObsEdges(std::vector<DEdge *> *minConvexHull, std::stack<DEdge *> *obsEdges, int index, int last_add_index)
{
	for (int j = last_add_index; j < minConvexHull->size(); j++)
	{
		bool visible = true;
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
			obsEdges->push((*minConvexHull)[j]);
	}
}

void Triangulation::InitializePoints(std::vector<Vector3> _points)
{
	points.reserve(_points.size());
	for (auto it = _points.cbegin(); it != _points.cend(); it++)
		points.push_back(DNode(*it));
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
	float min_x = std::numeric_limits<float>::max();
	float max_x = -std::numeric_limits<float>::max();
	float min_y = std::numeric_limits<float>::max();
	float max_y = -std::numeric_limits<float>::max();

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
	for (int i = 0; i < triangles.size(); i++)
		if (triangles[i] == t)
			return i;
	return -1;
}

int DEdge::getNodeIndex(DNode* n)
{
	return n == nodes[1];
}

DEdge* nodesConnected(DNode* n1, DNode* n2)
{
	for (auto edge : n1->edges)
		for (auto node : edge->nodes)
			if (node == n2)
				return edge;
	return NULL;
}


