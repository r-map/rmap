/**
  ******************************************************************************
  * @file    AT25SF161.h (Flash Register and Config 2_MByte)
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   This file contains all the description of the AT25SF161 QSPI memory.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Stimav4 is Copyright (C) 2023 ARPAE-SIMC urpsim@arpae.it</center></h2>
  * <h2><center>All rights reserved.</center></h2>
  *
  * This program is free software; you can redistribute it and/or
  * modify it under the terms of the GNU General Public License
  * as published by the Free Software Foundation; either version 2
  * of the License, or (at your option) any later version.
  * 
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  * 
  * You should have received a copy of the GNU General Public License
  * along with this program; if not, write to the Free Software
  * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
  * <http://www.gnu.org/licenses/>.
  * 
  ******************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __AT25SF161_H
#define __AT25SF161_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/**
  * @brief  AT25SF161 Configuration
  */
#define AT25SF161_FLASH_SIZE                  0x200000 	/* 16 MBits => 2MBytes */
#define AT25SF161_SECTOR64_SIZE								0x10000		/* 32 sectors of 64KBytes */
#define AT25SF161_SECTOR_SIZE                 0x8000   	/* 64 sectors of 32KBytes */
#define AT25SF161_BLOCK_SIZE 			            0x1000    /* 512 blocks of 4kBytes */
#define AT25SF161_PAGE_SIZE                   0x100     /* 8192 pages of 256 bytes */

#define AT25SF161_DUMMY_CYCLES_READ           1
#define AT25SF161_DUMMY_CYCLES_READ_QUAD      4

#define AT25SF161_BULK_ERASE_MAX_TIME         25000
#define AT25SF161_SECTOR_ERASE_MAX_TIME       3000
#define AT25SF161_BLOCK_ERASE_MAX_TIME    		300

/**
  * @brief  AT25SF161 Commands
  */
/* Reset Operations */
#ifndef __AT25SF161_H
#define RESET_ENABLE_CMD                     0x66
#define RESET_MEMORY_CMD                     0x99

/* Identification Commands */
#define READ_ID_CMD                          0x9E
#define READ_ID_CMD2                         0x9F
#define MULTIPLE_IO_READ_ID_CMD              0xAF
#define READ_SERIAL_FLASH_DISCO_PARAM_CMD    0x5A
#endif

/* Read Commands */
#define READ_CMD                             0x03
#define FAST_READ_CMD                        0x0B
#define DUAL_OUT_FAST_READ_CMD               0x3B
#define DUAL_INOUT_FAST_READ_CMD             0xBB
#define QUAD_OUT_FAST_READ_CMD               0x6B
#define QUAD_INOUT_FAST_READ_CMD             0xEB
#define QUAD_CONTINUOUS_READ_MODE_RESET			 0xFF

/* Protection Commands */
#define WRITE_ENABLE_CMD                     0x06
#define WRITE_DISABLE_CMD                    0x04

/* Status Register Commands */
#define READ_STATUS_REG_CMD                  0x05
#define READ_STATUS2_REG_CMD                 0x35
#define WRITE_STATUS_REG_CMD                 0x01
#define WRITE_EN_VOLAT_STATUS_REG_CMD				 0x50

/* Program Command */
#define PAGE_PROG_CMD                        0x02

/* Erase Commands */
#define BLOCK_ERASE_CMD                  		 0x20
#define SECTOR_ERASE_CMD                     0x52
#define SECTOR64_ERASE_CMD                   0xD8
#define BULK_ERASE_CMD                       0xC7
#define PROG_ERASE_RESUME_CMD                0x7A
#define PROG_ERASE_SUSPEND_CMD               0x75

/* Security Commands */
#define READ_SEC_REG_PAGE_CMD								 0x48
#define WRITE_SEC_REG_PAGE_CMD               0x42

/* Miscellaneous Commands */
#define READ_ID_CMD                          0x90
#define READ_ID_CMD2                         0x9F
#define DEEP_POWER_DOWN_CMD									 0xB9
#define RESUME_FROM_DEEP_PWD_CMD						 0xAB


/**
  * @brief  AT25SF161 Registers
  */
/* Status Register byte 1 (command READ_STATUS_REG_CMD) */
#define AT25SF161_SR_BUSY                     ((uint32_t)0x0001)  /*!< Device busy */
#define AT25SF161_SR_WEL	                    ((uint32_t)0x0002)  /*!< Write enable latch */
#define AT25SF161_SR_BLOCKPR                  ((uint32_t)0x005C)  /*!< Block protected against program and erase operations */
#define AT25SF161_SR_PRBOTTOM                 ((uint32_t)0x0020)  /*!< Protected memory area defined by BLOCKPR starts from top or bottom */
#define AT25SF161_SR_SRP0		                  ((uint32_t)0x0080)  /*!< Status register protection bit 0 */
/* Status Register byte 2 (command READ_STATUS2_REG_CMD) */
#define AT25SF161_SR_SRP1		                  ((uint32_t)0x0100)  /*!< Status register protection bit 1 */
#define AT25SF161_SR_QE			                  ((uint32_t)0x0200)  /*!< Quad Enable */
#define AT25SF161_SR_LB			                  ((uint32_t)0x3800)  /*!< Lock security register */
#define AT25SF161_SR_CMP		                  ((uint32_t)0x4000)  /*!< Complement Block Protection */
#define AT25SF161_FS_ERSUS                   	((uint32_t)0x8000)  /*!< Erase operation suspended */

#ifdef __cplusplus
}
#endif

#endif /* __AT25SF161_H */