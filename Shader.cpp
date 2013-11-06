#pragma warning (disable : 4530) //C++ exception handler used, but no unwind semantics

#include "Shader.h"

#include <fstream>
#include <string>
#include <algorithm>
#include <stdarg.h>
#include <string>

bool compileShader(const char* shaderPath, GLint shaderId, const std::string& shaderSource){
	const char* srcPtr = shaderSource.c_str();
	const int srcLen = shaderSource.length();
	glShaderSource(shaderId, 1, &srcPtr, &srcLen);
	glCompileShader(shaderId);
	GLint status;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);
	if(status != GL_TRUE){
		GLint logLength;
		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logLength);
		char* buffer = new char[logLength];
		glGetShaderInfoLog(shaderId, logLength, nullptr, buffer);
		printf("Compiling shader(%s) failed: %s\n", shaderPath, buffer);
		delete[] buffer;
		return false;
	}
	return true;
}
bool readEntireFile(const char* path, std::string* out_string){
	std::ifstream ifs(path);
	if(!ifs.good()) return false;
	ifs.seekg(0, std::ios::end);
	out_string->reserve((unsigned int)ifs.tellg());
	ifs.seekg(0, std::ios::beg);
	out_string->assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
	ifs.close();
	return true;
}
Shader::~Shader(){
	glDeleteProgram(m_Program.m_ProgramId);
	glDeleteShader(m_Program.m_FragmentId);
	glDeleteShader(m_Program.m_VertexId);
}
void Shader::loadFile(const char* vertexPath, const char* fragmentPath){
	std::string vertex, fragment;

	if(!readEntireFile(vertexPath, &vertex)){
		printf("Failed to load vertex shader: %s\n", vertexPath);
		return;
	}
	if(!readEntireFile(fragmentPath, &fragment)){
		printf("Failed to load fragment shader: %s\n", fragmentPath);
		return;
	}

	if(!m_Program.m_VertexId) m_Program.m_VertexId = glCreateShader(GL_VERTEX_SHADER);
	if(!m_Program.m_FragmentId) m_Program.m_FragmentId = glCreateShader(GL_FRAGMENT_SHADER);
	if(!m_Program.m_ProgramId) m_Program.m_ProgramId = glCreateProgram();

	if(compileShader(vertexPath, m_Program.m_VertexId, vertex) && compileShader(fragmentPath, m_Program.m_FragmentId, fragment)){
		glAttachShader(m_Program.m_ProgramId, m_Program.m_VertexId);
		glAttachShader(m_Program.m_ProgramId, m_Program.m_FragmentId);
		glLinkProgram(m_Program.m_ProgramId);

		//Test if linking successfull
		GLint linkStatus;
		glGetProgramiv(m_Program.m_ProgramId, GL_LINK_STATUS, &linkStatus);
		if(linkStatus != GL_TRUE){
			GLint logLength;
			glGetProgramiv(m_Program.m_ProgramId, GL_INFO_LOG_LENGTH, &logLength);
			char* buffer = new char[logLength];
			glGetProgramInfoLog(m_Program.m_ProgramId, logLength, nullptr, buffer);
			printf("Shaders(%s, %s) failed to link: %s\n", vertexPath, fragmentPath, buffer);
			delete[] buffer;
		}
	}
}
void Shader::bind(const Shader* shader){
	if(shader){
		glUseProgram(shader->m_Program.m_ProgramId);
	}else{
		glUseProgram(0);
	}
}
GLint Shader::uniformLocation(const char* path){
	return glGetUniformLocation(m_Program.m_ProgramId, path);
}
void Shader::setInt(GLint location, GLint value){
	glUniform1i(location, value);
}
void Shader::setFloat(GLint location, GLfloat value){
	glUniform1f(location, value);
}
void Shader::setVec2(GLint location, const float2& value){
	glUniform2fv(location, 1, reinterpret_cast<const GLfloat*>(&value));
}