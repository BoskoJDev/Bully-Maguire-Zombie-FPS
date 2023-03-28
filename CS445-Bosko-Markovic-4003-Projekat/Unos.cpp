#include "Unos.h"
#include "ID_za_zrak.h"
#include "PogonZvuka.h"
#include <iostream>
#include <random>
#include <memory>
#include <fstream>
#include "Neprijatelj.h"

using namespace irr;

Unos& Unos::Instanca()
{
    static Unos instanca;
    return instanca;
}

void Unos::SetTrenutnaScena(irr::IrrlichtDevice* uredjaj, TipScene tipScene)
{
    if (this->trenutnaScena)
    {
        this->trenutnaScena = std::move(std::make_unique<Scena>(uredjaj, tipScene));
        return;
    }
    
    this->trenutnaScena = std::make_unique<Scena>(uredjaj, tipScene);
}

Scena* Unos::GetTrenutnaScena() const { return this->trenutnaScena.get(); }

bool Unos::TasterPritisnut(irr::EKEY_CODE taster) const { return this->tasteri[taster]; }

void Unos::PovecajTezinu() { this->faktorTezine += 0.7f; }

float Unos::FaktorTezine() const { return this->faktorTezine; }

bool Unos::OnEvent(const irr::SEvent& dogadjaj)
{
    static bool tekstDodat = false;

    Igrac* igrac = this->trenutnaScena->LikIgraca();
    static Oruzje(*nasumicnoOruzje)(Igrac* igrac) = [](Igrac* igrac)
    {
        std::random_device uredjaj;
        std::mt19937 generator(uredjaj());
        std::uniform_int_distribution<std::mt19937::result_type> distribucija(0, 4);

        Oruzje izvuceno = Oruzje(distribucija(generator));
        return igrac->TrenutnoOruzje() == izvuceno ? nasumicnoOruzje(igrac) : izvuceno;
    };

    PogonZvuka& pz = PogonZvuka::Instanca();
    SEvent::SKeyInput tastatura = dogadjaj.KeyInput;
    SEvent::SMouseInput mis = dogadjaj.MouseInput;
    IrrlichtDevice* uredjaj = this->trenutnaScena->Uredjaj();
    scene::ISceneManager* sm = this->trenutnaScena->MenadzerScene();
    gui::IGUIEnvironment* gui = this->trenutnaScena->GUI();

    const core::dimension2du& ekran = sm->getVideoDriver()->getScreenSize();
    const core::vector2di& polaEkrana = { (s32)ekran.Width / 2, (s32)ekran.Height / 2 };
    switch (dogadjaj.EventType)
    {
        case EET_KEY_INPUT_EVENT:
            {
                this->tasteri[tastatura.Key] = tastatura.PressedDown;

                if (this->tasteri[KEY_ESCAPE])
                    Scena::pauzirano = !Scena::pauzirano;

                if (Scena::pauzirano)
                    return false;

                if (this->tasteri[KEY_KEY_R])
                    igrac->NapuniOruzje();

                if (!this->tasteri[KEY_KEY_F])
                    break;

                if (this->trenutnaScena->IgracBlizuKutije() && igrac->Poeni() >= 60)
                    igrac->SetOruzje(nasumicnoOruzje(igrac));

                if (this->trenutnaScena->Tip() == TipScene::KABINA)
                    return false;

                int najblize = this->trenutnaScena->NajblizaBarikada();
                if (najblize != -1)
                    this->trenutnaScena->IgracPopravljaBarikadu(najblize);
            }
            break;
        case EET_MOUSE_INPUT_EVENT:
            {
                if (mis.Event == EMIE_MOUSE_MOVED)
                    return false;

                if (Scena::pauzirano)
                    return false;

                TipScene ts = this->trenutnaScena->Tip();
                if (ts == TipScene::HIGHSCORE_MENI || ts == TipScene::POCETNI_MENI)
                    break;

                this->tasteri[KEY_LBUTTON] = mis.isLeftPressed();

                Oruzje trenutnoOruzje = igrac->TrenutnoOruzje();
                if (trenutnoOruzje != Oruzje::MITRALJEZ && mis.Event == EMIE_LMOUSE_PRESSED_DOWN)
                    igrac->Pucaj(this->trenutnaScena->Moc());

                if (!this->tasteri[KEY_LBUTTON])
                    break;

                if (igrac->MagacinPrazan())
                    break;

                scene::ICameraSceneNode* kamera = igrac->Kamera();
                scene::ISceneCollisionManager* ms = sm->getSceneCollisionManager();

                core::line3df zrak = ms->getRayFromScreenCoordinates(polaEkrana, kamera);
                if (Neprijatelj* cvor = (Neprijatelj*)ms->getSceneNodeFromRayBB(zrak, ID_ZrakPogadja))
                {
                    scene::ESCENE_NODE_TYPE tip = cvor->getType();
                    bool uslov = tip != scene::ESNT_LIGHT && tip != scene::ESNT_DUMMY_TRANSFORMATION;
                    if (float enemyHP = cvor->Zivot(); cvor->JeZiv() && uslov)
                    {
                        switch (trenutnoOruzje)
                        {
                            using enum Oruzje;

                            case PISTOLJ: cvor->SetZivot(enemyHP - 20.0f / this->faktorTezine);
                                break;
                            case SACMARA: cvor->SetZivot(enemyHP - 60.0f / this->faktorTezine);
                                break;
                            case PUSKA: cvor->SetZivot(enemyHP - 50.0f / this->faktorTezine);
                                break;
                            case KIRIJA_SMRTI: cvor->SetZivot(enemyHP - 100.0f);
                                break;
                        }

                        bool neprijateljJeSef = cvor->JeSef();
                        if (cvor->Zivot() <= -100.0f - (100.0f * neprijateljJeSef))
                        {
                            cvor->setPosition(core::vector3df(-9999999999999999999999.0f));
                            cvor->removeAnimators();
                            cvor->SetZiv(false);
                            this->trenutnaScena->PovecajBrojUbijenihZombija();
                            this->trenutnaScena->DodajPoene(10 + 10 * neprijateljJeSef);

                            static auto nasumicanZvuk = []() {
                                std::random_device uredjaj;
                                std::mt19937 generator(uredjaj());
                                std::uniform_int_distribution<std::mt19937::result_type> d(0, 2);

                                return d(generator);
                            };

                            if (nasumicanZvuk() == 0)
                                igrac->IzgovoriNesto();

                            static auto nasumicnaMoc = []() {
                                std::random_device uredjaj;
                                std::mt19937 generator(uredjaj());
                                std::uniform_int_distribution<std::mt19937::result_type> d(0, 7);

                                return d(generator);
                            };

                            uint32_t powerup = nasumicnaMoc();
                            if (this->trenutnaScena->TrenutniTalas() > 2 && powerup > 5)
                                this->trenutnaScena->SetMoc(TipMoci(powerup));
                        }
                    }
                }
            }
            break;
        case EET_GUI_EVENT:
            {
                if (dogadjaj.GUIEvent.EventType != gui::EGET_BUTTON_CLICKED)
                    break;

                TipScene tipPrethodneScene = TipScene::KORIDOR;
                core::recti prozor = gui->getVideoDriver()->getViewPort();
                core::vector2di sredinaProzora = { prozor.getWidth() / 2, prozor.getHeight() / 2 };
                switch (dogadjaj.GUIEvent.Caller->getID())
                {
                    case 0: // START
                        {                            
                            if (!this->trenutnaScena->UputstvoProcitano())
                            {
                                if (!tekstDodat)
                                {
                                    gui->clear();

                                    gui->addButton(core::recti(sredinaProzora.X - 100, sredinaProzora.Y,
                                        sredinaProzora.X + 100, sredinaProzora.Y + 60), nullptr, 0, L"START");
                                    gui->addButton(core::recti(sredinaProzora.X - 100, sredinaProzora.Y + 100,
                                        sredinaProzora.X + 100, sredinaProzora.Y + 160), nullptr, 1, L"INSTRUCTIONS");
                                    gui->addButton(core::recti(sredinaProzora.X - 100, sredinaProzora.Y + 200,
                                        sredinaProzora.X + 100, sredinaProzora.Y + 260), nullptr, 2, L"EXIT");
                                    gui->addStaticText(L"YOU WILL PLAY THE GAME WHEN YOU READ THE DAMN INSTRUCTIONS!",
                                        core::recti(sredinaProzora.X + 200, sredinaProzora.Y,
                                            sredinaProzora.X + 400, sredinaProzora.Y + 200));

                                    tekstDodat = true;
                                }

                                break;
                            }

                            Scena::uPocetnojFormi = false;
                            gui->clear();

                            core::stringw strw;
                            strw += L"\n\n\n\n\n\n\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
                            strw += L"PICK A MAP";
                            gui->addStaticText(strw.c_str(),
                                core::recti(0, 0, sredinaProzora.X * 2, sredinaProzora.Y * 2));

                            gui->addButton(core::recti(sredinaProzora.X - 200, sredinaProzora.Y,
                                sredinaProzora.X - 100, sredinaProzora.Y + 60), nullptr, 4,
                                L"CORRIDOR", L"Abandoned corridor in the middle of knowhere");
                            gui->addButton(core::recti(sredinaProzora.X - 50, sredinaProzora.Y,
                                sredinaProzora.X + 50, sredinaProzora.Y + 60), nullptr, 5,
                                L"CABIN", L"Medium size cabin in the woods");
                        }
                        break;
                    case 1: // INSTRUCTIONS
                        {
                            Scena::uPocetnojFormi = false;
                            this->trenutnaScena->SetUputstvoProcitano();
                            gui->clear();

                            core::stringw uputstvo;
                            uputstvo += L"\n\n\n\n\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
                            uputstvo += L"CONTROLS\n\n\n";
                            uputstvo += L"\t\t\t\t\t\t\t\t\t\t\t\tW - forward\n\n";
                            uputstvo += L"\t\t\t\t\t\t\t\t\t\t\t\tS - backward\n\n";
                            uputstvo += L"\t\t\t\t\t\t\t\t\t\t\t\tA - left\n\n";
                            uputstvo += L"\t\t\t\t\t\t\t\t\t\t\t\tD - right\n\n";
                            uputstvo += L"\t\t\t\t\t\t\t\t\t\t\t\tLeft mouse - fire\n\n";
                            uputstvo += L"\t\t\t\t\t\t\t\t\t\t\t\tR - reload\n\n";
                            gui->addStaticText(
                                uputstvo.c_str(),
                                core::recti(0, 0, sredinaProzora.X * 2, sredinaProzora.Y * 2)
                            );
                            gui->addButton(core::recti(sredinaProzora.X - 100, sredinaProzora.Y + 200,
                                sredinaProzora.X + 100, sredinaProzora.Y + 260), nullptr, 3, L"BACK");
                        }
                        break;
                    case 2: // EXIT
                        {
                            uredjaj->closeDevice();
                        }
                        break;
                    case 3: // BACK
                        {
                            Scena::uPocetnojFormi = true;

                            gui->clear();
                            gui->addButton(core::recti(sredinaProzora.X - 100, sredinaProzora.Y,
                                sredinaProzora.X + 100, sredinaProzora.Y + 60), nullptr, 0, L"START");
                            gui->addButton(core::recti(sredinaProzora.X - 100, sredinaProzora.Y + 100,
                                sredinaProzora.X + 100, sredinaProzora.Y + 160), nullptr, 1, L"INSTRUCTIONS");
                            gui->addButton(core::recti(sredinaProzora.X - 100, sredinaProzora.Y + 200,
                                sredinaProzora.X + 100, sredinaProzora.Y + 260), nullptr, 2, L"EXIT");
                        }
                        break;
                    case 4: // CORRIDOR
                        {
                            gui->clear();
                            tipPrethodneScene = TipScene::KORIDOR;
                            this->SetTrenutnaScena(uredjaj, TipScene::KORIDOR);
                            pz.StopirajZvuk(SvrhaZvuka::MUZIKA, 0);
                        }
                        break;
                    case 5: // CABIN
                        {
                            gui->clear();
                            tipPrethodneScene = TipScene::KABINA;
                            this->SetTrenutnaScena(uredjaj, TipScene::KABINA);
                            pz.StopirajZvuk(SvrhaZvuka::MUZIKA, 0);
                        }
                        break;
                    case 7: // TRY AGAIN
                        {
                            gui->clear();
                            this->SetTrenutnaScena(uredjaj, tipPrethodneScene);
                            if (pz.ZvukSePusta(SvrhaZvuka::MUZIKA, 2))
                                pz.StopirajZvuk(SvrhaZvuka::MUZIKA, 2);
                        }
                        break;
                    case 8: // MENU
                        {
                            Scena::uPocetnojFormi = true;
                            Scena::pauzirano = false;
                            tekstDodat = false;

                            gui->clear();
                            pz.StopirajZvuk(SvrhaZvuka::MUZIKA, 2);
                            this->SetTrenutnaScena(uredjaj, TipScene::POCETNI_MENI);
                        }
                        break;
                }
            }
            break;
    }  

    return false;
}