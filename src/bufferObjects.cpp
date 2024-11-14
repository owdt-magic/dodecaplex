#include "bufferObjects.h"

// Vertex Buffer Object
VBO::VBO() {}
VBO::VBO(GLfloat* vertices, GLsizeiptr size) {
	glGenBuffers(1, &ID);
	glBindBuffer(GL_ARRAY_BUFFER, ID);
	glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
}
void VBO::Bind() 	{ glBindBuffer(GL_ARRAY_BUFFER, ID); }
void VBO::Update()  { glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW); }
void VBO::Unbind()	{ glBindBuffer(GL_ARRAY_BUFFER, 0); }
void VBO::Delete() 	{ glDeleteBuffers(1, &ID); }

// Element(Index) Buffer Object
EBO::EBO() {}
EBO::EBO(GLuint* indices, GLsizeiptr size) {
	glGenBuffers(1, &ID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW);
	to_draw = size/sizeof(GLuint);
}
void EBO::Bind()	{ glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID); }
void EBO::Update()  { glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW); }
void EBO::Unbind()	{ glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }
void EBO::Delete()  { glDeleteBuffers(1, &ID); }

// Uniform Buffer Object
UBO::UBO() {}
UBO::UBO(GLfloat* vertices, GLsizeiptr size) {
	glGenBuffers(1, &ID);
	glBindBuffer(GL_UNIFORM_BUFFER, ID);
	glBufferData(GL_UNIFORM_BUFFER, size, vertices, GL_STATIC_DRAW);
}
void UBO::Bind() 	{ glBindBuffer(GL_UNIFORM_BUFFER, ID); }
void UBO::Unbind()	{ glBindBuffer(GL_UNIFORM_BUFFER, 0); }
void UBO::Delete() 	{ glDeleteBuffers(1, &ID); }

// Vertext Array Object
VAO::VAO() {}
VAO::VAO(GLfloat* vertices, GLsizeiptr verticesSize, \
			GLuint* indices, GLsizeiptr indicesSize) {
	glGenVertexArrays(1, &ID);
	glBindVertexArray(ID);
	// Create VBO and EBO with the provided data
	vbo = VBO(vertices, verticesSize);
	ebo = EBO(indices, indicesSize);
}
VAO::VAO(GLfloat* vertices, GLsizeiptr verticesSize, \
		GLfloat* colors, GLsizeiptr colorsSize, \
		GLuint* indices, GLsizeiptr indicesSize) : vbo(), cbo(), ebo() {
	glGenVertexArrays(1, &ID);
	glBindVertexArray(ID);
	vbo = VBO(vertices, verticesSize);
	cbo = VBO(colors, colorsSize);
	ebo = EBO(indices, indicesSize);
}
VAO::VAO(GLfloat* vertices, GLsizeiptr verticesSize, \
			GLfloat* colors, GLsizeiptr colorsSize) : vbo(), cbo() {
	glGenVertexArrays(1, &ID);
	glBindVertexArray(ID);
	// Create VBOs with the provided data, doesn't HAVE to be colors btw...
	vbo = VBO(vertices, verticesSize);
	cbo = VBO(colors, colorsSize);
}
VAO::VAO(GLfloat* vertices, GLsizeiptr verticesSize) : vbo(), ebo() { 
	glGenVertexArrays(1, &ID);
	glBindVertexArray(ID);
	vbo = VBO(vertices, verticesSize);
}
void VAO::NewIndeces(GLuint* indeces, GLsizeiptr indecesSize) {
	glGenVertexArrays(1, &ID);
	glBindVertexArray(ID);
	ebo = EBO(indeces, indecesSize);
}
void VAO::LinkAttrib(VBO& VBO, GLuint attrIdx, GLuint numComponents, \
 						GLenum type, GLsizeiptr stride, void* offset) {
	/* Think of these parameters as descibing how to index
	 into the vertex(VBO) arrays*/
	VBO.Bind();
	glVertexAttribPointer(attrIdx, numComponents, type, GL_FALSE, stride, offset);
	glEnableVertexAttribArray(attrIdx);
	VBO.Unbind();
	/*Attribute index, Num attributes, Type attributes, Stride Offset*/
}
void VAO::LinkMat4(VBO& VBO, GLuint attrIdx) {
	/*We can make some assumptions for a Mat4 that don't generalize to all
	VBO bindings...*/
	VBO.Bind();
	glEnableVertexAttribArray(attrIdx);
	glEnableVertexAttribArray(attrIdx + 1);
	glEnableVertexAttribArray(attrIdx + 2);
	glEnableVertexAttribArray(attrIdx + 3);
	glVertexAttribPointer(attrIdx,     4, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*16, (void*)0);
	glVertexAttribPointer(attrIdx + 1, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*16, (void*)(sizeof(GLfloat)*4));
	glVertexAttribPointer(attrIdx + 2, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*16, (void*)(sizeof(GLfloat)*8));
	glVertexAttribPointer(attrIdx + 3, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*16, (void*)(sizeof(GLfloat)*12));
	glVertexAttribDivisor(attrIdx, 1);
	glVertexAttribDivisor(attrIdx+1, 1);
	glVertexAttribDivisor(attrIdx+2, 1);
	glVertexAttribDivisor(attrIdx+3, 1);
	VBO.Unbind();
	/*NOTE: When you bind a mat4, you relenquish the subsequent attribute indeces for each column*/
};
void VAO::DrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices) {
	Bind();
	glDrawElements(mode, count, type, indices); 
    Unbind();
}
void VAO::DrawElements(GLenum mode) {
	// If you are lazy you can use this? (ME)
	Bind();
	glDrawElements(mode, ebo.to_draw, GL_UNSIGNED_INT, 0); 
    Unbind();
}	
void VAO::DrawArrays(GLenum mode, GLint first, GLsizei count) {
	Bind();
    glDrawArrays(mode, first, count);
    Unbind();
}
void VAO::Bind() 	{ glBindVertexArray(ID);}
void VAO::Unbind() 	{ glBindVertexArray(0); }
void VAO::Delete() 	{ glDeleteVertexArrays(1, &ID); }