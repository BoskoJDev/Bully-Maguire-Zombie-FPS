#include "Aplikacija.h"
#include "Unos.h"
#include <iostream>
#include <Windows.h>

static Unos& unos = Unos::Instanca();

Aplikacija::Aplikacija(bool vsync)
{
	// Sakrivanje konzole
	ShowWindow(GetConsoleWindow(), SW_HIDE);

	irr::SIrrlichtCreationParameters parametriKreiranja;
	parametriKreiranja.DeviceType = irr::EIDT_BEST;
	parametriKreiranja.DriverType = irr::video::EDT_OPENGL;
	parametriKreiranja.LoggingLevel = irr::ELL_NONE;
	parametriKreiranja.Vsync = vsync;
	parametriKreiranja.WindowSize = irr::core::dimension2du(900, 600);
	parametriKreiranja.ZBufferBits = 32U;
	if (this->uredjaj = irr::createDeviceEx(parametriKreiranja); !this->uredjaj)
	{
		std::cout << "Neuspela inicijalizacija igre!\n";
		exit(EXIT_FAILURE);
	}

	unos.SetTrenutnaScena(this->uredjaj, TipScene::POCETNI_MENI);
	this->uredjaj->setEventReceiver(&unos);
}

void Aplikacija::Pokreni()
{
	while (this->uredjaj->run())
		unos.GetTrenutnaScena()->Azuriraj();
}