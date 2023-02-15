#include <basics.h>
#include <device.h>

#define SCREEN_W 320
#define SCREEN_H 240

#define CTRL_OFFSET 0x00
#define SYNC_OFFSET 0x04

static uint32_t screen_width() {
  return SCREEN_W;
}

static uint32_t screen_height() { 
  return SCREEN_H;
}

static uint32_t screen_size() {
  return screen_width() * screen_height() * sizeof(uint8_t);
}

static void *vmem = NULL;
static uint8_t *vgactl_port_base = NULL;

#include <SDL2/SDL.h>

static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;

static void init_screen() {
  SDL_Window *window = NULL;
  char title[128];
  sprintf(title, "ISKEMU-UNISYS");
  SDL_Init(SDL_INIT_VIDEO);
  SDL_CreateWindowAndRenderer(980, 540, 0, &window, &renderer);
  SDL_SetWindowTitle(window, title);
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB332,
      SDL_TEXTUREACCESS_STATIC, SCREEN_W, SCREEN_H);
}

static inline void update_screen() {
  Log("biw");
  uint8_t *now = vmem;
  int a = 100;
  while(a--) {
    printf("%02x\n", *now++);
  }
  SDL_UpdateTexture(texture, NULL, vmem, SCREEN_W * sizeof(uint8_t));
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}

void vga_update_screen() {
  // TODO: call `update_screen()` when the sync register is non-zero,
  // then zero out the sync register
  uint8_t reg_sync = vgactl_port_base[SYNC_OFFSET];
  if(reg_sync) update_screen();
  vgactl_port_base[SYNC_OFFSET] = 0;
}

void init_vga() {
  vgactl_port_base = new_space(8);
  ((uint32_t *) vgactl_port_base)[CTRL_OFFSET] = (screen_width() << 16) | screen_height();

  add_mmio_map("vgactl", CONFIG_VGA_CTL_MMIO, vgactl_port_base, 8, NULL);

  vmem = new_space(screen_size());
  add_mmio_map("vmem", CONFIG_FB_ADDR, vmem, screen_size(), NULL);

  init_screen();
  memset(vmem, 0, screen_size());
}
