#include "Scena.h"
#include "Barikada.h"
#include "PogonZvuka.h"
#include "Unos.h"
#include "ID_za_zrak.h"
#include "Neprijatelj.h"
#include <random>
#include <iostream>
#include <memory>
#include <execution>
#include <fstream>

#define MITRALJEZOVA_BRZINA_VATRE 1.0f / 45.0f

using namespace irr;

namespace
{
	float deltaVreme = 0.0f;
	float prethodnoVreme = 0.0f;
	core::recti prozor;
	core::vector2di sredinaProzora;
	gui::IGUIFont* font;
	gui::IGUIFont* fontTalas;
	Unos& unos = Unos::Instanca();
	PogonZvuka& pz = PogonZvuka::Instanca();
	float pauzaIzmedjuKreiranja = 1.5f;
	float pauzaIzmedjuRundi = 20.0f;

	struct AnimiraniMagvajer
	{
		video::ITexture* sprajt = nullptr;
		int redoviAnimacije = 4;
		int koloneAnimacije = 5;
		int brzinaAnimacije = 150;
		float sirinaFrejma = 0.0f;
		float visinaFrejma = 0.0f;
		core::recti koordinateTeksture;
		core::recti pozicijaSprajta;
	};
	AnimiraniMagvajer bmgif;

	gui::IGUIButton* resumeDugme;
	gui::IGUIButton* menuDugme;
}

bool Scena::pauzirano = false;
bool Scena::uPocetnojFormi = true;

