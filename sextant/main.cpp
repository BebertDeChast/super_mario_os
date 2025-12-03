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
#include <sextant/types.h>

#include <sextant/Synchronisation/Spinlock/Spinlock.h>

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

void plotXYnewXnewY(PortSerie ps, int X, int Y, int newX, int newY)
{
    ps.ecrireMot("X =");
    ps.afficherBase(X, 10);
    ps.ecrireMot(" Y =");
    ps.afficherBase(Y, 10);
    ps.ecrireMot(" -> NewX =");
    ps.afficherBase(newX, 10);
    ps.ecrireMot(" NewY =");
    ps.afficherBase(newY, 10);
    ps.ecrireMot("\n");
}

void mario_bros()
{
    // Ecran 720x240, mode 8 bits (256 couleurs)
    EcranBochs display(720, 240, LEVEL_WIDTH, VBE_MODE::_8);
    Level level(&display);
    PortSerie ps;

    display.init();
    display.clear(0);
    display.set_palette(palette_vga);

    // Position initiale
    int marioX = 32;
    int marioY = 100;
    int marioOldX = marioX;
    int marioOldY = marioY;
    int scrollX = 0;
    int scrollY = 0; // Ajouté car requis par la fonction update
    bool isRight = true;

    display.paint_picture(level_sprite_indices, 0, 0, LEVEL_WIDTH, LEVEL_HEIGHT);
    ps.ecrireMot("Mario Bros started\n");
    // Affichage initial de Mario
    // plotXYnewXnewY(ps, marioX, marioY, marioX, marioY);

    while (true)
    {
        update_mario_position(marioX, marioY, scrollX, scrollY, display.getWidth(), display.getHeight(), isRight);

        // if (marioX != marioOldX || marioY != marioOldY)
        //     plotXYnewXnewY(ps, marioOldX, marioOldY, marioX, marioY);
        display.plot_moving_sprite(isRight ? marioSpriteData : marioSpriteDataReversed,
                                   MARIO_SPRITE_WIDTH, MARIO_SPRITE_HEIGHT,
                                   marioX, marioY,
                                   marioOldX, marioOldY,
                                   level_sprite_indices);
        marioOldX = marioX;
        marioOldY = marioY;
        
        // 4. Mise à jour de la caméra
        display.set_offset(scrollX, 0);
    }
}
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

    mario_bros();
}
