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

extern char __e_kernel, __b_kernel, __b_data, __e_data, __b_stack, __e_load;
int i;

extern vaddr_t bootstrap_stack_bottom; // Adresse de début de la pile d'exécution
extern size_t bootstrap_stack_size;	   // Taille de la pile d'exécution

void mario_bros()
{
	EcranBochs display(720, 480, 4000, VBE_MODE::_8);

	Level level(&display);

	display.init();
	display.clear(0);

	// only usefull in 4 or 8 bits modes
	display.set_palette(palette_vga);
	// display.plot_palette(0, 0, 25);

	display.paint_picture(level_sprite_indices, 0, 0, LEVEL_WIDTH, LEVEL_HEIGHT);

	
	
	int x = 0;
	Clavier clavier;
	while (1)
	{
		if (clavier.testChar()) {
			char c = clavier.getchar();
			if (c == 'd') {
				int limit = LEVEL_WIDTH - display.getWidth();
				if (x < limit)
					x = (x + 16 > limit) ? limit : x + 16;
			}
			display.set_offset(x, 0);
		}
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
