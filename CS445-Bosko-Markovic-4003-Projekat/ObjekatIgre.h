#pragma once
#include <IrrLicht/irrlicht.h>

class ObjekatIgre
{
public:
	static void SetMetaSelektor(irr::scene::IMetaTriangleSelector* meta);	
	static irr::scene::IMetaTriangleSelector* MetaSelektor();

	void SetZivot(float zivot);
	virtual void SetPozicija(const irr::core::vector3df& pozicija) = 0;

	float Zivot() const;
	virtual const irr::core::vector3df& Pozicija() const = 0;

protected:
	ObjekatIgre() = default;

	float zivot = 100.0f;
	static irr::scene::ISceneNode* glavniCvor;
	static irr::scene::IMetaTriangleSelector* metaSelektor;
};