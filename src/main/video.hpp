/***************************************************************************
    Video Rendering. 
    
    - Renders the System 16 Video Layers
    - Handles Reads and Writes to these layers from the main game code
    - Interfaces with platform specific rendering code

    Copyright Chris White.
    See license.txt for more details.
***************************************************************************/

#pragma once

#include "stdint.hpp"
#include "globals.hpp"
#include "roms.hpp"
#include "hwvideo/hwtiles.hpp"
#include "hwvideo/hwsprites.hpp"
#include "hwvideo/hwroad.hpp"

namespace shadow
{
    const static float ORIGINAL = 0.63f; // Hardware Intensity (63%)
    const static float MAME = 0.78f;     // Mame Intensity (78%)
};

class hwsprites;
class RenderBase;
struct video_settings_t;

class Video
{
public:
	hwsprites* sprite_layer;
    hwtiles* tile_layer;
    uint16_t* pixel_buffers[2];
    uint16_t* pixels;
    int current_pixel_buffer;

    bool enabled;

	Video();
    ~Video();
    
	int init(Roms* roms, video_settings_t* settings);
    void swap_buffers();
    void disable();
    int set_video_mode(video_settings_t* settings);
    void set_shadow_intensity(float);
    void prepare_frame();
    void render_frame();
    void present_frame(); 
    bool supports_window();
    bool supports_vsync();

    void clear_text_ram();
    void write_text8(uint32_t, const uint8_t);
	void write_text16(uint32_t*, const uint16_t);
	void write_text16(uint32_t, const uint16_t);
    void write_text32(uint32_t*, const uint32_t);
    void write_text32(uint32_t, const uint32_t);
    uint8_t read_text8(uint32_t);

    void clear_tile_ram();    
    void write_tile8(uint32_t, const uint8_t);
	void write_tile16(uint32_t*, const uint16_t);
	void write_tile16(uint32_t, const uint16_t);
    void write_tile32(uint32_t*, const uint32_t);
    void write_tile32(uint32_t, const uint32_t);
    uint8_t read_tile8(uint32_t);

	void write_sprite16(uint32_t*, const uint16_t);

	void write_pal8(uint32_t*, const uint8_t);
	void write_pal16(uint32_t*, const uint16_t);
	void write_pal32(uint32_t*, const uint32_t);
	void write_pal32(uint32_t, const uint32_t);
	uint8_t read_pal8(uint32_t);
	uint16_t read_pal16(uint32_t*);
	uint16_t read_pal16(uint32_t);
    uint32_t read_pal32(uint32_t*);

private:
    // SDL Renderer
    RenderBase* renderer;
    
	uint8_t palette[S16_PALETTE_ENTRIES * 2]; // 2 Bytes Per Palette Entry
    void refresh_palette(uint32_t);

};

extern Video video;