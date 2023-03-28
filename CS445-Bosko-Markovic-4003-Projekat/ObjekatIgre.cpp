#include "ObjekatIgre.h"

irr::scene::IMetaTriangleSelector* ObjekatIgre::metaSelektor = nullptr;
irr::scene::ISceneNode* ObjekatIgre::glavniCvor = nullptr;

void ObjekatIgre::SetMetaSelektor(irr::scene::IMetaTriangleSelector* meta)
{
	metaSelektor = meta;
}

irr::scene::IMetaTriangleSelector* ObjekatIgre::MetaSelektor() { return metaSelektor; }

void ObjekatIgre::SetZivot(float zivot) { this->zivot = zivot; }

float ObjekatIgre::Zivot() const { return this->zivot; }