#ifndef BUFFEROBJECTS_H
#define BUFFEROBJECTS_H

#include <glad/glad.h>

class VBO {
public:
	GLuint ID;
	GLfloat* vertices;
	GLsizeiptr size;
	
	VBO();
	VBO(GLfloat* vertices, GLsizeiptr size);

	void Bind();
	void Update();
	void Unbind();
	void Delete();
};

class EBO {
public:
	GLuint ID;
	GLuint* indices;
	GLsizeiptr size;
	int to_draw;
	
	EBO();
	EBO(GLuint* indices, GLsizeiptr size);

	void Bind();
	void Update();
	void Unbind();
	void Delete();
};

class UBO {
public:
	GLuint ID;
	GLfloat* vertices;
	GLsizeiptr size;
	
	UBO();
	UBO(GLfloat* vertices, GLsizeiptr size);

	void Bind();
	void Unbind();
	void Delete();
};

class VAO {
public:
	GLuint ID;
	VBO vbo;
	VBO cbo;
	EBO ebo;

	VAO();
	VAO(GLfloat* vertices, GLsizeiptr verticesSize, GLuint* indices, GLsizeiptr indicesSize);
	VAO(GLfloat* vertices, GLsizeiptr verticesSize, GLfloat* colors, GLsizeiptr colorsSize, \
		GLuint* indices, GLsizeiptr indicesSize);
	VAO(GLfloat* vertices, GLsizeiptr verticesSize, GLfloat* colors, GLsizeiptr colorsSize);
	VAO(GLfloat* vertices, GLsizeiptr verticesSize);
	void NewIndeces(GLuint* indeces, GLsizeiptr indecesSize);
	void LinkAttrib(VBO& VBO, GLuint attrIdx, GLuint numComponents, \
		GLenum type, GLsizeiptr stride, void* offset);
	void LinkMat4(VBO& VBO, GLuint attridx);
	void DrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices);
	void DrawElements(GLenum mode);
	void DrawArrays(GLenum mode, GLint first, GLsizei count);

	void Bind();
	void Unbind();
	void Delete();
};

#endif