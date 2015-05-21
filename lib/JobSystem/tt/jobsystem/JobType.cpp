#include "JobType.h"
#include <cstring>

JobType::JobType() :
	m_Name(nullptr),
	m_Color(0)
{}
JobType::JobType(const char* name, unsigned int color) :
	m_Color(color)
{
	size_t len  = strlen(name)+1; 
	name = new char[len];
	memcpy(m_Name, name, len); 
}