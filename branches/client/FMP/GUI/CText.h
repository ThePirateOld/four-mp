#pragma once

#include "CGUI.h"

class CText : public CElement
{
	CColor * pString;
public:
	CText( int X, int Y, int Width, int Height, const char * String = NULL, const char * String2 = NULL, const char * Callback = NULL );

	void Draw();
	void PreDraw();
	void MouseMove( CMouse * pMouse );

	void UpdateTheme( int iIndex );
};