/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <device.h>
#include <basics.h>

#include <SDL2/SDL.h>

#define KEYDOWN_MASK 0xf000
#define _KEY_NAME(k) _KEY_##k,

#define SDL_KEYMAP(k) keymap[concat(SDL_SCANCODE_, k)] = concat(AM_KEY_, k);

static uint32_t keymap[256] = {};

static void init_keymap() {
  _KEYS(SDL_KEYMAP)
}

#define KEY_QUEUE_LEN 1024
static int key_queue[KEY_QUEUE_LEN] = {};
static int key_f = 0, key_r = 0;

static void key_enqueue(uint32_t am_scancode) {
  key_queue[key_r] = am_scancode;
  key_r = (key_r + 1) % KEY_QUEUE_LEN;
  Assert(key_r != key_f, "key queue overflow!");
}

static uint32_t key_dequeue() {
  uint32_t key = 0;
  if (key_f != key_r) {
    key = key_queue[key_f];
    key_f = (key_f + 1) % KEY_QUEUE_LEN;
  }
  return key;
}

void send_key(uint8_t scancode, bool is_keydown) {
  uint32_t rcode = keymap[scancode];
  if (nemu_state.state == NEMU_RUNNING && scancode != 0) {
    uint32_t am_scancode = rcode | (!is_keydown ? KEYDOWN_MASK : 0);
    // Log("translate code to 0x%08x", am_scancode);
    key_enqueue(am_scancode);
  }
}

static uint32_t *i8042_data_port_base = NULL;

static void i8042_data_io_handler(uint32_t offset, int len, bool is_write) {
  assert(!is_write);
  assert(offset == 4);
  i8042_data_port_base[1] = key_dequeue();
  // Log("translate code to 0x%08x", i8042_data_port_base[2]);
}

void init_i8042() {
  i8042_data_port_base = (uint32_t *) new_space(16);
  i8042_data_port_base[1] = 0;

  init_keymap();

  add_mmio_map("keyboard", CONFIG_I8042_DATA_MMIO, i8042_data_port_base, 16, i8042_data_io_handler);
}
