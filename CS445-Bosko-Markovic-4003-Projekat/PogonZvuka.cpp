#include "PogonZvuka.h"
#include "Igrac.h"
#include <sstream>

using namespace irrklang;
using namespace irr;

PogonZvuka::PogonZvuka()
{
	this->pogon = createIrrKlangDevice();

	this->magvajerZvuci.reserve(10);
	this->magvajerZvuci[0] = this->pogon->addSoundSourceFromFile("zvuci/magvajer/bully_maguire_laugh.mp3");
	this->magvajerZvuci[1] = this->pogon->addSoundSourceFromFile("zvuci/magvajer/dirt_in_the_eye.mp3");
	this->magvajerZvuci[2] = this->pogon->addSoundSourceFromFile("zvuci/magvajer/he_despised_you.mp3");
	this->magvajerZvuci[3] = this->pogon->addSoundSourceFromFile("zvuci/magvajer/see_ya_chump.mp3");
	this->magvajerZvuci[4] = this->pogon->addSoundSourceFromFile("zvuci/magvajer/still_got_the_moves.mp3");
	this->magvajerZvuci[5] = this->pogon->addSoundSourceFromFile("zvuci/magvajer/stings_doesnt_it.mp3");
	this->magvajerZvuci[6] = this->pogon->addSoundSourceFromFile("zvuci/magvajer/you_re_trash_brock.mp3");
	this->magvajerZvuci[7] = this->pogon->addSoundSourceFromFile("zvuci/magvajer/you_want_forgiveness.mp3");
	this->magvajerZvuci[8] = this->pogon->addSoundSourceFromFile("zvuci/magvajer/ditkovich.mp3");
	this->magvajerZvuci[9] = this->pogon->addSoundSourceFromFile("zvuci/magvajer/my_back.mp3");

	this->pucnji.reserve(9);
	this->pucnji[0] = this->pogon->addSoundSourceFromFile("zvuci/oruzja/prazno.mp3");
	this->pucnji[1] = this->pogon->addSoundSourceFromFile("zvuci/oruzja/pistolj.mp3");
	this->pucnji[2] = this->pogon->addSoundSourceFromFile("zvuci/oruzja/sacmara.wav");
	this->pucnji[3] = this->pogon->addSoundSourceFromFile("zvuci/oruzja/puska.wav");
	this->pucnji[4] = this->pogon->addSoundSourceFromFile("zvuci/oruzja/mitraljez.wav");
	this->pucnji[5] = this->pogon->addSoundSourceFromFile("zvuci/oruzja/kirija.mp3");
	this->pucnji[6] = this->pogon->addSoundSourceFromFile("zvuci/oruzja/punjenje.wav");
	this->pucnji[7] = this->pogon->addSoundSourceFromFile("zvuci/oruzja/punjenje_kirije.mp3");
	this->pucnji[8] = this->pogon->addSoundSourceFromFile("zvuci/oruzja/dopuna.wav");

	this->muzika.reserve(6);
	this->muzika[0] = this->pogon->addSoundSourceFromFile("zvuci/muzika/meni.mp3");
	this->muzika[1] = this->pogon->addSoundSourceFromFile("zvuci/muzika/nova_runda.mp3");
	this->muzika[2] = this->pogon->addSoundSourceFromFile("zvuci/muzika/novi_highscore.mp3");
	this->muzika[3] = this->pogon->addSoundSourceFromFile("zvuci/muzika/popravka.wav");
	this->muzika[4] = this->pogon->addSoundSourceFromFile("zvuci/smeh.wav");
	this->muzika[5] = this->pogon->addSoundSourceFromFile("zvuci/neverica.mp3");
	this->muzika[6] = this->pogon->addSoundSourceFromFile("zvuci/muzika/Tank.wav");

	this->zvuciNeprijatelja.reserve(8);
	for (int i = 0; i < 7; i++)
	{
		std::stringstream ss;
		ss << "zvuci/sidni/sidni" << i << ".mp3";
		this->zvuciNeprijatelja[i] = this->pogon->addSoundSourceFromFile(ss.str().c_str());
	}
	this->zvuciNeprijatelja[7] = this->pogon->addSoundSourceFromFile("zvuci/unisteno.wav");
}

