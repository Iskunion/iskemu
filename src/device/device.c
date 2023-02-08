
#include <basics.h>
#include <device.h>
#include <SDL2/SDL.h>

void init_map();
void init_serial();
void init_timer();
void init_vga();
void init_i8042();
void init_audio();
void init_disk();
void init_sdcard();
void init_alarm();

void send_key(uint8_t, bool);
void vga_update_screen();

void device_update() {
  static uint64_t last = 0;
  uint64_t now = get_time();
  if (now - last < 1000000 / TIMER_HZ) {
    return;
  }
  last = now;

  // vga_update_screen();

  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
        nemu_state.state = NEMU_QUIT;
        break;
      // If a key was pressed
      // case SDL_KEYDOWN:
      // case SDL_KEYUP: {
      //   uint8_t k = event.key.keysym.scancode;
      //   bool is_keydown = (event.key.type == SDL_KEYDOWN);
      //   send_key(k, is_keydown);
      //   break;
      // }
      default: break;
    }
  }
}

void sdl_clear_event_queue() {
  SDL_Event event;
  while (SDL_PollEvent(&event));
}

void init_device() {

  init_map();
  // init_alarm();
  init_serial();
  init_timer();

  // init_vga();
  // init_i8042();
  // init_audio();
  // init_disk();

  // init_sdcard();
  // init_media();
}
