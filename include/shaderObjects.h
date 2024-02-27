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
	
	EBO();
	EBO(GLuint* indices, GLsizeiptr size);

	void Bind();
	void Update();
	void Unbind();
	void Delete();
};

class VAO {
public:
	GLuint ID;
	VBO vbo;
	VBO cbo;
	EBO ebo;
	
	VAO(GLfloat* vertices, GLsizeiptr verticesSize, GLuint* indices, GLsizeiptr indicessSize);
	VAO(GLfloat* vertices, GLsizeiptr verticesSize, GLfloat* colors, GLsizeiptr colorsSize);
	VAO(GLfloat* vertices, GLsizeiptr verticesSize);
	void LinkAttrib(VBO& VBO, GLuint attrIdx, GLuint numComponents, \
		GLenum type, GLsizeiptr stride, void* offset);
	void DrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices);
	void DrawArrays(GLenum mode, GLint first, GLsizei count);

	void Bind();
	void Unbind();
	void Delete();
};