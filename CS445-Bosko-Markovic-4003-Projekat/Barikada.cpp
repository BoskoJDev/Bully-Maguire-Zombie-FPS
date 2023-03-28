#include "Barikada.h"
#include "PogonZvuka.h"
#include "ID_za_zrak.h"
#include "Igrac.h"
#include <iostream>

using namespace irr;

Barikada::Barikada(scene::ISceneManager* menadzer)
{
	this->model = menadzer->addCubeSceneNode(5.0f, nullptr, ID_ZrakIzbegava);
	this->model->setMaterialTexture(0, menadzer->getVideoDriver()->getTexture("Stripes.jpg"));
	this->model->setMaterialFlag(video::EMF_LIGHTING, false);
	this->model->setMaterialFlag(video::EMF_FOG_ENABLE, true);
	this->model->setScale(core::vector3df(2.5f, 1.0f, 1.0f));
}

void Barikada::OduzmiDeoZivota() { this->zivot -= 1000; }

void Barikada::UnistiSe()
{
	if (this->zvukPusten)
		return;

	PogonZvuka::Instanca().Pusti3DZvuk(7, this->Pozicija());
	this->zvukPusten = true;
	this->model->setVisible(false);
}

void Barikada::PopraviSe()
{
	this->zivot = 100;
	this->model->setVisible(true);
	this->zvukPusten = false;
}

float Barikada::RazdaljinaOdIgraca(Igrac* igrac)
{
	return igrac->Pozicija().getDistanceFrom(this->model->getPosition());
}

const core::vector3df& Barikada::Pozicija() const {	return this->model->getPosition(); }

void Barikada::SetPozicija(const core::vector3df& pozicija) { this->model->setPosition(pozicija); }

void Barikada::SetRotacija(float rotacija)
{
	this->model->setRotation(core::vector3df(0.0f, rotacija, 0.0f));
}