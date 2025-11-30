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
extern size_t bootstrap_stack_size;	   // Taille de la pile d'exécution

void wait_vsync() {
    while (lireOctet(0x3DA) & 8);
    
    while (!(lireOctet(0x3DA) & 8));
}

void mario_bros()
{
    EcranBochs display(720, 240, 4000, VBE_MODE::_8);

    Level level(&display);

    display.init();
    display.clear(0);

    // only usefull in 4 or 8 bits modes
    display.set_palette(palette_vga);

	wait_vsync();

    display.paint_picture(level_sprite_indices, 0, 0, LEVEL_WIDTH, LEVEL_HEIGHT);
    
    int solY = 480 - 302; 
    int marioY = solY;
    int marioX = 0;
    int scrollX = 0;
    bool isRight = true;
    Clavier clavier;

    int verticalVelocity = 0;
    const int gravity = 2;      
    const int jumpImpulse = -20; 
    while (true)
    {
        display.paint_picture(level_sprite_indices, 0, 0, LEVEL_WIDTH, LEVEL_HEIGHT);

        // 2. Gestion des Entrées (Inputs)
        if (clavier.testChar()) {
            char c = clavier.getchar();
            int screen_width = display.getWidth();
            int screen_center = screen_width / 2;

            if (c == 'd') {
                isRight = true;
                int scroll_limit = LEVEL_WIDTH - screen_width;

                marioX += 16;
                
                // Scrolling vers la droite
                if ((marioX - scrollX) >= screen_center && scrollX < scroll_limit) {
                    scrollX += 16;
                    if (scrollX > scroll_limit) scrollX = scroll_limit;
                }
            } 
            else if (c == 'q') {
                isRight = false;
                if (marioX > scrollX) {
                    marioX -= 16;
                }
            }
            else if (c == 'z') {
                if (marioY >= solY) {
                    verticalVelocity = jumpImpulse;
                }
            }
        }

        // 3. Physique Verticale (Gravité)
        marioY += verticalVelocity;

        if (marioY < solY) {
            // Si Mario est en l'air, la gravité augmente sa vitesse de chute
            verticalVelocity += gravity;
        } else {
            // Atterrissage / Sol
            marioY = solY;
            verticalVelocity = 0;
        }

        // 4. On affiche Mario et on met à jour la caméra
        display.plot_sprite(isRight ? sprite_data : sprite_data_reversed, SPRITE_WIDTH, SPRITE_HEIGHT, marioX, marioY);
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