Scena::Scena(IrrlichtDevice* uredjaj, TipScene tip)
{
	this->tipScene = tip;
	this->brojZombijaPoTalasu = 5 + 3 * (this->tipScene == TipScene::KABINA);

	this->uredjaj = uredjaj;
	this->drajver = this->uredjaj->getVideoDriver();
	this->gui = this->uredjaj->getGUIEnvironment();
	this->menadzerScene = this->uredjaj->getSceneManager();

	font = this->gui->getFont("fontcourier.bmp");
	font->setKerningWidth(3);
	font->setKerningHeight(5);
	this->gui->getSkin()->setFont(font);
	
	fontTalas = this->gui->getFont("fonthaettenschweiler.bmp");
	fontTalas->setKerningWidth(6);
	fontTalas->setKerningHeight(9);

	this->glavniCvor = this->menadzerScene->getRootSceneNode();

	this->zombiji.reserve(brojZombijaPoTalasu);

	if (this->tipScene == TipScene::POCETNI_MENI || this->tipScene == TipScene::HIGHSCORE_MENI)
	{
		prozor = this->drajver->getViewPort();
		sredinaProzora = core::vector2di(prozor.getWidth() / 2, prozor.getHeight() / 2);
		if (this->tipScene == TipScene::POCETNI_MENI)
		{
			pz.Pusti2DZvuk(SvrhaZvuka::MUZIKA, 0, true);

			this->gui->addButton(core::recti(sredinaProzora.X - 100, sredinaProzora.Y,
				sredinaProzora.X + 100, sredinaProzora.Y + 60), nullptr, 0, L"START");
			this->gui->addButton(core::recti(sredinaProzora.X - 100, sredinaProzora.Y + 100,
				sredinaProzora.X + 100, sredinaProzora.Y + 160), nullptr, 1, L"INSTRUCTIONS");
			this->gui->addButton(core::recti(sredinaProzora.X - 100, sredinaProzora.Y + 200,
				sredinaProzora.X + 100, sredinaProzora.Y + 260), nullptr, 2, L"EXIT");
		}
		else
		{
			core::stringw hs = L"Previous highscore: ";
			if (std::ifstream fajl("highscore.sup", std::ios::binary); fajl.is_open())
			{
				fajl >> this->prethodniHS;
				hs += this->prethodniHS;
				fajl.close();
			}

			hs += L" round";
			this->gui->addStaticText((hs + (this->prethodniHS == 1 ? L"" : L"s")).c_str(),
				core::recti(sredinaProzora.X + 50, sredinaProzora.Y - 100,
				sredinaProzora.X + 200, sredinaProzora.Y + 160));

			this->gui->addButton(core::recti(sredinaProzora.X - 100, sredinaProzora.Y + 100,
				sredinaProzora.X + 100, sredinaProzora.Y + 160), nullptr, 7, L"TRY AGAIN");
			this->gui->addButton(core::recti(sredinaProzora.X - 100, sredinaProzora.Y + 200,
				sredinaProzora.X + 100, sredinaProzora.Y + 260), nullptr, 8, L"MENU");

			if (this->talas > this->prethodniHS)
			{
				if (!pz.ZvukSePusta(SvrhaZvuka::MUZIKA, 2))
					pz.Pusti2DZvuk(SvrhaZvuka::MUZIKA, 2);

				bmgif.sprajt = this->drajver->getTexture("sprajtovi/bully maguire gif.png");
				core::dimension2du velicina = bmgif.sprajt->getSize();
				bmgif.sirinaFrejma = velicina.Width / float(bmgif.koloneAnimacije);
				bmgif.visinaFrejma = velicina.Height / float(bmgif.redoviAnimacije);
				bmgif.pozicijaSprajta = core::recti(100 - bmgif.sirinaFrejma / 2,
					200 - bmgif.visinaFrejma / 2, 100 + bmgif.sirinaFrejma / 2,
					200 + bmgif.visinaFrejma / 2);
				bmgif.koordinateTeksture = core::recti(0, 0, bmgif.sirinaFrejma, bmgif.visinaFrejma);

				std::ofstream fajl("highscore.sup", std::ios::binary);
				fajl << talas;
				fajl.close();
			}

			this->talas = 1;
			this->igracUmro = false;
			this->uredjaj->getCursorControl()->setVisible(true);
			if (pz.ZvukSePusta(SvrhaZvuka::MUZIKA, 1))
				pz.StopirajZvuk(SvrhaZvuka::MUZIKA, 1);
		}

		return;
	}

	if (core::array<core::vector3df> pozicijeStvaranja; this->tipScene == TipScene::KORIDOR)
	{
		this->menadzerScene->loadScene("mape/koridor.irr");
		pozicijeStvaranja.push_back(core::vector3df(40.0f, -10.0f, -100.0f));
		pozicijeStvaranja.push_back(core::vector3df(0.0f, -10.0f, -100.0f));
		pozicijeStvaranja.push_back(core::vector3df(40.0f, -10.0f, -100.0f));
		this->mapaTackiStvaranja.insert(0, pozicijeStvaranja);
		pozicijeStvaranja.clear();

		pozicijeStvaranja.push_back(core::vector3df(100.0f, 0.0f, 70.0f));
		pozicijeStvaranja.push_back(core::vector3df(100.0f, 0.0f, 100.0f));
		pozicijeStvaranja.push_back(core::vector3df(100.0f, 0.0f, 130.0f));
		this->mapaTackiStvaranja.insert(1, pozicijeStvaranja);
		pozicijeStvaranja.clear();

		pozicijeStvaranja.push_back(core::vector3df(0.0f, 0.0f, 200.0f));
		pozicijeStvaranja.push_back(core::vector3df(-60.0f, 0.0f, 200.0f));
		pozicijeStvaranja.push_back(core::vector3df(-110.0f, 0.0f, 200.0f));
		this->mapaTackiStvaranja.insert(2, pozicijeStvaranja);
		pozicijeStvaranja.clear();
	}
	else if (this->tipScene == TipScene::KABINA)
	{
		this->menadzerScene->loadScene("mape/kabina.irr");
		pozicijeStvaranja.push_back(core::vector3df(-1.43542f, 8.14201f, 626.534f));
		pozicijeStvaranja.push_back(core::vector3df(-463.76, 10.9726f, 547.608f));
		pozicijeStvaranja.push_back(core::vector3df(426.083f, 9.00677f, 607.906f));
		this->mapaTackiStvaranja.insert(0, pozicijeStvaranja);
		pozicijeStvaranja.clear();

		pozicijeStvaranja.push_back(core::vector3df(-727.399, 9.31705, 281.775));
		pozicijeStvaranja.push_back(core::vector3df(-741.677f, 9.82728f, -141.044f));
		pozicijeStvaranja.push_back(core::vector3df(-743.067f, 8.96476f, -550.499f));
		this->mapaTackiStvaranja.insert(1, pozicijeStvaranja);
		pozicijeStvaranja.clear();

		pozicijeStvaranja.push_back(core::vector3df(-423.904f, 6.65966f, -713.945f));
		pozicijeStvaranja.push_back(core::vector3df(-734.173f, 9.43921f, -136.386f));
		pozicijeStvaranja.push_back(core::vector3df(-729.562f, 10.3553f, -558.835f));
		this->mapaTackiStvaranja.insert(2, pozicijeStvaranja);
		pozicijeStvaranja.clear();
	}

	ObjekatIgre::SetMetaSelektor(this->menadzerScene->createMetaTriangleSelector());

	this->kutija = this->menadzerScene->addMeshSceneNode(
		this->menadzerScene->getMesh("kutija/Weapon box.obj"), this->glavniCvor, ID_ZrakIzbegava
	);
	this->kutija->setName("misterija");
	this->kutija->setScale(core::vector3df(10.0f));
	this->kutija->setMaterialFlag(video::EMF_LIGHTING, false);
	this->kutija->setMaterialTexture(0, drajver->getTexture("kutija/Weapon box_Albedo.tga"));
	this->kutija->setMaterialTexture(1, drajver->getTexture("kutija/Weapon box_Metallic.tga"));
	this->kutija->setMaterialTexture(2, drajver->getTexture("kutija/Weapon box_Normal.tga"));
	this->kutija->setMaterialTexture(3, drajver->getTexture("kutija/Weapon box_Roughness.tga"));

	if (this->tipScene == TipScene::KORIDOR)
	{
		this->kutija->setPosition(core::vector3df(0.0f, -10.0f, 130.0f));

		this->barikade.reserve(3);
		for (int i = 0; i < 3; i++)
			this->barikade.emplace_back(this->menadzerScene);

		this->barikade[0].SetPozicija(core::vector3df(0.0f, -10.0f, 45.0f));

		this->barikade[1].SetPozicija(core::vector3df(60.0f, -10.0f, 97.0f));
		this->barikade[1].SetRotacija(90.0f);

		this->barikade[2].SetPozicija(core::vector3df(-61.0f, -10.0f, 140.0f));
	}
	else if (this->tipScene == TipScene::KABINA)
	{
		this->kutija->setPosition(core::vector3df(-100.0f, 3.0f, -60.0f));
		this->kutija->setRotation(core::vector3df(0.0f, 90.0f, 0.0f));
	}

	Neprijatelj::SetScena(this);

	this->menadzerScene->getSceneNodesFromType(scene::ESNT_MESH, this->cvorovi);
	for (u32 i = 0; i < this->cvorovi.size(); i++)
	{
		scene::ISceneNode* cvor = this->cvorovi[i];
		cvor->setParent(this->glavniCvor);
		cvor->setID(ID_ZrakIzbegava);
		cvor->setMaterialFlag(video::EMF_LIGHTING, this->tipScene != TipScene::KORIDOR);
		cvor->setMaterialFlag(video::EMF_FOG_ENABLE, this->tipScene == TipScene::KORIDOR);
		scene::ITriangleSelector* selektor = this->menadzerScene->createTriangleSelector(
			((scene::IMeshSceneNode*)cvor)->getMesh(), cvor
		);
		if (selektor)
		{
			ObjekatIgre::MetaSelektor()->addTriangleSelector(selektor);
			selektor->drop();
		}
	}

	this->igrac = std::make_unique<Igrac>(this->menadzerScene);
	this->igrac->SetScena(this);

	this->meta.first = this->drajver->getTexture("sprajtovi/meta.png");
	this->meta.second = meta.first->getSize();

	prethodnoVreme = this->uredjaj->getTimer()->getTime();

	menuDugme = this->gui->addButton(core::recti(sredinaProzora.X - 100, sredinaProzora.Y + 200,
		sredinaProzora.X + 100, sredinaProzora.Y + 300), nullptr, 8, L"MENU", L"Quit carnage");

	if (this->tipScene == TipScene::KABINA)
	{
		this->lampa = this->menadzerScene->addLightSceneNode(this->igrac->Kamera(),
			core::vector3df(0.0f), video::SColorf(1.0f, 1.0f, 1.0f), 100.0f, ID_ZrakIzbegava);

		video::SLight podaci;
		podaci.AmbientColor = video::SColorf(0.5f, 0.5f, 0.5f);
		podaci.CastShadows = true;
		podaci.DiffuseColor = video::SColorf(1.0f, 1.0f, 1.0f);
		podaci.OuterCone = 25.0f;
		podaci.Type = video::ELT_SPOT;
		this->lampa->setLightData(podaci);
		return;
	}

	this->drajver->setFog(video::SColor(0, 80, 80, 80), video::EFT_FOG_LINEAR, 10.0f, 50.0f,
		1.0f, true, false);
}

