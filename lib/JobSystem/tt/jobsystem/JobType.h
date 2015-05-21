#pragma once

class JobType{
private:
	//Debugging
	char* m_Name;
	unsigned int m_Color;

public:
	JobType();
	JobType(const char* name, unsigned int color);
	~JobType();
};