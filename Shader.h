#pragma once
#include <gl/glew.h>


class Shader{
public:
	struct Program{
		GLuint m_ProgramId; //Opengl handle to program
		GLuint m_VertexId;  //opengl handle to vertex shader
		GLuint m_FragmentId; //opengl handle to fragment shader

		Program() : m_ProgramId(0), m_VertexId(0), m_FragmentId(0){}
	};
	Program m_Program;
public:
	Shader(){}
	~Shader();

	GLint uniformLocation(const char* name);

	void loadFile(const char* vertexShaderPath, const char* fragmentShaderPath);
	static void bind(const Shader* shader);
	static void setInt(GLint location, GLint value);
	static void setFloat(GLint location, GLfloat value);
}; 
