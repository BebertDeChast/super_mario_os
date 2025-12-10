#include <hal/multiboot.h>
#include <drivers/Ecran.h>
#include <drivers/PortSerie.h>

#include <sextant/interruptions/idt.h>
#include <sextant/interruptions/irq.h>
#include <sextant/interruptions/handler/handler_tic.h>
#include <sextant/interruptions/handler/handler_clavier.h>
#include <drivers/timer.h>
#include <drivers/Clavier.h>

#include <sextant/memoire/memoire.h>

#include <sextant/ordonnancements/cpu_context.h>
#include <sextant/ordonnancements/preemptif/thread.h>
#include <sextant/Activite/Threads.h>
#include <sextant/types.h>

#include <sextant/Synchronisation/Spinlock/Spinlock.h>
#include <sextant/Synchronisation/Semaphore/Semaphore.h>

#include <hal/pci.h>
#include <drivers/vga.h>
#include <drivers/EcranBochs.h>

#include <sextant/sprite.h>
#include <Applications/MarioBros/Movement.h>

#include <Applications/Level/Level.h>
#include <Applications/Level/Level_display_data.h>
#include <hal/fonctionsES.h>

extern char __e_kernel, __b_kernel, __b_data, __e_data, __b_stack, __e_load;
int i;

extern vaddr_t bootstrap_stack_bottom; // Adresse de début de la pile d'exécution
extern size_t bootstrap_stack_size;    // Taille de la pile d'exécution

struct SharedData {
    struct {
        Spinlock spin;
        int val = 0;
        void lock() { spin.Take(&val); }
        void unlock() { spin.Release(&val); }
    } lock;
    
    // Inputs (Written by Keyboard, Read/Reset by Logic)
    bool wantLeft = false;
    bool wantRight = false;
    bool wantJump = false;

    // Game State (Written by Logic, Read by Display)
    int marioX = 32;
    int marioY = 100;
    int scrollX = 0;
    int scrollY = 0;
    bool isRight = true;
};

Semaphore render_next_frame;

// Thread Keyboard : gère les entrées claviers et renvoie au thread Logic les touches appuyées
class KeyboardThread : public Threads {
    SharedData* data;
    Clavier keyboard;
public:
    KeyboardThread(SharedData* d) : data(d) {}
    void run() override {
        while(true) {
            bool left = false, right = false, jump = false;
            // Read all pending characters
            if (keyboard.is_pressed(AZERTY::K_Q)) left = true;
            if (keyboard.is_pressed(AZERTY::K_D)) right = true;
            if (keyboard.is_pressed(AZERTY::K_Z)) jump = true;
            
            if (left || right || jump) {
                data->lock.lock();
                if (left) data->wantLeft = true;
                if (right) data->wantRight = true;
                if (jump) data->wantJump = true;
                data->lock.unlock();
            }
            
            thread_yield();
        }
    }
};

// Thread Logic : gère la logique.
class LogicThread : public Threads {
    SharedData* data;
    int width, height;
public:
    LogicThread(SharedData* d, int w, int h) : data(d), width(w), height(h) {}
    void run() override {
        while(true) {
            data->lock.lock();
            bool wLeft = data->wantLeft;
            bool wRight = data->wantRight;
            bool wJump = data->wantJump;
            
            // Reset inputs for next frame logic
            data->wantLeft = false;
            data->wantRight = false;
            data->wantJump = false;
            
            int mx = data->marioX;
            int my = data->marioY;
            int sx = data->scrollX;
            int sy = data->scrollY;
            bool isRight = data->isRight;
            data->lock.unlock();

            // Run physics
            update_mario_position(mx, my, sx, sy, width, height, isRight, wLeft, wRight, wJump);

            data->lock.lock();
            data->marioX = mx;
            data->marioY = my;
            data->scrollX = sx;
            data->scrollY = sy;
            data->isRight = isRight;
            data->lock.unlock();
            
            thread_yield(); 
        }
    }
};

// Display : gère l'affichage
class DisplayThread : public Threads {
    SharedData* data;
public:
    DisplayThread(SharedData* d) : data(d) {}
    void run() override {
        // Ecran 720x240, mode 8 bits (256 couleurs)
        EcranBochs display(720, 240, LEVEL_WIDTH, VBE_MODE::_8);
        Level level(&display);
        PortSerie ps;

        display.init();
        display.clear(0);
        display.set_palette(palette_vga);
        
        display.paint_picture(level_sprite_indices, 0, 0, LEVEL_WIDTH, LEVEL_HEIGHT);
        ps.ecrireMot("Mario Bros separated threads started\n");

        int oldX = 32, oldY = 100; // Init match SharedData defaults

        while(true) {
            data->lock.lock();
            int curX = data->marioX;
            int curY = data->marioY;
            int curScrollX = data->scrollX;
            bool curIsRight = data->isRight;
            data->lock.unlock();

            display.set_offset(curScrollX, 0);

            if (curX != oldX || curY != oldY) {
                 display.plot_moving_sprite(curIsRight ? marioSpriteData : marioSpriteDataReversed,
                                        MARIO_SPRITE_WIDTH, MARIO_SPRITE_HEIGHT,
                                        curX, curY,
                                        oldX, oldY,
                                        level_sprite_indices);
                 oldX = curX;
                 oldY = curY;
            }

            render_next_frame.P();
        }
    }
};

extern "C" void Sextant_main(unsigned long magic, unsigned long addr)
{
    Ecran ecran;
    Timer timer;

    idt_setup();
    irq_setup();
    // Initialisation de la frequence de l'horloge

    timer.i8254_set_frequency(1000);
    irq_set_routine(IRQ_TIMER, ticTac);

    asm volatile("sti\n"); // Autorise les interruptions

    irq_set_routine(IRQ_KEYBOARD, handler_clavier);

    multiboot_info_t *mbi;
    mbi = (multiboot_info_t *)addr;

    mem_setup(&__e_kernel, (mbi->mem_upper << 10) + (1 << 20), &ecran);

    ecran.effacerEcran(NOIR);

    thread_subsystem_setup(bootstrap_stack_bottom, bootstrap_stack_size);
    sched_subsystem_setup();

    irq_set_routine(IRQ_TIMER, sched_clk);

    // initialize pci bus to detect GPU address
    checkBus(0);

    // Create shared data
    static SharedData data;

    // Create and start threads
    static KeyboardThread kbd(&data);
    static LogicThread logic(&data, 720, 240);
    static DisplayThread display(&data);

    kbd.start();
    logic.start();
    display.start();

    while (1) {
        thread_yield();
    }
}
