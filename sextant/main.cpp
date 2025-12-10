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

#include <Applications/Keyboard/Keyboard.h>
#include <Applications/MarioBros/Logic.h>

extern char __e_kernel, __b_kernel, __b_data, __e_data, __b_stack, __e_load;
int i;

extern vaddr_t bootstrap_stack_bottom; // Adresse de début de la pile d'exécution
extern size_t bootstrap_stack_size;    // Taille de la pile d'exécution

Semaphore render_next_frame;

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
            data->lock.P();
            int curX = data->marioX;
            int curY = data->marioY;
            int curScrollX = data->scrollX;
            unsigned char *curSprite = data->marioSprite;
            data->lock.V();

            display.set_offset(curScrollX, 0);

            if (curX != oldX || curY != oldY) {
                 display.plot_moving_sprite(curSprite,
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
    static KeyboardData kbdData;
    static SharedData data;

    // Create and start threads
    static KeyboardThread kbd(&kbdData);
    static LogicThread logic(&kbdData, &data, 720, 240);
    static DisplayThread display(&data);

    kbd.start();
    logic.start();
    display.start();

    while (1) {
        thread_yield();
    }
}
