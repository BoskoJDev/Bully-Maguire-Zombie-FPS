#pragma once
#include <irrKlang/irrKlang.h>
#include <IrrLicht/irrlicht.h>
#include <unordered_map>

class Igrac;

enum class SvrhaZvuka : char
{
	BULI_MAGVAJER,
	CUDOVISTE,
	MUZIKA,
	PUCANJ
};

class PogonZvuka final
{
public:
	PogonZvuka(const PogonZvuka&) = delete;
	PogonZvuka(PogonZvuka&&) = delete;
	~PogonZvuka();

	static PogonZvuka& Instanca();

	void AzurirajPozicijuOsluskivaca(Igrac* igrac);

	void Pusti2DZvuk(SvrhaZvuka sz, int zvuk, bool ponavljaj = false);
	void Pusti3DZvuk(int zvuk, const irr::core::vector3df& pozicija, bool ponavljaj = false);

	void StopirajZvuk(SvrhaZvuka sz, int zvuk);

	void PauzirajSve(bool pauziraj);

	bool ZvukSePusta(SvrhaZvuka sz, int zvuk);

	bool BiloKojiZvukNeprijateljaSePusta();

	std::unordered_map<int, irrklang::ISoundSource*> MagvajerZvuci() const;

private:
	PogonZvuka();

	irrklang::ISoundEngine* pogon;
	std::unordered_map<int, irrklang::ISoundSource*> magvajerZvuci;
	std::unordered_map<int, irrklang::ISoundSource*> pucnji;
	std::unordered_map<int, irrklang::ISoundSource*> muzika;
	std::unordered_map<int, irrklang::ISoundSource*> zvuciNeprijatelja;
};