#include "Keyboard.h"
#include <sextant/ordonnancements/preemptif/thread.h>

KeyboardThread::KeyboardThread(KeyboardData* d) {
    kbdData = d;
}

void KeyboardThread::run() {
    while(true) {
        bool left = false, right = false, jump = false;
        // Read all pending characters
        if (keyboard.is_pressed(AZERTY::K_Q)) left = true;
        if (keyboard.is_pressed(AZERTY::K_D)) right = true;
        if (keyboard.is_pressed(AZERTY::K_Z)) jump = true;
        
        if (left || right || jump) {
            kbdData->lock.P();
            if (left) kbdData->wantLeft = true;
            if (right) kbdData->wantRight = true;
            if (jump) kbdData->wantJump = true;
            kbdData->lock.V();
        }
        
        thread_yield();
    }
}