PogonZvuka::~PogonZvuka()
{
	this->pogon->removeAllSoundSources();
	this->pogon->drop();
}

PogonZvuka& PogonZvuka::Instanca()
{
	static PogonZvuka instanca;
	return instanca;
}

void PogonZvuka::AzurirajPozicijuOsluskivaca(Igrac* igrac)
{
	scene::ICameraSceneNode* kamera = igrac->Kamera();
	core::vector3df pozicijaKamere = kamera->getAbsolutePosition();
	core::vector3df pravacKamere = kamera->getTarget();
	vec3df pozicijaKamereIgraca = { pozicijaKamere.X, pozicijaKamere.Y, pozicijaKamere.Z };
	vec3df pravacKamereIgraca = { pravacKamere.X, pravacKamere.Y, pravacKamere.Z };
	this->pogon->setListenerPosition(pozicijaKamereIgraca, pravacKamereIgraca);
}

void PogonZvuka::Pusti2DZvuk(SvrhaZvuka sz, int zvuk, bool ponavljaj)
{
	switch (sz)
	{
		using enum SvrhaZvuka;

		case BULI_MAGVAJER: this->pogon->play2D(this->magvajerZvuci[zvuk], ponavljaj);
			break;
		case MUZIKA: this->pogon->play2D(this->muzika[zvuk], ponavljaj);
			break;
		case PUCANJ: this->pogon->play2D(this->pucnji[zvuk], ponavljaj);
			break;
	}
}

void PogonZvuka::Pusti3DZvuk(int zvuk, const core::vector3df& pozicija, bool ponavljaj)
{
	vec3d poz = { pozicija.X, pozicija.Y, pozicija.Z };
	ISound* pustenZvuk = this->pogon->play3D(this->zvuciNeprijatelja[zvuk], poz, ponavljaj,
		false, true, true);
	pustenZvuk->setMinDistance(50.0f);
	pustenZvuk->drop();
}

void PogonZvuka::StopirajZvuk(SvrhaZvuka sz, int zvuk)
{
	ISoundSource* izvorZvuka = this->magvajerZvuci[zvuk];
	switch (sz)
	{
		using enum SvrhaZvuka;

		case PUCANJ: izvorZvuka = this->pucnji[zvuk];
			break;
		case MUZIKA: izvorZvuka = this->muzika[zvuk];
			break;
		case CUDOVISTE: izvorZvuka = this->zvuciNeprijatelja[zvuk];
			break;
	}

	this->pogon->stopAllSoundsOfSoundSource(izvorZvuka);
}

void PogonZvuka::PauzirajSve(bool pauziraj) { this->pogon->setAllSoundsPaused(pauziraj); }

bool PogonZvuka::ZvukSePusta(SvrhaZvuka sz, int zvuk)
{
	switch (sz)
	{
		using enum SvrhaZvuka;

		case BULI_MAGVAJER:
			return this->pogon->isCurrentlyPlaying(this->magvajerZvuci[zvuk]);
		case MUZIKA:
			return this->pogon->isCurrentlyPlaying(this->muzika[zvuk]);
		case PUCANJ:
			return this->pogon->isCurrentlyPlaying(this->pucnji[zvuk]);
		case CUDOVISTE:
			return this->pogon->isCurrentlyPlaying(this->zvuciNeprijatelja[zvuk]);
	}
}

bool PogonZvuka::BiloKojiZvukNeprijateljaSePusta()
{
	for (const auto&[zvuk, s] : this->zvuciNeprijatelja)
	{
		if (this->ZvukSePusta(SvrhaZvuka::CUDOVISTE, zvuk))
			return true;
	}

	return false;
}

std::unordered_map<int, irrklang::ISoundSource*> PogonZvuka::MagvajerZvuci() const
{
	return this->magvajerZvuci;
}