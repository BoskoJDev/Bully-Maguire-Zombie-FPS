#include <iostream>
#include "Igrac.h"
#include "Scena.h"
#include "Neprijatelj.h"
#include "ID_za_zrak.h"
#include "Barikada.h"
#include "PogonZvuka.h"
#include <random>
#include "Unos.h"

using namespace irr;

static PogonZvuka& pz = PogonZvuka::Instanca();
static scene::ISceneManager* menadzer;

Scena* Neprijatelj::scena;

Neprijatelj::Neprijatelj(scene::ISceneManager* menadzerScene, Igrac* igrac, Barikada* barikada,
	core::vector3df& poz) : CAnimatedMeshSceneNode(nullptr, nullptr, menadzerScene, ID_ZrakPogadja)
{
	if (!menadzer)
		menadzer = menadzerScene;

	scene::IAnimatedMesh* mes = menadzer->getMesh("neprijatelj/Archattack.md2");

	this->model = menadzer->addAnimatedMeshSceneNode(mes, glavniCvor, ID_ZrakPogadja);

	TipScene tipScene = scena->Tip();
	this->model->setMaterialFlag(video::EMF_LIGHTING, tipScene == TipScene::KABINA);
	this->model->setMaterialFlag(video::EMF_FOG_ENABLE, tipScene == TipScene::KORIDOR);
	this->model->setMaterialTexture(0, menadzer->getVideoDriver()->getTexture("neprijatelj/archvile.png"));
	this->SetVelicina(0.3f);
	this->metaBarikada = barikada;
	this->model->setPosition(poz);
	this->metaIgrac = igrac;
	
	scene::ITriangleSelector* selektor = menadzer->createTriangleSelector(mes, this->model);
	if (selektor)
	{
		metaSelektor->addTriangleSelector(selektor);
		selektor->drop();
	}

	scene::ISceneNodeAnimator* animator = menadzer->createCollisionResponseAnimator(
		metaSelektor,
		this->model,
		core::vector3df(1.0f, 1.0f, 1.0f),
		core::vector3df(0.0f, -2.0f, 0.0f)
	);
	this->model->addAnimator(animator);
	animator->drop();

	this->brzina = 10.0f * (1 + this->sef) + 4.0f * Unos::Instanca().FaktorTezine() +
		1.0f * (scena->Tip() == TipScene::KABINA);
}

void Neprijatelj::SetBarikadaZaobidjena(bool zaobidjeno) { this->barikadaZaobidjena = zaobidjeno; }

bool Neprijatelj::BarikadaZaobidjena() const { return this->barikadaZaobidjena; }

void Neprijatelj::SetScena(Scena* tip) { scena = tip; }

Barikada* Neprijatelj::Meta() const { return this->metaBarikada; }

void Neprijatelj::SetPozicija(const core::vector3df& pozicija) { this->model->setPosition(pozicija); }

const core::vector3df& Neprijatelj::Pozicija() const { return this->model->getPosition(); }

void Neprijatelj::SetVelicina(float skalar) { this->model->setScale(core::vector3df(skalar)); }

void Neprijatelj::SetJeSef(bool jeSef)
{
	this->model->setMaterialTexture(0, menadzer->getVideoDriver()->getTexture("neprijatelj/archpain3.png"));
	this->model->removeAnimators();
	scene::ISceneNodeAnimator* animator = menadzer->createCollisionResponseAnimator(
		metaSelektor,
		this->model,
		core::vector3df(5.0f, 5.0f, 5.0f),
		core::vector3df(0.0f, -2.0f, 0.0f)
	);
	this->model->addAnimator(animator);
	animator->drop();
	this->sef = jeSef;
	pz.Pusti2DZvuk(SvrhaZvuka::MUZIKA, 6, true);
}

bool Neprijatelj::JeSef() { return this->sef; }

void Neprijatelj::SetZiv(bool jeZiv) { this->ziv = jeZiv; }

bool Neprijatelj::JeZiv() const { return this->ziv; }

void Neprijatelj::Napadni(Barikada* barikada)
{
	if (this->ziv)
	{
		this->setPosition(core::vector3df(999.0f));
		return;
	}

	if (barikada->Zivot() < 0)
	{
		barikada->UnistiSe();
		this->model->setMD2Animation(scene::EMAT_RUN);
		napada = false;
		trci = true;
		return;
	}

	if (!napada)
	{
		this->model->setMD2Animation(scene::EMAT_ATTACK);
		trci = false;
		napada = true;
	}
	barikada->OduzmiDeoZivota();
}

void Neprijatelj::Napadni(Igrac* igrac)
{
	if (scena->JeIgracMrtav())
		return;

	int zivotIgraca = igrac->Zivot();
	if (zivotIgraca < 0)
	{
		scene::ICameraSceneNode* kameraIgraca = igrac->Kamera();
		kameraIgraca->setInputReceiverEnabled(false);
		static bool zvukSePusta = false;
		if (!zvukSePusta)
		{
			pz.Pusti2DZvuk(SvrhaZvuka::BULI_MAGVAJER, 9);
			zvukSePusta = true;
		}
		
		if (!pz.ZvukSePusta(SvrhaZvuka::BULI_MAGVAJER, 9))
		{
			scena->SetIgracMrtav(true);
			zvukSePusta = false;
			kameraIgraca->removeAnimators();
		}
		
		return;
	}

	this->model->setMD2Animation(scene::EMAT_ATTACK);
	igrac->SetZivot(zivotIgraca -= 10);
}

void Neprijatelj::IdiKaBarikadi(float deltaVreme)
{
	core::vector3df pozicijaBarikade = this->metaBarikada->Pozicija();
	core::vector3df pozicija = this->Pozicija();
	core::vector3df trajekcija = (pozicijaBarikade - pozicija).normalize() *
		this->brzina * deltaVreme * !Scena::pauzirano;

	this->SetPozicija(pozicija += trajekcija);

	if (pozicija.getDistanceFrom(pozicijaBarikade) < 10.0f)
	{
		this->Napadni(this->metaBarikada);
		this->model->setLoopMode(false);
		return;
	}

	if (!trci)
	{
		this->model->setMD2Animation(scene::EMAT_RUN);
		trci = true;
		napada = false;
	}
}

void Neprijatelj::IdiKaIgracu(float deltaVreme)
{
	core::vector3df pozicijaIgraca = this->metaIgrac->Pozicija();
	core::vector3df pozicija = this->Pozicija();
	core::vector3df trajekcija = (pozicijaIgraca - pozicija).normalize() *
		this->brzina * deltaVreme * !Scena::pauzirano;

	this->SetPozicija(pozicija += trajekcija);

	if (pozicija.getDistanceFrom(pozicijaIgraca) < 10.0f)
		this->Napadni(this->metaIgrac);
}