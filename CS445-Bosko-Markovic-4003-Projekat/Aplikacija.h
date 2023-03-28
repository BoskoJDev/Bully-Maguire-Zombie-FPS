#pragma once
#include <IrrLicht/irrlicht.h>

class Aplikacija
{
public:
	Aplikacija(bool vsync = false);

	void Pokreni();

private:
	irr::IrrlichtDevice* uredjaj;
};