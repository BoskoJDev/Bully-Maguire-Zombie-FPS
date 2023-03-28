#pragma once
#include <irrlicht/irrlicht.h>
#include <utility>
#include "Neprijatelj.h"
#include "Igrac.h"
#include <vector>
#include <memory>

class Scena
{
public:
	Scena(irr::IrrlichtDevice* uredjaj, TipScene tip);
	~Scena();

	void Azuriraj();
	void IgracPopravljaBarikadu(int najblizaBarikada);

	bool IgracBlizuKutije();
	int NajblizaBarikada();

	void SetUputstvoProcitano();
	bool UputstvoProcitano() const;

	irr::IrrlichtDevice* Uredjaj() const;
	irr::scene::ISceneManager* MenadzerScene() const;
	irr::video::IVideoDriver* VideoDrajver() const;
	irr::gui::IGUIEnvironment* GUI() const;
	Igrac* LikIgraca() const;
	TipScene Tip() const;

	void DodajPoene(int poeni);
	void SetIgracMrtav(bool mrtav);
	bool JeIgracMrtav() const;
	void PovecajBrojUbijenihZombija();
	int Poeni() const;
	int TrenutniTalas() const;

	void SetMoc(TipMoci moc);
	TipMoci Moc() const;

	static bool pauzirano;
	static bool uPocetnojFormi;

private:
	void Granice();
	void PokreniTalas();
	void LogikaMapa();

	irr::IrrlichtDevice* uredjaj;

	irr::scene::ISceneManager* menadzerScene;
	irr::video::IVideoDriver* drajver;
	irr::gui::IGUIEnvironment* gui;
	irr::scene::ISceneNode* glavniCvor;
	irr::scene::IMetaTriangleSelector* metaSelektor;

	std::unique_ptr<Igrac> igrac;
	std::vector<Barikada> barikade;
	std::vector<Neprijatelj> zombiji;
	irr::core::array<irr::scene::ISceneNode*> cvorovi;
	TipScene tipScene;
	bool uputstvoProcitano = false;
	bool sefKreran = false;
	bool talasPokrenut = false;
	bool igracUmro = false; int brojZombijaPoTalasu;
	int brojUbijenihZombija = 0;
	int poeni = 0;
	int prethodniHS = 0;
	int kreiraniZombiji = 0;
	int talas = 3;
	TipMoci tipMoci = TipMoci::NISTA;
	std::pair<irr::video::ITexture*, irr::core::dimension2du> meta;
	irr::core::map<int, irr::core::array<irr::core::vector3df>> mapaTackiStvaranja;
	irr::scene::IMeshSceneNode* kutija;
	irr::scene::ILightSceneNode* lampa;
};