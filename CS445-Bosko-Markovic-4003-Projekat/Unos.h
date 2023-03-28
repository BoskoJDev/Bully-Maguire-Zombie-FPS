#pragma once
#include "Scena.h"
#include <memory>

class Unos final : public irr::IEventReceiver
{
public:
	Unos(const Unos&) = delete;
	Unos(Unos&&) = delete;

	static Unos& Instanca();

	void SetTrenutnaScena(irr::IrrlichtDevice* uredjaj, TipScene tipScene);
	Scena* GetTrenutnaScena() const;

	bool TasterPritisnut(irr::EKEY_CODE taster) const;

	void PovecajTezinu();
	float FaktorTezine() const;

private:
	Unos() = default;

	bool OnEvent(const irr::SEvent& dogadjaj) override;

	bool tasteri[irr::KEY_KEY_CODES_COUNT] = { false };

	std::unique_ptr<Scena> trenutnaScena;

	float faktorTezine = 1.0f;
};