Scena::~Scena()
{
	this->zombiji.clear();
	this->barikade.clear();

	for (u32 i = 0; i < this->cvorovi.size(); i++)
		this->cvorovi[i]->setPosition(core::vector3df(9999.0f));

	this->cvorovi.clear();
}

void Scena::Azuriraj()
{
	if (this->igracUmro)
	{
		pauzaIzmedjuKreiranja = 1.5f;
		pauzaIzmedjuRundi = 20.0f;
		pauzirano = false;
		unos.SetTrenutnaScena(this->uredjaj, TipScene::HIGHSCORE_MENI);
		return;
	}

	float trenutnoVreme = this->uredjaj->getTimer()->getTime();
	deltaVreme = (trenutnoVreme - prethodnoVreme) / 1000;
	prethodnoVreme = trenutnoVreme;

	this->drajver->beginScene(true, true, irr::video::SColor(255));
	{
		switch (this->tipScene)
		{
			using enum TipScene;

			case POCETNI_MENI:
				{
					if (!uPocetnojFormi)
						break;

					fontTalas->draw(
						L"Bully Maguire bullies archviles\n\t\t\t\t\t\t\t\t(or otherwise?)",
						core::recti(-500, 100, 1400, 200), video::SColor(255, 255, 150, 0),
						true, true);
				}
				break;
			case HIGHSCORE_MENI:
				{
					if (talas < prethodniHS)
						break;

					static int trenutniRed = 0;
					static int trenutnaKolona = 0;
					static float prethodnoVremeMenjanja = 0.0f;

					deltaVreme = this->uredjaj->getTimer()->getTime() - prethodnoVremeMenjanja;
					if (deltaVreme > bmgif.brzinaAnimacije)
					{
						prethodnoVremeMenjanja = this->uredjaj->getTimer()->getTime();
						trenutnaKolona++;
					}

					if (trenutnaKolona > bmgif.koloneAnimacije)
					{
						trenutnaKolona = 0;
						trenutniRed++;
						if (trenutniRed > bmgif.redoviAnimacije)
							trenutniRed = 0;
					}

					this->drajver->draw2DImage(bmgif.sprajt, bmgif.pozicijaSprajta,
						bmgif.koordinateTeksture + core::vector2di(
							bmgif.sirinaFrejma * trenutniRed, bmgif.visinaFrejma * trenutnaKolona
						));
				}
				break;
			case KORIDOR:
			case KABINA:
				{
					pz.PauzirajSve(pauzirano);
					this->uredjaj->getCursorControl()->setVisible(pauzirano);
					menuDugme->setVisible(pauzirano);
					this->igrac->Kamera()->setInputReceiverEnabled(!pauzirano);
					this->LogikaMapa();
				}
				break;
		}

		this->gui->drawAll();
	}
	this->drajver->endScene();
}

