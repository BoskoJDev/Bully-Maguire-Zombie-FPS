#include "Igrac.h"
#include "ID_za_zrak.h"
#include "PogonZvuka.h"
#include <iostream>
#include <random>
#include "Scena.h"

using namespace irr;

static PogonZvuka& pz = PogonZvuka::Instanca();

Igrac::Igrac(scene::ISceneManager* menadzer)
{
	this->mapaOruzja.reserve(5);
	this->ResetujOruzja();

	SKeyMap kontrole[] = {
		{ EKA_MOVE_FORWARD, KEY_KEY_W },
		{ EKA_MOVE_BACKWARD, KEY_KEY_S },
		{ EKA_STRAFE_LEFT, KEY_KEY_A },
		{ EKA_STRAFE_RIGHT, KEY_KEY_D },
		{ EKA_JUMP_UP, KEY_SPACE }
	};
	this->kamera = menadzer->addCameraSceneNodeFPS(nullptr, 100.0f, 0.07f, ID_ZrakIzbegava,
		kontrole, 5, false, 0.1f);

	scene::ISceneNodeAnimator* animator = menadzer->createCollisionResponseAnimator(
		metaSelektor,
		this->kamera,
		core::vector3df(2.0f, 7.0f, 2.0f),
		core::vector3df(0.0f, -2.0f, 0.0f)
	);
	this->kamera->addAnimator(animator);
	animator->drop();
}

void Igrac::SetPoeni(int poeni) { this->poeni = poeni; }

void Igrac::IzgovoriNesto()
{
	for (const auto&[indeks, s] : pz.MagvajerZvuci())
	{
		if (pz.ZvukSePusta(SvrhaZvuka::BULI_MAGVAJER, indeks))
			return;
	}

	static auto nasumicanZvuk = []() {
		std::random_device uredjaj;
		std::mt19937 generator(uredjaj());
		std::uniform_int_distribution<std::mt19937::result_type> distribucija(0, 7);

		return distribucija(generator);
	};

	pz.Pusti2DZvuk(SvrhaZvuka::BULI_MAGVAJER, nasumicanZvuk());
}

void Igrac::SetOruzje(Oruzje oruzje)
{
	if (oruzje < this->oruzje)
	{
		pz.Pusti2DZvuk(SvrhaZvuka::MUZIKA, 4);
		pz.Pusti2DZvuk(SvrhaZvuka::MUZIKA, 5);
	}

	if (oruzje == Oruzje::KIRIJA_SMRTI)
		pz.Pusti2DZvuk(SvrhaZvuka::BULI_MAGVAJER, 8);

	this->oruzje = oruzje;
	this->ResetujOruzja();
	this->scena->DodajPoene(-60);
}

void Igrac::SetPozicija(const core::vector3df& pozicija) { this->kamera->setPosition(pozicija); }

void Igrac::DuplirajMuniciju()
{
	pz.Pusti2DZvuk(SvrhaZvuka::PUCANJ, 8);
	this->ResetujOruzja(true);
}

const bool Igrac::MagacinPrazan() { return this->mapaOruzja[this->oruzje].first == 0; }

const int Igrac::Poeni() { return this->poeni; }

const int Igrac::Meci() { return this->mapaOruzja[this->oruzje].first; }

const int Igrac::Municija() { return this->mapaOruzja[this->oruzje].second; }

void Igrac::NapuniOruzje()
{
	std::pair<int, int>& municija = this->mapaOruzja[this->oruzje];
	if (municija.second == 0)
		return;

	int municijaMagacina{};
	switch (this->oruzje)
	{
		using enum Oruzje;

		case PISTOLJ: municijaMagacina = 8;
			break;
		case SACMARA: municijaMagacina = 12;
			break;
		case PUSKA: municijaMagacina = 30;
			break;
		case MITRALJEZ: municijaMagacina = 75;
			break;
		case KIRIJA_SMRTI: municijaMagacina = 50;
			break;
	}

	if (municija.first == municijaMagacina)
		return;

	for (int i = municija.first; i < municijaMagacina;)
	{
		if (municija.second == 0)
			break;

		municija.first = ++i;
		municija.second--;
	}

	if (!pz.ZvukSePusta(SvrhaZvuka::PUCANJ, 7 - (this->oruzje != Oruzje::KIRIJA_SMRTI)))
		pz.Pusti2DZvuk(SvrhaZvuka::PUCANJ, 7 - (this->oruzje != Oruzje::KIRIJA_SMRTI));
}

void Igrac::Pucaj(TipMoci tip)
{
	if (this->mapaOruzja[this->oruzje].first == 0) // Ako je magacin prazan
	{
		pz.Pusti2DZvuk(SvrhaZvuka::PUCANJ, 0);
		return;
	}

	this->mapaOruzja[this->oruzje].first -= (this->oruzje == Oruzje::PUSKA ? 3 : 1) * 
		(tip != TipMoci::NEOGRANICENA_MUNICIJA);
	pz.Pusti2DZvuk(SvrhaZvuka::PUCANJ, int(this->oruzje) + 1);
}

void Igrac::SetScena(Scena* scena) { this->scena = scena; }

Oruzje Igrac::TrenutnoOruzje() const { return this->oruzje; }

const core::vector3df& Igrac::Pozicija() const { return this->kamera->getPosition(); }

scene::ICameraSceneNode* Igrac::Kamera() const { return this->kamera; }

void Igrac::ResetujOruzja(bool duplirajMuniciju)
{
	this->mapaOruzja[Oruzje::PISTOLJ] = { 8, 40 * (1 + duplirajMuniciju) };
	this->mapaOruzja[Oruzje::SACMARA] = { 12, 48 * (1 + duplirajMuniciju) };
	this->mapaOruzja[Oruzje::PUSKA] = { 30, 150 * (1 + duplirajMuniciju) };
	this->mapaOruzja[Oruzje::MITRALJEZ] = { 75, 300 * (1 + duplirajMuniciju) };
	this->mapaOruzja[Oruzje::KIRIJA_SMRTI] = { 50, 150 * (1 + duplirajMuniciju) };
}