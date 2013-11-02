#include "HDRSurface.h"
#include <FreeImage.h>

namespace Tmpl8{
HDRSurface::HDRSurface() :
	m_Width(0),
	m_Height(0),
	m_Data(nullptr)
{}
HDRSurface::HDRSurface(const char* path){
	FIBITMAP* dib = FreeImage_Load(FIF_HDR, path);
	float* buffer = (float*)FreeImage_GetBits(dib);
	m_Width = FreeImage_GetWidth(dib);
	m_Height = FreeImage_GetHeight(dib);

	m_Data = (float4*)_aligned_malloc(sizeof(float4)*m_Width*m_Height, 16);
	for(int y=0; y<m_Height; ++y){
		float3* line = (float3*)FreeImage_GetScanLine(dib, m_Height - 1 - y);
		for(int x=0; x<m_Width; ++x){
			m_Data[y*m_Width + x ] = float4(line[x].x, line[x].y, line[x].z, 1.0f);
		}
	}
}
const float4* HDRSurface::buffer() const{
	return m_Data;
}
int HDRSurface::width() const{
	return m_Width;
}
int HDRSurface::height() const{
	return m_Height;
}
};