#include "Aplikacija.h"

int main()
{
    Aplikacija* app = new Aplikacija(true);
    app->Pokreni();
    delete app;

    return 0;
}