bool Scena::IgracBlizuKutije()
{
	return this->igrac->Pozicija().getDistanceFrom(this->kutija->getPosition()) < 10.0f;
}

int Scena::NajblizaBarikada()
{
	for (int i = 0; i < 3; i++)
	{
		if (this->barikade[i].RazdaljinaOdIgraca(this->igrac.get()) < 10.0f)
			return i;
	}

	return -1;
}

void Scena::IgracPopravljaBarikadu(int najblizaBarikada)
{
	if (this->barikade[najblizaBarikada].Zivot() == 100)
		return;

	pz.Pusti2DZvuk(SvrhaZvuka::MUZIKA, 3);
	this->barikade[najblizaBarikada].PopraviSe();
	this->poeni += 10;
}

void Scena::SetUputstvoProcitano()
{
	std::ofstream fajl("procitano_uputstvo.sup", std::ios::binary);
	fajl << true;
	fajl.close();
}

bool Scena::UputstvoProcitano() const
{
	bool uputstvoProcitano;
	std::ifstream fajl("procitano_uputstvo.sup", std::ios::binary);
	if (!fajl.is_open())
	{
		fajl.close();
		return false;
	}

	fajl >> uputstvoProcitano;
	fajl.close();
	return uputstvoProcitano;
}

IrrlichtDevice* Scena::Uredjaj() const { return this->uredjaj; }

