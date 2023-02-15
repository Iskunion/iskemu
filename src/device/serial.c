#include <device.h>
#include <cpu.h>
#include <mainmem.h>

#define CTRL_OFFSET   0x00
#define STATUS_OFFSET 0x04
#define BAUD_OFFSET   0x08
#define TX_OFFSET     0x0c
#define RX_OFFSET     0x10

static uint8_t *serial_base = NULL;

typedef struct Serial_Ctrl
{
  bool tx_enable : 1;
  bool rx_enable : 1;
  uint32_t : 30;
} Serial_Ctrl;

typedef struct Serial_Status
{
  bool tx_busy  : 1;
  bool rx_ready : 1;
  uint32_t : 30;
} Serial_Status;

static Serial_Ctrl serial_ctrl = {.rx_enable = 0, .tx_enable = 0};
static Serial_Status serial_status = {.rx_ready = 0, .tx_busy = 0};
FILE *uartdatain;

static uint32_t ctrl2int(Serial_Ctrl target) {
  return *(uint32_t *) &target;
}
static Serial_Ctrl int2ctrl(uint32_t target) {
  return *(Serial_Ctrl *) &target;
}

static uint32_t status2int(Serial_Status target) {
  return *(uint32_t *) &target;
}
static Serial_Status int2status(uint32_t target) {
  return *(Serial_Status *) &target;
}

static void serial_putc(char ch) {
  putc(ch, stderr);
}

static void serial_io_handler(uint32_t offset, int len, bool is_write) {
  uint8_t ch;
  switch (offset) {
    case TX_OFFSET:
      if (is_write) serial_putc(host_read(serial_base + TX_OFFSET, 1));
      else panic("uart_tx do not support read");
      break;
    case RX_OFFSET:
      if (is_write) panic("uart_rx do not support write");
      else {
        ch = getc(uartdatain);
        host_write(serial_base + RX_OFFSET, 1, ch == EOF ? '\0' : ch);
      }
      break;
    case BAUD_OFFSET:
      panic("not implemented");
      break;
    case CTRL_OFFSET:
      if (is_write) serial_ctrl = int2ctrl(host_read(serial_base + CTRL_OFFSET, 4));
      else host_write(serial_base + CTRL_OFFSET, 4, ctrl2int(serial_ctrl));
      break;
    case STATUS_OFFSET:
      if (is_write) {
        Serial_Status data = int2status(host_read(serial_base + STATUS_OFFSET, 4));
        serial_status.rx_ready = data.rx_ready;
      }
      else host_write(serial_base + STATUS_OFFSET, 4, status2int(serial_status));
      break;
    default: panic("do not support offset = %d", offset);
  }
}

void init_serial() {
  serial_base = new_space(20);
  uartdatain = fopen("../res/uartdatain", "r");
  add_mmio_map("serial", CONFIG_SERIAL_MMIO, serial_base, 20, serial_io_handler);
}