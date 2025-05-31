#include "bufferObjects.h"
#include <numeric>

CPUBufferPair::CPUBufferPair(size_t v_size, size_t i_size) : v_max(v_size), i_max(i_size){
    v_buff = (GLfloat*) malloc(v_size);
    i_buff = (GLuint*)  malloc(i_size);

    if ((v_buff == NULL) || (i_buff == NULL)) {
        throw std::runtime_error("Failed to initialize buffers for dodecaplex with sizes: "+
            std::to_string(v_size)+" and "+ std::to_string(i_size));
    }
    reset();
}
void CPUBufferPair::reset(){
    v_head = 0;
    i_head = 0;
    offset=0;
}
void CPUBufferPair::setHead(int v, int i, int o){
	v_head = v;
	i_head = i; 
	offset = o;
}

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
VAO::VAO(CPUBufferPair& buffer_writer) {
	glGenVertexArrays(1, &ID);
	glBindVertexArray(ID);
	// Create VBO and EBO with the provided data
	vbo = VBO(buffer_writer.v_buff, buffer_writer.v_head*sizeof(GLfloat));
	ebo = EBO(buffer_writer.i_buff, buffer_writer.i_head*sizeof(GLuint));
}
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
void VAO::LinkVecs(std::vector<int> pattern, int total) {
	int subtotal = 0;
	vbo.Bind();
	for (int idx = 0; idx < pattern.size(); ++idx){		
		glVertexAttribPointer(idx, pattern[idx], GL_FLOAT, GL_FALSE, total*sizeof(float), (void*)(subtotal*sizeof(float)));
		glEnableVertexAttribArray(idx);	
		subtotal += pattern[idx];
	}
	vbo.Unbind();
}
void VAO::LinkVecs(std::vector<int> pattern) {	
	int total = std::accumulate(pattern.begin(), pattern.end(), 0);	
	LinkVecs(pattern, total);
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
void VAO::UpdateAttribSubset(VBO& VBO, GLintptr offset, GLsizeiptr size, const void* data) {
	VBO.Bind();
	glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
	VBO.Unbind();
}
void VAO::UpdateAttribSubset(EBO& EBO, GLintptr offset, GLsizeiptr size, const void* data) {
	EBO.Bind();
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, size, data);
}
void VAO::Bind() 	{ glBindVertexArray(ID);}
void VAO::Unbind() 	{ glBindVertexArray(0); }
void VAO::Delete() 	{ glDeleteVertexArrays(1, &ID); }

VAO rasterPipeVAO(){
	// Simple VAO to cover the screen, good for raster only shader art.
    GLfloat quadVertices[] = {		
        -1.0f, -1.0f,  // bottom left
        1.0f, -1.0f,  // bottom right
        1.0f,  1.0f,  // top right
        -1.0f,  1.0f   // top left
    };
    GLuint quadIndices[] = {
        0, 1, 2,  // first triangle
        2, 3, 0   // second triangle
    };
    VAO fullscreenQuad(quadVertices, sizeof(quadVertices), quadIndices, sizeof(quadIndices));
    fullscreenQuad.LinkVecs({2}, 2);

	return fullscreenQuad;
};