scene::ISceneManager* Scena::MenadzerScene() const { return this->menadzerScene; }

video::IVideoDriver* Scena::VideoDrajver() const { return this->drajver; }

gui::IGUIEnvironment* Scena::GUI() const { return this->gui; }

Igrac* Scena::LikIgraca() const { return this->igrac.get(); }

TipScene Scena::Tip() const { return this->tipScene; }

void Scena::DodajPoene(int poeni) { this->poeni += poeni; }

void Scena::SetIgracMrtav(bool mrtav) { this->igracUmro = mrtav; }

bool Scena::JeIgracMrtav() const { return this->igracUmro; }

void Scena::PovecajBrojUbijenihZombija() { this->brojUbijenihZombija++; }

int Scena::Poeni() const { return this->poeni; }

int Scena::TrenutniTalas() const { return this->talas; }

void Scena::SetMoc(TipMoci moc) { this->tipMoci = moc; }

TipMoci Scena::Moc() const { return this->tipMoci; }

void Scena::Granice()
{
	core::vector3df pozicijaIgraca = this->igrac->Pozicija();
	if (this->tipScene == TipScene::KORIDOR)
	{
		if (pozicijaIgraca.X > 55.0f)
		{
			pozicijaIgraca.X = 55.0f;
			this->igrac->SetPozicija(pozicijaIgraca);
		}
		if (pozicijaIgraca.Z < 50.0f)
		{
			pozicijaIgraca.Z = 50.0f;
			this->igrac->SetPozicija(pozicijaIgraca);
		}
		if (pozicijaIgraca.Z > 134.0f)
		{
			pozicijaIgraca.Z = 134.0f;
			this->igrac->SetPozicija(pozicijaIgraca);
		}
	}
	else if (this->tipScene == TipScene::KABINA)
	{
		if (pozicijaIgraca.X < -745.0f)
		{
			pozicijaIgraca.X = -745.0f;
			this->igrac->SetPozicija(pozicijaIgraca);
		}
		if (pozicijaIgraca.X > 625.0f)
		{
			pozicijaIgraca.X = 625.0f;
			this->igrac->SetPozicija(pozicijaIgraca);
		}
		if (pozicijaIgraca.Z > 625.0f)
		{
			pozicijaIgraca.Z = 625.0f;
			this->igrac->SetPozicija(pozicijaIgraca);
		}
		if (pozicijaIgraca.Z < -745.0f)
		{
			pozicijaIgraca.Z = -745.0f;
			this->igrac->SetPozicija(pozicijaIgraca);
		}
	}
}

void Scena::PokreniTalas()
{
	if (this->talasPokrenut)
		return;

	static auto nasumicnaBarikada = []()
	{
		std::random_device uredjaj;
		std::mt19937 generator(uredjaj());
		std::uniform_int_distribution<std::mt19937::result_type> distribucija(0, 2);

		return distribucija(generator);
	};

	static float vreme = 0.0f;
	vreme += deltaVreme * !pauzirano;
	if (vreme > pauzaIzmedjuKreiranja && kreiraniZombiji < brojZombijaPoTalasu)
	{
		int barikada = nasumicnaBarikada();
		this->zombiji.emplace_back(
			this->menadzerScene,
			this->igrac.get(),
			this->tipScene != TipScene::KABINA ? &this->barikade[barikada] : nullptr,
			this->mapaTackiStvaranja.find(barikada)->getValue()[nasumicnaBarikada()]
		);

		if (this->tipScene == TipScene::KABINA && !this->sefKreran && this->talas % 3 == 0)
		{
			Neprijatelj& sef = this->zombiji.back();
			sef.SetVelicina(5.0f);
			sef.SetJeSef(true);
			this->sefKreran = true;
		}
		
		vreme = 0.0f;
		this->talasPokrenut = ++this->kreiraniZombiji == this->brojZombijaPoTalasu;
	}
}

