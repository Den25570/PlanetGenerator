#include "VAO.hpp"

void VAO::initializeBuffers(std::vector<int> attributes_size, std::vector<GLfloat> * vertices, std::vector<GLuint> * indices) {
	//Создаём буферы в памяти
	glGenVertexArrays(1, &VAO_index);
	glGenBuffers(1, &VBO_index);
	if (indices != NULL)glGenBuffers(1, &EBO_index);

	//Привязываем индексы к буферам
	glBindVertexArray(VAO_index);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_index);
	if (indices != NULL) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_index);

	//Привязываем данные к буферам
	glBufferData(GL_ARRAY_BUFFER, vertices->size() * sizeof(GLfloat), &(*vertices)[0], GL_STATIC_DRAW);
	if (indices != NULL) glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices->size() * sizeof(GLuint), &(*indices)[0], GL_STATIC_DRAW);

	// Устанавливаем указатели на вершинные атрибуты 
	int total_attributes_size = 0;
	for (int i = 0; i < attributes_size.size(); i++)
		total_attributes_size += attributes_size[i];

	for (int i = 0, prev_attributes_size = 0; i < attributes_size.size(); prev_attributes_size += attributes_size[i], i++)
	{
		glVertexAttribPointer(i, attributes_size[i], GL_FLOAT, GL_FALSE, total_attributes_size * sizeof(GLfloat), (GLvoid*)(prev_attributes_size * sizeof(GLfloat)));
		glEnableVertexAttribArray(i);
	}
	// Отвязываем VAO
	glBindVertexArray(0);
}

void VAO::use(int polygon_mode_side, int polygon_mode) {
	shader->Use();
	glBindVertexArray(VAO_index);
	glPolygonMode(polygon_mode_side, polygon_mode);
}

void VAO::draw(int draw_mode) {
	if (indices_size == 0) 
		glDrawArrays(draw_mode, 0, vertices_size);
	else 
		glDrawElements(draw_mode, indices_size, GL_UNSIGNED_INT, 0);
}