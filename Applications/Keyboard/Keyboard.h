#pragma once
#include <sextant/Activite/Threads.h>
#include <drivers/Clavier.h>
#include <sextant/Synchronisation/Semaphore/Semaphore.h>

struct KeyboardData {
    // Inputs (Written by Keyboard, Read/Reset by Logic)
    bool wantLeft = false;
    bool wantRight = false;
    bool wantJump = false;

    Semaphore lock;
};

// Thread Keyboard : gère les entrées claviers et renvoie au thread Logic les touches appuyées
class KeyboardThread : public Threads {
    KeyboardData* kbdData;
    Clavier keyboard;
public:
    KeyboardThread(KeyboardData* d);
    void run() override;
};
