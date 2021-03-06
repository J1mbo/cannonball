/***************************************************************************
    Video Rendering. 
    
    - Renders the System 16 Video Layers
    - Handles Reads and Writes to these layers from the main game code
    - Interfaces with platform specific rendering code

    Copyright Chris White.
    See license.txt for more details.
***************************************************************************/

#include <iostream>

#include "video.hpp"
#include "setup.hpp"
#include "globals.hpp"
#include "frontend/config.hpp"

#ifdef WITH_OPENGL

#if defined SDL2
#include "sdl2/rendergl.hpp"
#else
#include "sdl/rendergl.hpp"
#endif

#endif

#if defined SDL2

#if defined WITH_OPENGLES
#include "sdl2/rendergles.hpp"
#else
#include "sdl2/rendersurface.hpp"
#endif

#else
#include "sdl/rendersw.hpp"
#endif //SDL2

Video video;

Video::Video(void)
{
    #ifdef WITH_OPENGL
    renderer     = new RenderGL();

    #elif defined SDL2

    #ifdef WITH_OPENGLES
    renderer	 = new RenderGLES();
    #else
    renderer     = new RenderSurface();
    #endif

    #else
    renderer     = new RenderSW();
    #endif

    pixels       = NULL;
    sprite_layer = new hwsprites();
    tile_layer   = new hwtiles();
}

Video::~Video(void)
{
    video.disable();
    delete tile_layer;
    delete sprite_layer;
    delete renderer;
}

int Video::init(Roms* roms, video_settings_t* settings)
{
    if (!set_video_mode(settings))
        return 0;

    // Internal pixel array. The size of this is always constant
    pixels = new uint16_t[config.s16_width * config.s16_height];

    // Convert S16 tiles to a more useable format
    tile_layer->init(roms->tiles.rom, config.video.hires != 0);
    clear_tile_ram();
    clear_text_ram();
    if (roms->tiles.rom)
    {
        delete[] roms->tiles.rom;
        roms->tiles.rom = NULL;
    }

    // Convert S16 sprites
    sprite_layer->init(roms->sprites.rom);
    if (roms->sprites.rom)
    {
        delete[] roms->sprites.rom;
        roms->sprites.rom = NULL;
    }

    // Convert S16 Road Stuff
    hwroad.init(roms->road.rom, config.video.hires != 0);
    if (roms->road.rom)
    {
        delete[] roms->road.rom;
        roms->road.rom = NULL;
    }

    enabled = true;
    return 1;
}

void Video::disable()
{
    renderer->disable();
    delete[] pixels;
}

// ------------------------------------------------------------------------------------------------
// Configure video settings from config file
// ------------------------------------------------------------------------------------------------

int Video::set_video_mode(video_settings_t* settings)
{
    if (settings->widescreen)
    {
        config.s16_width  = S16_WIDTH_WIDE;
        config.s16_x_off = (S16_WIDTH_WIDE - S16_WIDTH) / 2;
    }
    else
    {
        config.s16_width = S16_WIDTH;
        config.s16_x_off = 0;
    }

    config.s16_height = S16_HEIGHT;

    // Internal video buffer is doubled in hi-res mode.
    if (settings->hires)
    {
        config.s16_width  <<= 1;
        config.s16_height <<= 1;
    }

    if (settings->scanlines < 0) settings->scanlines = 0;
    else if (settings->scanlines > 100) settings->scanlines = 100;

    if (settings->scale < 1)
        settings->scale = 1;

    renderer->init(config.s16_width, config.s16_height, settings->scale, settings->mode, settings->scanlines);

    return 1;
}

void Video::draw_frame()
{
    // Renderer Specific Frame Setup
    if (!renderer->start_frame())
        return;

    if (!enabled)
    {
        // Fill with black pixels
        for (int i = 0; i < config.s16_width * config.s16_height; i++)
            pixels[i] = 0;
    }
    else
    {
        // OutRun Hardware Video Emulation
        tile_layer->update_tile_values();

        (hwroad.*hwroad.render_background)(pixels);
        tile_layer->render_tile_layer(pixels, 1, 0);      // background layer
        tile_layer->render_tile_layer(pixels, 0, 0);      // foreground layer
        (hwroad.*hwroad.render_foreground)(pixels);
        sprite_layer->render(8);
        tile_layer->render_text_layer(pixels, 1);

     }

//    renderer->draw_frame(pixels);
//    renderer->finalize_frame();
}


void Video::finalize_frame()
{
    renderer->draw_frame(pixels);
    renderer->finalize_frame();
}


// ---------------------------------------------------------------------------
// Text Handling Code
// ---------------------------------------------------------------------------

void Video::clear_text_ram()
{
    for (uint32_t i = 0; i <= 0xFFF; i++)
        tile_layer->text_ram[i] = 0;
}

