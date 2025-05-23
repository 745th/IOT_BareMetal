/*
 * Copyright: Olivier Gruber (olivier dot gruber at acm dot org)
 *
 * This program is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 */

#include "main.h"
#include "uart.h"
#include "uart-mmio.h"

struct uart uarts[NUARTS];

struct queue evlist;

//rl for read listener
void uart_init(uint32_t uartno, void* bar) {
  struct uart* uart = &uarts[uartno];
  uart->uartno = uartno;
  uart->bar = bar;
  // no hardware initialization necessary
  // when running on QEMU, the UARTs are
  // already initialized, as long as we
  // do not rely on interrupts.
}

void uarts_init() {
  uart_init(UART0,UART0_BASE_ADDRESS);
  uart_init(UART1,UART1_BASE_ADDRESS);
  uart_init(UART2,UART2_BASE_ADDRESS);
}

struct uart* getuart(int uartno)
{
  return &uarts[uartno];
}

void uart_enable(uint32_t uartno) {
  struct uart* uart = &uarts[uartno];
  
  //enable RX interrupt
  mmio_write32(uart->bar,0x038,UART_FR_REMPTY);

}

void uart_disable(uint32_t uartno) {
  struct uart*uart = &uarts[uartno];
  
  //disable RX interrupt
  mmio_write32(uart->bar,0x044, UART_FR_REMPTY);
}

void uart_receive(uint8_t uartno, char *pt) {
  struct uart*uart = &uarts[uartno];
  if(!(mmio_read32(uart->bar,UART_FR) & UART_FR_REMPTY))
  {
    if(mmio_read32(uart->bar,UART_DR) == 'p')
    {
      uart_send_string(uartno,"\033[H\033[J");
    }
    else
    {
      *pt = mmio_read32(uart->bar,UART_DR);
    }
  }
  else
  {
    *pt=0;
  }
    
}

/**
 * Sends a character through the given uart, this is a blocking call
 * until the character has been sent.
 */
void uart_send(uint8_t uartno, char s) {
  struct uart* uart = &uarts[uartno];
  while((mmio_read32(uart->bar,UART_FR) & UART_FR_TFUL));
    mmio_write32(uart->bar,UART_DR, s);
  //panic();
}

/**
 * This is a wrapper function, provided for simplicity,
 * it sends a C string through the given uart.
 */
void uart_send_string(uint8_t uartno, const char *s) {
  while (*s != '\0') {
    uart_send(uartno, *s);
    s++;
  }
}

void event_put(void (*handler)(void* arg)) {
  uint32_t next = (evlist.head + 1) % MAX_EVENTS;
  evlist.list[evlist.head].handler = handler;
  evlist.head = next;
}

struct event* event_pop() {
  struct event* ev;
  uint32_t next = (evlist.tail + 1) % MAX_EVENTS;
  ev = &evlist.list[evlist.tail];
  evlist.tail = next;
  return ev;
}

void event_init()
{
  for(int i=0;i<MAX_EVENTS;i++)
  {
    evlist.list[i].handler = NULL;
  }
}

void event_reset()
{
  evlist.tail =0;
}

