/*
 * Copyright (C) 2011 Fergus Noble <fergusnoble@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "spi.h"
#include "max2769.h"
#include <libopencm3/stm32/f4/gpio.h>
#include <libopencm3/stm32/f4/rcc.h>

void max2769_write(u8 addr, u32 data)
{
  u32 write_word = ((data << 4) & 0xFFFFFFF0) | (addr & 0x0F);

  spi_slave_select(SPI_SLAVE_FRONTEND);

  spi_xfer(SPI_BUS_FRONTEND, (write_word >> 24) & 0xFF);
  spi_xfer(SPI_BUS_FRONTEND, (write_word >> 16) & 0xFF);
  spi_xfer(SPI_BUS_FRONTEND, (write_word >>  8) & 0xFF);
  spi_xfer(SPI_BUS_FRONTEND, (write_word >>  0) & 0xFF);

  spi_slave_deselect();

}

void max2769_setup()
{
  /* Setup MAX2769 PGM (PB8) - low */
  RCC_AHB1ENR |= RCC_AHB1ENR_IOPBEN;
	gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, GPIO8);
  gpio_clear(GPIOB, GPIO8);

  /* Setup MAX2769 NSHDN (PB9) - high */
	gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO9);
  gpio_set(GPIOB, GPIO9);

  /* Setup MAX2769 NIDLE (PB10) - high */
	gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO10);
  gpio_set(GPIOB, GPIO10);

  /* Register settings from Colin "max_regs_defaults.vhd" */
//  max2769_write(MAX2769_CONF1, 0xA2959A3); //LNA1 selected
  max2769_write(MAX2769_CONF1, 0xA2939A3); //LNA2 selected
  volatile u32 conf1 = 0;
  conf1 = MAX2769_CONF1_CHIPEN |
          MAX2769_CONF1_ILNA1(8) |
          MAX2769_CONF1_ILNA2(2) |
          MAX2769_CONF1_ILO(2) |
          MAX2769_CONF1_IMIX(1) |
          MAX2769_CONF1_MIXPOLE_13MHZ |
          MAX2769_CONF1_MIXEN |
          MAX2769_CONF1_ANTEN |
          MAX2769_CONF1_FCEN(26) |
          MAX2769_CONF1_FBW_2_5MHZ |
          MAX2769_CONF1_F3OR5_5 |
          MAX2769_CONF1_FCENX_BP |
          MAX2769_CONF1_FGAIN_HIGH;
  conf1 |= MAX2769_CONF1_LNAMODE_LNA2;

//  max2769_write(MAX2769_CONF1, 0xA2919A3); //LNA1|LNA2: current gated
  max2769_write(MAX2769_CONF2, 0x8550308);
  volatile u32 conf2 = 0;
  conf2 = MAX2769_CONF2_IQEN |
          MAX2769_CONF2_GAINREF(170) |
          MAX2769_CONF2_AGCMODE_INDEP |
          MAX2769_CONF2_FORMAT_SIGN_MAG |
          MAX2769_CONF2_BITS_3 |
          MAX2769_CONF2_DRVCFG_CMOS |
          MAX2769_CONF2_LOEN;

  max2769_write(MAX2769_CONF3, 0xEAFE1DC);
  volatile u32 conf3 = 0;
  conf3 = MAX2769_CONF3_RESERVED |
          MAX2769_CONF3_GAININ(0b111010) |
          MAX2769_CONF3_FSLOWEN |
          MAX2769_CONF3_ADCEN |
          MAX2769_CONF3_DRVEN |
          MAX2769_CONF3_FOFSTEN |
          MAX2769_CONF3_FILTEN |
          MAX2769_CONF3_FHIPEN |
          MAX2769_CONF3_PGAIEN |
          /*MAX2769_CONF3_PGAQEN |*/
          /* STRM stuff was set before but its unused, can leave as zeros. */
          0;


  max2769_write(MAX2769_PLLCONF, 0x9EC0008);
  volatile u32 pllconf = 0;
  pllconf = MAX2769_PLLCONF_RESERVED |
            MAX2769_PLLCONF_VCOEN |
            MAX2769_PLLCONF_REFOUTEN |
            MAX2769_PLLCONF_REFDIV_DIV_NONE |
            MAX2769_PLLCONF_IXTAL_BUFF_NORMAL |
            MAX2769_PLLCONF_XTALCAP(0b10000) |
            MAX2769_PLLCONF_LDMUX(0) |
            MAX2769_PLLCONF_ICP_0_5MA |
            MAX2769_PLLCONF_CPTEST(0) |
            MAX2769_PLLCONF_INTPLL;

  max2769_write(MAX2769_DIV,
    MAX2769_DIV_NDIV(1536) |
    MAX2769_DIV_RDIV(16)
  );
  max2769_write(MAX2769_FDIV,
    MAX2769_FDIV_RESERVED |
    MAX2769_FDIV_FDIV(0x80000)
  );
  /*max2769_write(MAX2769_STRM,*/
    /*MAX2769_STRM_FRAMECOUNT(0x8000000)*/
  /*);*/
  max2769_write(MAX2769_CLK,
    MAX2769_CLK_L_CNT(256) |
    MAX2769_CLK_M_CNT(1563) |
    MAX2769_CLK_SERCLK
  );
}