void Video::write_text8(uint32_t addr, const uint8_t data)
{
    tile_layer->text_ram[addr & 0xFFF] = data;
}

void Video::write_text16(uint32_t* addr, const uint16_t data)
{
    tile_layer->text_ram[*addr & 0xFFF] = (data >> 8) & 0xFF;
    tile_layer->text_ram[(*addr+1) & 0xFFF] = data & 0xFF;

    *addr += 2;
}

void Video::write_text16(uint32_t addr, const uint16_t data)
{
    tile_layer->text_ram[addr & 0xFFF] = (data >> 8) & 0xFF;
    tile_layer->text_ram[(addr+1) & 0xFFF] = data & 0xFF;
}

void Video::write_text32(uint32_t* addr, const uint32_t data)
{
    tile_layer->text_ram[*addr & 0xFFF] = (data >> 24) & 0xFF;
    tile_layer->text_ram[(*addr+1) & 0xFFF] = (data >> 16) & 0xFF;
    tile_layer->text_ram[(*addr+2) & 0xFFF] = (data >> 8) & 0xFF;
    tile_layer->text_ram[(*addr+3) & 0xFFF] = data & 0xFF;

    *addr += 4;
}

void Video::write_text32(uint32_t addr, const uint32_t data)
{
    tile_layer->text_ram[addr & 0xFFF] = (data >> 24) & 0xFF;
    tile_layer->text_ram[(addr+1) & 0xFFF] = (data >> 16) & 0xFF;
    tile_layer->text_ram[(addr+2) & 0xFFF] = (data >> 8) & 0xFF;
    tile_layer->text_ram[(addr+3) & 0xFFF] = data & 0xFF;
}

uint8_t Video::read_text8(uint32_t addr)
{
    return tile_layer->text_ram[addr & 0xFFF];
}

// ---------------------------------------------------------------------------
// Tile Handling Code
// ---------------------------------------------------------------------------

void Video::clear_tile_ram()
{
//    for (uint32_t i = 0; i <= 0xFFFF; i++)
//        tile_layer->tile_ram[i] = 0;
    memset(tile_layer->tile_ram,0,0xFFFF); // JJP
}

void Video::write_tile8(uint32_t addr, const uint8_t data)
{
    tile_layer->tile_ram[addr & 0xFFFF] = data;
} 

void Video::write_tile16(uint32_t* addr, const uint16_t data)
{
    tile_layer->tile_ram[*addr & 0xFFFF] = (data >> 8) & 0xFF;
    tile_layer->tile_ram[(*addr+1) & 0xFFFF] = data & 0xFF;

    *addr += 2;
}

void Video::write_tile16(uint32_t addr, const uint16_t data)
{
    tile_layer->tile_ram[addr & 0xFFFF] = (data >> 8) & 0xFF;
    tile_layer->tile_ram[(addr+1) & 0xFFFF] = data & 0xFF;
}   

void Video::write_tile32(uint32_t* addr, const uint32_t data)
{
    tile_layer->tile_ram[*addr & 0xFFFF] = (data >> 24) & 0xFF;
    tile_layer->tile_ram[(*addr+1) & 0xFFFF] = (data >> 16) & 0xFF;
    tile_layer->tile_ram[(*addr+2) & 0xFFFF] = (data >> 8) & 0xFF;
    tile_layer->tile_ram[(*addr+3) & 0xFFFF] = data & 0xFF;

    *addr += 4;
}

void Video::write_tile32(uint32_t addr, const uint32_t data)
{
    tile_layer->tile_ram[addr & 0xFFFF] = (data >> 24) & 0xFF;
    tile_layer->tile_ram[(addr+1) & 0xFFFF] = (data >> 16) & 0xFF;
    tile_layer->tile_ram[(addr+2) & 0xFFFF] = (data >> 8) & 0xFF;
    tile_layer->tile_ram[(addr+3) & 0xFFFF] = data & 0xFF;
}

uint8_t Video::read_tile8(uint32_t addr)
{
    return tile_layer->tile_ram[addr & 0xFFFF];
}


// ---------------------------------------------------------------------------
// Sprite Handling Code
// ---------------------------------------------------------------------------

void Video::write_sprite16(uint32_t* addr, const uint16_t data)
{
    sprite_layer->write(*addr & 0xfff, data);
    *addr += 2;
}

// ---------------------------------------------------------------------------
// Palette Handling Code
// ---------------------------------------------------------------------------

void Video::write_pal8(uint32_t* palAddr, const uint8_t data)
{
    palette[*palAddr & 0x1fff] = data;
    refresh_palette(*palAddr & 0x1fff);
    *palAddr += 1;
}

