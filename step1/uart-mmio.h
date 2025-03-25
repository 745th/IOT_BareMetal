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

#ifndef UART_MMIO_H_
#define UART_MMIO_H_

/**
 * To fill this header file,
 * look at the document describing the Versatile Application Board:
 *
 *    Versatile Application Baseboard for ARM926EJ-S (DUI0225)
 */

/*
 * We need the base address for the different serial lines.
 */

#define UART0_BASE_ADDRESS ((void*)0x101F1000) // ???
#define UART1_BASE_ADDRESS ((void*)0x101F2000) // ???
#define UART2_BASE_ADDRESS ((void*)0x101F3000) // ???

/*
 * Is the UART chipset a PL011?
 * If so, we need the details for the data and status registers.
 */
#define UART_DR 0x0000 // ???
#define UART_FR 0x018 // ???
#define UART_FIFO 0x0030
#define UART_FR_TFUL (1 << 5)
#define UART_FR_REMPTY (1 << 4)

#endif /* UART_MMIO_H_ */
