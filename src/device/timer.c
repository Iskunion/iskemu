#include <cpu.h>
#include <device.h>
#include <mainmem.h>

static uint32_t intr_nr = 0;
static uint32_t timer_rtc = 0;
// static uint32_t uptime = 0;

#define RTC_OFFSET    0x00
#define UPTIME_OFFSET 0x04
#define INTR_OFFSET   0x08

static uint8_t *rtc_port_base = NULL;

static void rtc_io_handler(uint32_t offset, int len, bool is_write) {
  switch (offset)
  {
    case RTC_OFFSET:
      if (is_write) timer_rtc = host_read(rtc_port_base + RTC_OFFSET, 4);
      else host_write(rtc_port_base + RTC_OFFSET, 4, timer_rtc = get_time() / 1000000);
      break;
    case UPTIME_OFFSET:
      if (is_write) panic("timer readonly");
      else host_write(rtc_port_base + UPTIME_OFFSET, 4, (uint32_t) get_time());
      break;
    case INTR_OFFSET:
      if (is_write) intr_nr = host_read(rtc_port_base + INTR_OFFSET, 4);
      else host_write(rtc_port_base + INTR_OFFSET, 4, intr_nr);
      break;
    default:
      panic("error");
      break;
  }
}

// static void timer_intr() {
//   if (nemu_state.state == NEMU_RUNNING) {
//     extern void dev_raise_intr();
//     dev_raise_intr();
//   }
// }

void init_timer() {
  rtc_port_base = new_space(12);
  timer_rtc = get_time();
  add_mmio_map("rtc", CONFIG_RTC_MMIO, rtc_port_base, 12, rtc_io_handler);

  // add_alarm_handle(timer_intr);
}