void Video::write_pal16(uint32_t* palAddr, const uint16_t data)
{    
    uint32_t adr = *palAddr & 0x1fff;
    palette[adr]   = (data >> 8) & 0xFF;
    palette[adr+1] = data & 0xFF;
    refresh_palette(adr);
    *palAddr += 2;
}

void Video::write_pal32(uint32_t* palAddr, const uint32_t data)
{    
    uint32_t adr = *palAddr & 0x1fff;

    palette[adr]   = (data >> 24) & 0xFF;
    palette[adr+1] = (data >> 16) & 0xFF;
    palette[adr+2] = (data >> 8) & 0xFF;
    palette[adr+3] = data & 0xFF;

    refresh_palette(adr);
    refresh_palette(adr+2);

    *palAddr += 4;
}

void Video::write_pal32(uint32_t adr, const uint32_t data)
{    
    adr &= 0x1fff;

    palette[adr]   = (data >> 24) & 0xFF;
    palette[adr+1] = (data >> 16) & 0xFF;
    palette[adr+2] = (data >> 8) & 0xFF;
    palette[adr+3] = data & 0xFF;
    refresh_palette(adr);
    refresh_palette(adr+2);
}

uint8_t Video::read_pal8(uint32_t palAddr)
{
    return palette[palAddr & 0x1fff];
}

uint16_t Video::read_pal16(uint32_t palAddr)
{
    uint32_t adr = palAddr & 0x1fff;
    return (palette[adr] << 8) | palette[adr+1];
}

uint16_t Video::read_pal16(uint32_t* palAddr)
{
    uint32_t adr = *palAddr & 0x1fff;
    *palAddr += 2;
    return (palette[adr] << 8)| palette[adr+1];
}

uint32_t Video::read_pal32(uint32_t* palAddr)
{
    uint32_t adr = *palAddr & 0x1fff;
    *palAddr += 4;
    return (palette[adr] << 24) | (palette[adr+1] << 16) | (palette[adr+2] << 8) | palette[adr+3];
}

// Convert internal System 16 RRRR GGGG BBBB format palette to renderer output format
void Video::refresh_palette(uint32_t palAddr)
{
/*
Palette format:
        D15 : Shade hi/lo
 	D14 : Blue bit 0
	D13 : Green bit 0
	D12 : Red bit 0
	D11 : Blue bit 4
	D10 : Blue bit 3
	D9  : Blue bit 2
	D8  : Blue bit 1
	D7  : Green bit 4
	D6  : Green bit 3
	D5  : Green bit 2
	D4  : Green bit 1
	D3  : Red bit 4
	D2  : Red bit 3
	D1  : Red bit 2
	D0  : Red bit 1

 Tiles from the text layer use color entries 0 to 63, divided into eight
 8-color palettes.

 Tiles from the foreground and background layers use color entries 0-1023,
 divided into 128 8-color palettes.

 Sprites use color entries 1024-2047, divided into 64 16-color palettes.

 The backdrop color is shown behind all layers in the active display area
 and is selected from palette entry #0. The remainder of the frame (meaning
 overscan, vertical retrace, and vertical blank) is always shown as black.
 If the display is turned off through bit 5 of $C40001, the affected lines
 are also shown as black.

Shadow / hilight processing:

 If a sprite uses palette $3F, its' pixels with values 1 to 14 change how
 underlying pixels from the background are shown (Values 0 and 15 are
 still treated as transparent).

 For each background pixel that is obscured by the sprite in question,
 bit 15 of its' corresponding entry from color RAM is checked. If cleared,
 the color is shown at half intensity. If set, the color is shown at double
 intensity (or brightness, if that's a better way to describe it). The actual
 sprite pixel is not shown at all. Most games leave bit 15 cleared in all
 entries, so any sprite using palette $3F will create a shadow.

 Sprite priority still works in the same way: if a shadow or hilight sprite
 is behind one layer but in front of another, only the pixels behind the
 sprite are affected. Shadow and hilight processing affects the backdrop
 color, text layer, and both the foreground and background layers. It has no
 effect on other sprites.

 If two sprites overlap each other and one of them uses palette $3F, the last
 one drawn (having the highest inter-sprite priority) will select how the
 background is affected.
*/

    palAddr &= ~1;
    uint32_t a = (palette[palAddr] << 8) | palette[palAddr + 1];
    uint32_t r = (a & 0x000f) << 1; // r rrr0
    uint32_t g = (a & 0x00f0) >> 3; // g ggg0
    uint32_t b = (a & 0x0f00) >> 7; // b bbb0
    if ((a & 0x1000) != 0)
        r |= 1; // r rrrr
    if ((a & 0x2000) != 0)
        g |= 1; // g gggg
    if ((a & 0x4000) != 0)
        b |= 1; // b bbbb
//    if ((a & 0x8000) != 0)
//        hilite = 1; // highlight bit

    renderer->convert_palette(palAddr, r, g, b);
}
