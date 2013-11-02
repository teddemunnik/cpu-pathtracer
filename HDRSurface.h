#pragma once
#include "template.h"

namespace Tmpl8{
class HDRSurface{
private:
	int m_Width;
	int m_Height;
	float4* m_Data;

public:
	HDRSurface();
	HDRSurface(const char* path);

	const float4* buffer() const;
	int width() const;
	int height() const;
};
};