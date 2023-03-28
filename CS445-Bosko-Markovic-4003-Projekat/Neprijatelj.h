#pragma once
#include "Igrac.h"
#include "CAnimatedMeshSceneNode.h"
#include <array>

class Barikada;
class Igrac;

enum class TipScene : char
{
	POCETNI_MENI, HIGHSCORE_MENI, KORIDOR, KABINA
};

class Neprijatelj : public irr::scene::CAnimatedMeshSceneNode, public ObjekatIgre
{
public:
	Neprijatelj(irr::scene::ISceneManager* menadzer, Igrac* igrac, Barikada* barikada,
		irr::core::vector3df& poz);

	static void SetScena(Scena* scena);

	void SetBarikadaZaobidjena(bool zaobidjeno);
	bool BarikadaZaobidjena() const;

	Barikada* Meta() const;

	void SetPozicija(const irr::core::vector3df& pozicija) override;
	const irr::core::vector3df& Pozicija() const override;

	void SetVelicina(float skalar);

	void SetJeSef(bool jeSef);
	bool JeSef();

	void SetZiv(bool jeZiv);
	bool JeZiv() const;

	void IdiKaBarikadi(float deltaVreme);
	void IdiKaIgracu(float deltaVreme);

private:
	static Scena* scena;

	void Napadni(Barikada* barikada);
	void Napadni(Igrac* igrac);

	bool ziv = false;
	bool trci = false;
	bool napada = false;
	bool sef = false;
	int napad = 50;
	bool barikadaZaobidjena = false;
	int brzina;
	irr::scene::IAnimatedMeshSceneNode* model;
	irr::core::vector3df trajekcija;
	Barikada* metaBarikada;
	Igrac* metaIgrac;
};