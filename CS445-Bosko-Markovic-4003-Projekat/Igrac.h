#pragma once
#include "ObjekatIgre.h"
#include <unordered_map>

enum class Oruzje : char
{
	PISTOLJ, SACMARA, PUSKA, MITRALJEZ, KIRIJA_SMRTI
};

enum class TipMoci : char
{
	NISTA = 5, NEOGRANICENA_MUNICIJA = 6, DOPUNA_MUNICIJE = 7
};

class Scena;

class Igrac : public ObjekatIgre
{
public:
	Igrac(irr::scene::ISceneManager* menadzer);

	void SetPoeni(int poeni);

	void IzgovoriNesto();

	void NapuniOruzje();
	void Pucaj(TipMoci tip);

	void SetScena(Scena* scena);

	void SetOruzje(Oruzje oruzje);
	void SetPozicija(const irr::core::vector3df& pozicija) override;
	
	void DuplirajMuniciju();

	const bool MagacinPrazan();
	const int Poeni();
	const int Meci();
	const int Municija();
	Oruzje TrenutnoOruzje() const;
	const irr::core::vector3df& Pozicija() const override;
	irr::scene::ICameraSceneNode* Kamera() const;

private:
	void ResetujOruzja(bool duplirajMuniciju = false);

	int poeni = 0;
	Scena* scena;
	irr::scene::ICameraSceneNode* kamera;
	Oruzje oruzje = Oruzje::PISTOLJ;
	std::unordered_map<Oruzje, std::pair<int, int>> mapaOruzja;
};