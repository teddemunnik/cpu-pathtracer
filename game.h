// Template for GP1, version 1
// IGAD/NHTV - Jacco Bikker - 2006-2013

#pragma once

namespace Tmpl8 {

class Surface;
class Game
{
public:
	void SetTarget( Surface* a_Surface ) { m_Surface = a_Surface; }
	void Init();
	void Draw2DView();
	void Tick( float a_DT );
	void Draw();
	void KeyDown( unsigned int code );
	void KeyUp( unsigned int code );
	void MouseMove( unsigned int x, unsigned int y ){}
	void MouseUp( unsigned int button ) {}
	void MouseDown( unsigned int button ) {}
private:
	Surface* m_Surface;
};

}; // namespace Tmpl8