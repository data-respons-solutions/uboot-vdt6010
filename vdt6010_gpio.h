#ifndef __VDT6010_GPIO_H__
#define __VDT6010_GPIO_H__

#include <asm/gpio.h>

#define USDHC2_CD		IMX_GPIO_NR(1, 4)
#define SPI_NOR_CS		IMX_GPIO_NR(5, 29)

#define UART1_RS485EN2	IMX_GPIO_NR(4, 11)
#define UART1_RS422EN2	IMX_GPIO_NR(4, 10)
#define UART1_RS232EN2	IMX_GPIO_NR(4, 9)

#define GPIO_1			IMX_GPIO_NR(4, 30)
#define GPIO_2			IMX_GPIO_NR(4, 31)
#define GPIO_3			IMX_GPIO_NR(5, 5)
#define GPIO_4			IMX_GPIO_NR(5, 6)
#define GPIO_5			IMX_GPIO_NR(5, 7)
#define GPIO_6			IMX_GPIO_NR(5, 8)
#define GPIO_7			IMX_GPIO_NR(5, 9)
#define GPIO_8			IMX_GPIO_NR(5, 10)

#define LAN8710_RST		IMX_GPIO_NR(6, 14)
#define LAN8710_IRQ		IMX_GPIO_NR(1, 9)

#define PCIE_RST		IMX_GPIO_NR(4, 8)

#define GPIO_MCU		IMX_GPIO_NR(1, 0)

#define LED_EN			IMX_GPIO_NR(7, 12)
#define DEBUG_LED		IMX_GPIO_NR(7, 8)
#define RTC_IRQ			IMX_GPIO_NR(1, 3)

#endif /* __VDT6010_GPIO_H__ */
