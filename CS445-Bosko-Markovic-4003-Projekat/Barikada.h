#pragma once
#include "ObjekatIgre.h"

class Igrac;

class Barikada : public ObjekatIgre
{
public:
	Barikada(irr::scene::ISceneManager* menadzer);

	void OduzmiDeoZivota();
	void UnistiSe();
	void PopraviSe();

	float RazdaljinaOdIgraca(Igrac* igrac);
	const irr::core::vector3df& Pozicija() const override;

	void SetPozicija(const irr::core::vector3df& pozicija) override;
	void SetRotacija(float rotacija); // Ako barikada treba da se zavrti oko y-ose

private:
	irr::scene::IMeshSceneNode* model;
	bool zvukPusten = false;
};