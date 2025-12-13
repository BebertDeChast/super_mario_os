#include <hal/multiboot.h>
#include <drivers/Ecran.h>
#include <drivers/PortSerie.h>

#include <sextant/interruptions/idt.h>
#include <sextant/interruptions/irq.h>
#include <sextant/interruptions/handler/handler_tic.h>
#include <sextant/interruptions/handler/handler_clavier.h>
#include <drivers/timer.h>

#include <sextant/memoire/memoire.h>

#include <sextant/ordonnancements/preemptif/thread.h>
#include <sextant/Activite/Threads.h>
#include <sextant/types.h>

#include <sextant/Synchronisation/Semaphore/Semaphore.h>

#include <hal/pci.h>

#include <Applications/Keyboard/Keyboard.h>
#include <Applications/MarioBros/Logic.h>
#include <Applications/MarioBros/MobLogic.h>
#include <Applications/GameDisplay/GameDisplay.h>

extern char __e_kernel, __b_kernel, __b_data, __e_data, __b_stack, __e_load;
int i;

extern vaddr_t bootstrap_stack_bottom; // Adresse de début de la pile d'exécution
extern size_t bootstrap_stack_size;    // Taille de la pile d'exécution

Semaphore render_next_frame;

extern "C" void Sextant_main(unsigned long magic, unsigned long addr)
{
    Ecran ecran;
    Timer timer;
    PortSerie ps;

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
    static GameData data;

    // Initialize GameData with default values before starting threads
    data.marioX = 50;
    data.marioY = 180;
    data.scrollX = 0;
    data.scrollY = 0;
    data.marioSprite = marioSpriteData; // Assign the default right-facing sprite
    data.goombaX = 200;
    data.goombaY = 180;
    data.goombaActive = true;
    data.resetGoomba = false;

    ps.ecrireMot("\nStarting MarioBros...\n");
    ps.afficherGameData(&data);
    data.lock.V();
    kbdData.lock.V();

    // Create and start threads
    static KeyboardThread kbd(&kbdData);
    static LogicThread logic(&kbdData, &data, 720, 240);
    static MobLogic mobLogic(&data, 720, 240);
    static GameDisplay display(&data);

    kbd.start();
    logic.start();
    mobLogic.start();
    display.start();

    while (1)
    {
        thread_yield();
    }
}
