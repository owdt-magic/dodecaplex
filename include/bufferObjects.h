#ifndef BUFFEROBJECTS_H
#define BUFFEROBJECTS_H

#include <glad/glad.h>
#include <iostream>
#include <vector>

struct CPUBufferPair {
    GLfloat* v_buff;
    GLuint*  i_buff;
    int v_head, i_head; 
    uint offset;
	size_t v_max, i_max;
	CPUBufferPair() {};
    CPUBufferPair(size_t v_size, size_t i_size);
    void reset();
	void setHead(int v, int i, int o);
};

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
	VAO(CPUBufferPair& buffer_writer);
	VAO(GLfloat* vertices, GLsizeiptr verticesSize, GLuint* indices, GLsizeiptr indicesSize);
	VAO(GLfloat* vertices, GLsizeiptr verticesSize, GLfloat* colors, GLsizeiptr colorsSize, \
		GLuint* indices, GLsizeiptr indicesSize);
	VAO(GLfloat* vertices, GLsizeiptr verticesSize, GLfloat* colors, GLsizeiptr colorsSize);
	VAO(GLfloat* vertices, GLsizeiptr verticesSize);
	void NewIndeces(GLuint* indeces, GLsizeiptr indecesSize);
	void LinkVecs(std::vector<int> pattern, int total);
	void LinkVecs(std::vector<int> pattern);
	void LinkAttrib(VBO& VBO, GLuint attrIdx, GLuint numComponents, \
		GLenum type, GLsizeiptr stride, void* offset);
	void LinkMat4(VBO& VBO, GLuint attridx);
	void DrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices);
	void DrawElements(GLenum mode);
	void DrawArrays(GLenum mode, GLint first, GLsizei count);
	void UpdateAttribSubset(VBO& VBO, GLintptr offset, GLsizeiptr size, const void* data);
	void UpdateAttribSubset(EBO& EBO, GLintptr offset, GLsizeiptr size, const void* data);

	void Bind();
	void Unbind();
	void Delete();
};

VAO rasterPipeVAO();

#endif