void Scena::LogikaMapa()
{
	core::stringw powerUpStr;

	static float trajanjeMoci = 0.0f;
	trajanjeMoci += deltaVreme * (this->tipMoci == TipMoci::NEOGRANICENA_MUNICIJA && !pauzirano);
	if (this->tipMoci == TipMoci::NEOGRANICENA_MUNICIJA)
	{
		powerUpStr = L"Unlimited ammo: ";
		powerUpStr += 10 - int(trajanjeMoci);
		if (trajanjeMoci > 10.0f)
		{
			this->tipMoci = TipMoci::NISTA;
			trajanjeMoci = 0.0f;
		}
	}

	this->Granice();
	this->menadzerScene->drawAll();

	font->draw(powerUpStr, core::recti(sredinaProzora.X - 100, sredinaProzora.Y,
		sredinaProzora.X, sredinaProzora.Y), video::SColor(255, 255, 255, 255));

	if (this->tipMoci == TipMoci::DOPUNA_MUNICIJE)
	{
		this->igrac->DuplirajMuniciju();
		this->tipMoci = TipMoci::NISTA;
	}

	if (this->brojUbijenihZombija == this->brojZombijaPoTalasu)
	{
		static float vreme = 0.0f;
		if (!pz.ZvukSePusta(SvrhaZvuka::MUZIKA, 1))
			pz.Pusti2DZvuk(SvrhaZvuka::MUZIKA, 1);

		pz.StopirajZvuk(SvrhaZvuka::MUZIKA, 6);

		font->draw(core::stringw(L"Next round starts in ") += int(pauzaIzmedjuRundi - vreme),
			core::recti(sredinaProzora.X - 100, sredinaProzora.Y - 200,	sredinaProzora.X,
			sredinaProzora.Y), video::SColor(255, 255, 255, 255), true, true);

		vreme += deltaVreme * !pauzirano;
		if (vreme > pauzaIzmedjuRundi)
		{
			this->brojZombijaPoTalasu += 3;
			this->zombiji.clear();
			this->zombiji.reserve(this->brojZombijaPoTalasu);
			this->brojUbijenihZombija = 0;
			this->kreiraniZombiji = 0;
			unos.PovecajTezinu();
			this->talas++;
			this->sefKreran = false;
			this->talasPokrenut = false;
			vreme = 0.0f;
		}
	}

	this->PokreniTalas();

	Oruzje oruzjeIgraca = this->igrac->TrenutnoOruzje();
	if (unos.TasterPritisnut(KEY_LBUTTON) && oruzjeIgraca == Oruzje::MITRALJEZ)
	{
		static float brzinaPucanja = 0.0f;
		brzinaPucanja += deltaVreme;
		if (brzinaPucanja > MITRALJEZOVA_BRZINA_VATRE)
		{
			this->igrac->Pucaj(this->tipMoci);
			scene::ISceneCollisionManager* ms = this->menadzerScene->getSceneCollisionManager();
			core::line3df zrak = ms->getRayFromScreenCoordinates(
				sredinaProzora, this->igrac->Kamera()
			);
			if (Neprijatelj* cvor = (Neprijatelj*)ms->getSceneNodeFromRayBB(zrak, ID_ZrakPogadja))
			{
				if (cvor->JeZiv() && cvor->getType() != scene::ESNT_LIGHT)
				{
					float enemyHP = cvor->Zivot();
					cvor->SetZivot(enemyHP - (50.0f / unos.FaktorTezine()));
					if (cvor->Zivot() < -100.0f - (100.0f * cvor->JeSef()))
					{
						cvor->setPosition(core::vector3df(-999999999999.0f));
						cvor->removeAnimators();
						cvor->SetZiv(false);
						this->brojUbijenihZombija++;
						this->poeni += 10;
						
						static auto nasumicanZvuk = []() {
							std::random_device uredjaj;
							std::mt19937 generator(uredjaj());
							std::uniform_int_distribution<std::mt19937::result_type> d(0, 1);

							return d(generator);
						};

						if (nasumicanZvuk() == 0)
							this->igrac->IzgovoriNesto();

						static auto nasumicnaMoc = []() {
							std::random_device uredjaj;
							std::mt19937 generator(uredjaj());
							std::uniform_int_distribution<std::mt19937::result_type> d(0, 7);

							return d(generator);
						};

						if (uint32_t powerup = nasumicnaMoc(); powerup > 5)
							this->tipMoci = TipMoci(powerup);
					}
				}
			}
			brzinaPucanja = 0.0f;
		}
	}

	static core::stringw string;
	switch (oruzjeIgraca)
	{
		using enum Oruzje;

		case PISTOLJ: string = L"Beretta M9";
			break;
		case SACMARA: string = L"SPAS-12";
			break;
		case PUSKA: string = L"M16A4 Colt";
			break;
		case MITRALJEZ: string = L"M249 LMG";
			break;
		case KIRIJA_SMRTI: string = L"Rent's wrath";
			break;
	}

	this->drajver->draw2DRectangle(video::SColor(99, 0, 0, 0),
		core::recti(0, 0, string.size() * (10 + 1), 40));
	font->draw(string, core::recti(0, 0, 100, 100), video::SColor(255, 255, 255, 255));
	font->draw(core::stringw(std::to_wstring(this->igrac->Meci()).c_str()) + L" | " +
		core::stringw(std::to_wstring(this->igrac->Municija()).c_str()),
		core::recti(0, 20, 100, 100), video::SColor(255, 255, 255, 255));

	this->drajver->draw2DRectangle(video::SColor(255, 100, 0, 0), core::recti(0, 40, 100, 60));
	font->draw(core::stringw(std::to_wstring(this->igrac->Poeni()).c_str()),
		core::recti(0, 40, 100, 50), video::SColor(255, 255, 255, 255));

	if (this->tipScene != TipScene::KABINA)
	{
		if (int najblizaBarikada = this->NajblizaBarikada(); najblizaBarikada != -1)
		{
			if (this->barikade[najblizaBarikada].Zivot() < 100)
			{
				font->draw(L"Press F to fix the damn barricade",
					core::recti(sredinaProzora.X - 200, sredinaProzora.Y + 100,
						sredinaProzora.X + 300, sredinaProzora.Y + 150),
					video::SColor(255, 255, 255, 255));
			}
		}
	}

	if (this->IgracBlizuKutije() && this->poeni >= 60)
	{
		font->draw(L"Press F for a random weapon", core::recti(sredinaProzora.X - 200,
			sredinaProzora.Y + 100,	sredinaProzora.X + 300, sredinaProzora.Y + 150),
			video::SColor(255, 255, 255, 255));
	}

	this->igrac->SetPoeni(this->poeni);
	std::for_each(this->zombiji.begin(), this->zombiji.end(),
		[this](Neprijatelj& neprijatelj)
		{
			static auto nasumicanZvuk = []()
			{
				std::random_device uredjaj;
				std::mt19937 generator(uredjaj());
				std::uniform_int_distribution<std::mt19937::result_type> distr(0, 7);

				return distr(generator);
			};

			core::vector3df pozicijaNeprijatelja = neprijatelj.Pozicija();
			if (u32 zvuk = nasumicanZvuk(); zvuk != 0 && !pz.BiloKojiZvukNeprijateljaSePusta())
				pz.Pusti3DZvuk(zvuk, pozicijaNeprijatelja);

			Barikada* meta = neprijatelj.Meta();
			if (this->tipScene == TipScene::KABINA || neprijatelj.BarikadaZaobidjena())
			{
				neprijatelj.IdiKaIgracu(deltaVreme);
				return;
			}

			if (meta->Pozicija().getDistanceFrom(pozicijaNeprijatelja) > 5.0f)
				neprijatelj.IdiKaBarikadi(deltaVreme);
			else
				neprijatelj.SetBarikadaZaobidjena(true);

			if (meta->Zivot() < 0.0f)
				meta->UnistiSe();
		});

	this->drajver->draw2DImage(this->meta.first, sredinaProzora - core::vector2di(100, 100),
		core::recti(100, 100, 200, 200), nullptr, video::SColor(255, 255, 255, 255), true);

	font->draw(core::stringw(L"Remaining archviles: ") += (this->brojZombijaPoTalasu -
		this->brojUbijenihZombija), core::recti(0, 110, 150, 120),
		video::SColor(255, 255, 255, 255));

	pz.AzurirajPozicijuOsluskivaca(this->igrac.get());

	font->draw(core::stringw(L"ROUND ") + std::to_wstring(this->talas).c_str(),
		core::recti(0, 90, 100, 100), video::SColor(255, 200, 0, 0));
}