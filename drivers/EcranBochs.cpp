#include "EcranBochs.h"

#include <hal/fonctionsES.h>
#include <hal/pci.h>
#include <sextant/types.h>
#include <drivers/Ecran.h>

ui8_t *EcranBochs::VRAM;

EcranBochs::EcranBochs(ui16_t displayedWidth, ui16_t height, ui16_t virtualWidth, VBE_MODE mode) : displayedWidth(displayedWidth), height(height), virtualWidth(virtualWidth), mode(mode), topBuffer(false), framebuffer(VRAM)
{
}

void EcranBochs::init()
{
    Ecran e;
    ui16_t detectScreen = lireRegistre(VBE_INDEX::ID);

    if (detectScreen != 0xB0C5)
    {
        // error should print to serial port ?
        e.afficherMot("No card found");
    }

    // disable card
    ecrireRegistre(VBE_INDEX::ENABLE, VBE_DISPI_DISABLED);

    // set size & bit depth
    ecrireRegistre(VBE_INDEX::XRES, displayedWidth);
    ecrireRegistre(VBE_INDEX::YRES, height);
    ecrireRegistre(VBE_INDEX::BPP, mode);

    // set virtual width for double buffering
    ecrireRegistre(VBE_INDEX::VIRT_WIDTH, virtualWidth);
    ecrireRegistre(VBE_INDEX::X_OFFSET, 0);
    ecrireRegistre(VBE_INDEX::Y_OFFSET, 0);

    // enable screen
    ecrireRegistre(VBE_INDEX::ENABLE, VBE_DISPI_ENABLED | VBE_DISPI_LFB_ENABLED);
}

void EcranBochs::swapBuffer()
{
    ecrireRegistre(VBE_INDEX::Y_OFFSET, topBuffer ? 0 : height);

    if (!topBuffer)
    {
        framebuffer = VRAM;
    }
    else
    {
        framebuffer = VRAM + displayedWidth * height * bytesPerPixel();
    }

    topBuffer = !topBuffer;
}

void EcranBochs::clear(ui8_t color)
{
    for (ui16_t y = 0; y < height; y++)
    {
        for (ui16_t x = 0; x < displayedWidth; x++)
        {
            paint(x, y, color);
        }
    }
}

ui16_t EcranBochs::getWidth()
{
    return displayedWidth;
}

ui16_t EcranBochs::getHeight()
{
    return height;
}

void EcranBochs::set_palette(ui8_t palette_vga[256][3])
{
    ecrireOctet(0, 0x3C8);
    for (int i = 0; i < 256; ++i)
    {
        ecrireOctet(palette_vga[i][0], 0x3C9);
        ecrireOctet(palette_vga[i][1], 0x3C9);
        ecrireOctet(palette_vga[i][2], 0x3C9);
    }
}

void EcranBochs::paint(unsigned int x, unsigned int y, char color)
{
    if (mode == VBE_MODE::_8)
    {
        ui32_t offset = y * displayedWidth + x;
        VRAM[offset] = color;
    }
}

void EcranBochs::plot_square(int x, int y, int size, ui8_t color)
{
    for (int row = 0; row < size; row++)
    {
        ui32_t base = (y + row) * displayedWidth + x;
        for (int col = 0; col < size; col++)
        {
            framebuffer[base + col] = color;
        }
    }
}

void EcranBochs::plot_palette(int x, int y, int size)
{
    int row, col;
    for (row = 0; row < 16; row++)
    {
        for (col = 0; col < 16; col++)
        {
            plot_square(x + col * size, y + row * size, size, row * 16 + col);
        }
    }
}

void EcranBochs::plot_sprite(void *buffer, ui16_t width, ui16_t height, ui16_t x, ui16_t y)
{

    ui8_t *buf = (ui8_t *)buffer;
    for (ui16_t row = 0; row < height; row++)
    {
        ui32_t base = (y + row) * getWidth() + x;
        for (ui16_t col = 0; col < width; col++)
        {
            ui8_t color = *buf++;
            if (color != 0)
            {
                framebuffer[base + col] = color;
            }
        }
    }
}

ui16_t EcranBochs::lireRegistre(VBE_INDEX id)
{
    ecrireMot(id, VBE_DISPI_IOPORT_INDEX);
    return lireMot(VBE_DISPI_IOPORT_DATA);
}

void EcranBochs::ecrireRegistre(VBE_INDEX id, ui16_t value)
{
    ecrireMot(id, VBE_DISPI_IOPORT_INDEX);
    ecrireMot(value, VBE_DISPI_IOPORT_DATA);
}

ui8_t EcranBochs::bytesPerPixel()
{
    if (mode == VBE_MODE::_15)
        return 2;
    return mode / 8;
}

void EcranBochs::set_offset(ui16_t x, ui16_t y)
{
    ecrireRegistre(VBE_INDEX::X_OFFSET, x);
    ecrireRegistre(VBE_INDEX::Y_OFFSET, y);
}

void EcranBochs::paint_picture(const unsigned char *picture, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{
    for (unsigned int row = 0; row < h; row++)
    {
        ui32_t base = (y + row) * displayedWidth + x;
        for (unsigned int col = 0; col < w; col++)
        {
            ui8_t color = picture[row * w + col];
            framebuffer[base + col] = color;
        }
    }
}