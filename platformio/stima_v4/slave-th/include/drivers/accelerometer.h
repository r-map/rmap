/**
  ******************************************************************************
  * @file    accelerometer.h
  * @author  Moreno Gasperini <m.gasperini@digiteco.it>
  * @brief   accelerometer IIS328DQ header file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (C) 2022  Moreno Gasperini <m.gasperini@digiteco.it>
  * All rights reserved.</center></h2>
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
#ifndef _ACCELEROMETER_H
#define _ACCELEROMETER_H


/* Includes ------------------------------------------------------------------*/
#include <STM32FreeRTOS.h>
#include "ticks.hpp"
#include "thread.hpp"
#include "semaphore.hpp"
#include "Wire.h"

using namespace cpp_freertos;

/** IIS328DQ Accelerometer
  * I2C Device Address 8 bit format  depends from BIT SA0 -> 0x18 - 0x19 */
#define ACCELEROMETER_IIS328DQ_I2C_ADDR_DEFAULT     (0x19)
#define ACCELEROMETER_WAIT_CHECK_HARDWARE           (5)
#define ACCELEROMETER_MAX_CHECK_ATTEMPT             (5)
/** Device Identification (Who am I) **/
#define IIS328DQ_ID            0x32

/** BIT Mode definitions */
#define DRV_LITTLE_ENDIAN   __ORDER_LITTLE_ENDIAN__
#define DRV_BIG_ENDIAN      __ORDER_BIG_ENDIAN__
#define DRV_BYTE_ORDER      __BYTE_ORDER__
#define PROPERTY_DISABLE    (0U)
#define PROPERTY_ENABLE     (1U)

#define ARR_REG_FILTER      10

// Class Accelerometer implementation
class Accelerometer {

public:

  // ********* Typedef Structure and register ******** //

  // Local COORD for gestion raw_data
  enum coordinate {
    X,
    Y,
    Z
  };

  typedef struct
  {
    #if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t bit0       : 1;
    uint8_t bit1       : 1;
    uint8_t bit2       : 1;
    uint8_t bit3       : 1;
    uint8_t bit4       : 1;
    uint8_t bit5       : 1;
    uint8_t bit6       : 1;
    uint8_t bit7       : 1;
    #elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t bit7       : 1;
    uint8_t bit6       : 1;
    uint8_t bit5       : 1;
    uint8_t bit4       : 1;
    uint8_t bit3       : 1;
    uint8_t bit2       : 1;
    uint8_t bit1       : 1;
    uint8_t bit0       : 1;
    #endif /* DRV_BYTE_ORDER */
  } bitwise_t;

  #define IIS328DQ_WHO_AM_I                  0x0FU
  #define IIS328DQ_CTRL_REG1                 0x20U
  typedef struct
  {
  #if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t xen                      : 1;
    uint8_t yen                      : 1;
    uint8_t zen                      : 1;
    uint8_t dr                       : 2;
    uint8_t pm                       : 3;
  #elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t pm                       : 3;
    uint8_t dr                       : 2;
    uint8_t zen                      : 1;
    uint8_t yen                      : 1;
    uint8_t xen                      : 1;
  #endif /* DRV_BYTE_ORDER */
  } iis328dq_ctrl_reg1_t;

  #define IIS328DQ_CTRL_REG2                 0x21U
  typedef struct
  {
  #if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t hpcf                     : 2;
    uint8_t hpen                     : 2;
    uint8_t fds                      : 1;
    uint8_t hpm                      : 2;
    uint8_t boot                     : 1;
  #elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t boot                     : 1;
    uint8_t hpm                      : 2;
    uint8_t fds                      : 1;
    uint8_t hpen                     : 2;
    uint8_t hpcf                     : 2;
  #endif /* DRV_BYTE_ORDER */
  } iis328dq_ctrl_reg2_t;

  #define IIS328DQ_CTRL_REG3                 0x22U
  typedef struct
  {
  #if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t i1_cfg                   : 2;
    uint8_t lir1                     : 1;
    uint8_t i2_cfg                   : 2;
    uint8_t lir2                     : 1;
    uint8_t pp_od                    : 1;
    uint8_t ihl                      : 1;
  #elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t ihl                      : 1;
    uint8_t pp_od                    : 1;
    uint8_t lir2                     : 1;
    uint8_t i2_cfg                   : 2;
    uint8_t lir1                     : 1;
    uint8_t i1_cfg                   : 2;
  #endif /* DRV_BYTE_ORDER */
  } iis328dq_ctrl_reg3_t;

  #define IIS328DQ_CTRL_REG4                 0x23U
  typedef struct
  {
  #if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t sim                      : 1;
    uint8_t st                       : 3; /* STsign + ST */
    uint8_t fs                       : 2;
    uint8_t ble                      : 1;
    uint8_t bdu                      : 1;
  #elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t bdu                      : 1;
    uint8_t ble                      : 1;
    uint8_t fs                       : 2;
    uint8_t st                       : 3; /* STsign + ST */
    uint8_t sim                      : 1;
  #endif /* DRV_BYTE_ORDER */
  } iis328dq_ctrl_reg4_t;

  #define IIS328DQ_CTRL_REG5                 0x24U
  typedef struct
  {
  #if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t turnon                   : 2;
    uint8_t not_used_01              : 6;
  #elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t not_used_01              : 6;
    uint8_t turnon                   : 2;
  #endif /* DRV_BYTE_ORDER */
  } iis328dq_ctrl_reg5_t;

  #define IIS328DQ_HP_FILTER_RESET           0x25U
  #define IIS328DQ_REFERENCE                 0x26U
  #define IIS328DQ_STATUS_REG                0x27U
  typedef struct
  {
  #if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t xda                      : 1;
    uint8_t yda                      : 1;
    uint8_t zda                      : 1;
    uint8_t zyxda                    : 1;
    uint8_t _xor                     : 1;
    uint8_t yor                      : 1;
    uint8_t zor                      : 1;
    uint8_t zyxor                    : 1;
  #elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t zyxor                    : 1;
    uint8_t zor                      : 1;
    uint8_t yor                      : 1;
    uint8_t _xor                     : 1;
    uint8_t zyxda                    : 1;
    uint8_t zda                      : 1;
    uint8_t yda                      : 1;
    uint8_t xda                      : 1;
  #endif /* DRV_BYTE_ORDER */
  } iis328dq_status_reg_t;

  #define IIS328DQ_OUT_X_L                   0x28U
  #define IIS328DQ_OUT_X_H                   0x29U
  #define IIS328DQ_OUT_Y_L                   0x2AU
  #define IIS328DQ_OUT_Y_H                   0x2BU
  #define IIS328DQ_OUT_Z_L                   0x2CU
  #define IIS328DQ_OUT_Z_H                   0x2DU
  #define IIS328DQ_INT1_CFG                  0x30U
  typedef struct
  {
  #if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t xlie                     : 1;
    uint8_t xhie                     : 1;
    uint8_t ylie                     : 1;
    uint8_t yhie                     : 1;
    uint8_t zlie                     : 1;
    uint8_t zhie                     : 1;
    uint8_t _6d                      : 1;
    uint8_t aoi                      : 1;
  #elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t aoi                      : 1;
    uint8_t _6d                      : 1;
    uint8_t zhie                     : 1;
    uint8_t zlie                     : 1;
    uint8_t yhie                     : 1;
    uint8_t ylie                     : 1;
    uint8_t xhie                     : 1;
    uint8_t xlie                     : 1;
  #endif /* DRV_BYTE_ORDER */
  } iis328dq_int1_cfg_t;

  #define IIS328DQ_INT1_SRC                  0x31U
  typedef struct
  {
  #if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t xl                       : 1;
    uint8_t xh                       : 1;
    uint8_t yl                       : 1;
    uint8_t yh                       : 1;
    uint8_t zl                       : 1;
    uint8_t zh                       : 1;
    uint8_t ia                       : 1;
    uint8_t not_used_01              : 1;
  #elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t not_used_01              : 1;
    uint8_t ia                       : 1;
    uint8_t zh                       : 1;
    uint8_t zl                       : 1;
    uint8_t yh                       : 1;
    uint8_t yl                       : 1;
    uint8_t xh                       : 1;
    uint8_t xl                       : 1;
  #endif /* DRV_BYTE_ORDER */
  } iis328dq_int1_src_t;

  #define IIS328DQ_INT1_THS                  0x32U
  typedef struct
  {
  #if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t ths                      : 7;
    uint8_t not_used_01              : 1;
  #elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t not_used_01              : 1;
    uint8_t ths                      : 7;
  #endif /* DRV_BYTE_ORDER */
  } iis328dq_int1_ths_t;

  #define IIS328DQ_INT1_DURATION             0x33U
  typedef struct
  {
  #if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t d                        : 7;
    uint8_t not_used_01              : 1;
  #elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t not_used_01              : 1;
    uint8_t d                        : 7;
  #endif /* DRV_BYTE_ORDER */
  } iis328dq_int1_duration_t;

  #define IIS328DQ_INT2_CFG                  0x34U
  typedef struct
  {
  #if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t xlie                     : 1;
    uint8_t xhie                     : 1;
    uint8_t ylie                     : 1;
    uint8_t yhie                     : 1;
    uint8_t zlie                     : 1;
    uint8_t zhie                     : 1;
    uint8_t _6d                      : 1;
    uint8_t aoi                      : 1;
  #elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t aoi                      : 1;
    uint8_t _6d                      : 1;
    uint8_t zhie                     : 1;
    uint8_t zlie                     : 1;
    uint8_t yhie                     : 1;
    uint8_t ylie                     : 1;
    uint8_t xhie                     : 1;
    uint8_t xlie                     : 1;
  #endif /* DRV_BYTE_ORDER */
  } iis328dq_int2_cfg_t;

  #define IIS328DQ_INT2_SRC                  0x35U
  typedef struct
  {
  #if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t xl                       : 1;
    uint8_t xh                       : 1;
    uint8_t yl                       : 1;
    uint8_t yh                       : 1;
    uint8_t zl                       : 1;
    uint8_t zh                       : 1;
    uint8_t ia                       : 1;
    uint8_t not_used_01              : 1;
  #elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t not_used_01              : 1;
    uint8_t ia                       : 1;
    uint8_t zh                       : 1;
    uint8_t zl                       : 1;
    uint8_t yh                       : 1;
    uint8_t yl                       : 1;
    uint8_t xh                       : 1;
    uint8_t xl                       : 1;
  #endif /* DRV_BYTE_ORDER */
  } iis328dq_int2_src_t;

  #define IIS328DQ_INT2_THS                  0x36U
  typedef struct
  {
  #if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t ths                      : 7;
    uint8_t not_used_01              : 1;
  #elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t not_used_01              : 1;
    uint8_t ths                      : 7;
  #endif /* DRV_BYTE_ORDER */
  } iis328dq_int2_ths_t;

  #define IIS328DQ_INT2_DURATION             0x37U
  typedef struct
  {
  #if DRV_BYTE_ORDER == DRV_LITTLE_ENDIAN
    uint8_t d                        : 7;
    uint8_t not_used_01              : 1;
  #elif DRV_BYTE_ORDER == DRV_BIG_ENDIAN
    uint8_t not_used_01              : 1;
    uint8_t d                        : 7;
  #endif /* DRV_BYTE_ORDER */
  } iis328dq_int2_duration_t;

  // Accelerometer IIS328DQ_Register_Union
  typedef union
  {
    iis328dq_ctrl_reg1_t                     ctrl_reg1;
    iis328dq_ctrl_reg2_t                     ctrl_reg2;
    iis328dq_ctrl_reg3_t                     ctrl_reg3;
    iis328dq_ctrl_reg4_t                     ctrl_reg4;
    iis328dq_ctrl_reg5_t                     ctrl_reg5;
    iis328dq_status_reg_t                    status_reg;
    iis328dq_int1_cfg_t                      int1_cfg;
    iis328dq_int1_src_t                      int1_src;
    iis328dq_int1_ths_t                      int1_ths;
    iis328dq_int1_duration_t                 int1_duration;
    iis328dq_int2_cfg_t                      int2_cfg;
    iis328dq_int2_src_t                      int2_src;
    iis328dq_int2_ths_t                      int2_ths;
    iis328dq_int2_duration_t                 int2_duration;
    bitwise_t                                bitwise;
    uint8_t                                  byte;
  } iis328dq_reg_t;

  // *************** Class Constructor *************** //

  Accelerometer();
  Accelerometer(TwoWire *_wire, BinarySemaphore *_wireLock, uint8_t _i2c_address = ACCELEROMETER_IIS328DQ_I2C_ADDR_DEFAULT);
  
  // ************ Library implemenmtation ************ //

  int32_t iis328dq_read_reg(uint8_t reg, uint8_t *data, uint16_t len);
  int32_t iis328dq_write_reg(uint8_t reg, uint8_t *data, uint16_t len);

  void push_raw_data(int16_t *data_raw);
  int16_t get_raw_mean(coordinate request);

  float_t iis328dq_from_fs2_to_mg(int16_t lsb);
  float_t iis328dq_from_fs2_to_mg(coordinate request);
  float_t iis328dq_from_fs4_to_mg(int16_t lsb);
  float_t iis328dq_from_fs4_to_mg(coordinate request);
  float_t iis328dq_from_fs8_to_mg(int16_t lsb);
  float_t iis328dq_from_fs8_to_mg(coordinate request);

  float_t iis328dq_from_fsx_to_inc(int16_t lsb);
  float_t iis328dq_from_fsx_to_inc(coordinate request);

  int32_t iis328dq_axis_x_data_set(uint8_t val);
  int32_t iis328dq_axis_x_data_get(uint8_t *val);

  int32_t iis328dq_axis_y_data_set(uint8_t val);
  int32_t iis328dq_axis_y_data_get(uint8_t *val);

  int32_t iis328dq_axis_z_data_set(uint8_t val);
  int32_t iis328dq_axis_z_data_get(uint8_t *val);

  typedef enum
  {
    IIS328DQ_ODR_OFF   = 0x00,
    IIS328DQ_ODR_ON    = 0x01,
    IIS328DQ_ODR_Hz5   = 0x02,
    IIS328DQ_ODR_1Hz   = 0x03,
    IIS328DQ_ODR_5Hz2  = 0x04,
    IIS328DQ_ODR_5Hz   = 0x05,
    IIS328DQ_ODR_10Hz  = 0x06,
    IIS328DQ_ODR_50Hz  = 0x01,
    IIS328DQ_ODR_100Hz = 0x11,
    IIS328DQ_ODR_400Hz = 0x21,
    IIS328DQ_ODR_1kHz  = 0x31,
  } iis328dq_dr_t;
  int32_t iis328dq_data_rate_set(iis328dq_dr_t val);
  int32_t iis328dq_data_rate_get(iis328dq_dr_t *val);

  typedef enum
  {
    IIS328DQ_NORMAL_MODE      = 0,
    IIS328DQ_REF_MODE_ENABLE  = 1,
  } iis328dq_hpm_t;
  int32_t iis328dq_reference_mode_set(iis328dq_hpm_t val);
  int32_t iis328dq_reference_mode_get(iis328dq_hpm_t *val);

  typedef enum
  {
    IIS328DQ_2g  = 0,
    IIS328DQ_4g  = 1,
    IIS328DQ_8g  = 3,
  } iis328dq_fs_t;
  int32_t iis328dq_full_scale_set(iis328dq_fs_t val);
  int32_t iis328dq_full_scale_get(iis328dq_fs_t *val);

  int32_t iis328dq_block_data_update_set(uint8_t val);
  int32_t iis328dq_block_data_update_get(uint8_t *val);

  int32_t iis328dq_status_reg_get(iis328dq_status_reg_t *val);

  int32_t iis328dq_flag_data_ready_get(uint8_t *val);

  int32_t iis328dq_acceleration_raw_get(int16_t *val);

  int32_t iis328dq_device_id_get(uint8_t *buff);

  int32_t iis328dq_boot_set(uint8_t val);
  int32_t iis328dq_boot_get(uint8_t *val);

  typedef enum
  {
    IIS328DQ_ST_DISABLE   = 0,
    IIS328DQ_ST_POSITIVE  = 1,
    IIS328DQ_ST_NEGATIVE  = 5,
  } iis328dq_st_t;
  int32_t iis328dq_self_test_set(iis328dq_st_t val);
  int32_t iis328dq_self_test_get(iis328dq_st_t *val);

  typedef enum
  {
    IIS328DQ_LSB_AT_LOW_ADD  = 0,
    IIS328DQ_MSB_AT_LOW_ADD  = 1,
  } iis328dq_ble_t;
  int32_t iis328dq_data_format_set(iis328dq_ble_t val);
  int32_t iis328dq_data_format_get(iis328dq_ble_t *val);

  typedef enum
  {
    IIS328DQ_CUT_OFF_8Hz   = 0,
    IIS328DQ_CUT_OFF_16Hz  = 1,
    IIS328DQ_CUT_OFF_32Hz  = 2,
    IIS328DQ_CUT_OFF_64Hz  = 3,
  } iis328dq_hpcf_t;
  int32_t iis328dq_hp_bandwidth_set(iis328dq_hpcf_t val);
  int32_t iis328dq_hp_bandwidth_get(iis328dq_hpcf_t *val);

  typedef enum
  {
    IIS328DQ_HP_DISABLE            = 0,
    IIS328DQ_HP_ON_OUT             = 4,
    IIS328DQ_HP_ON_INT1            = 1,
    IIS328DQ_HP_ON_INT2            = 2,
    IIS328DQ_HP_ON_INT1_INT2       = 3,
    IIS328DQ_HP_ON_INT1_INT2_OUT   = 7,
    IIS328DQ_HP_ON_INT2_OUT        = 6,
    IIS328DQ_HP_ON_INT1_OUT        = 5,
  } iis328dq_hpen_t;
  int32_t iis328dq_hp_path_set(iis328dq_hpen_t val);
  int32_t iis328dq_hp_path_get(iis328dq_hpen_t *val);

  int32_t iis328dq_hp_reset_get(void);

  int32_t iis328dq_hp_reference_value_set(uint8_t val);
  int32_t iis328dq_hp_reference_value_get(uint8_t *val);

  typedef enum
  {
    IIS328DQ_SPI_4_WIRE  = 0,
    IIS328DQ_SPI_3_WIRE  = 1,
  } iis328dq_sim_t;
  int32_t iis328dq_spi_mode_set(iis328dq_sim_t val);
  int32_t iis328dq_spi_mode_get(iis328dq_sim_t *val);

  typedef enum
  {
    IIS328DQ_PAD1_INT1_SRC           = 0,
    IIS328DQ_PAD1_INT1_OR_INT2_SRC   = 1,
    IIS328DQ_PAD1_DRDY               = 2,
    IIS328DQ_PAD1_BOOT               = 3,
  } iis328dq_i1_cfg_t;
  int32_t iis328dq_pin_int1_route_set(iis328dq_i1_cfg_t val);
  int32_t iis328dq_pin_int1_route_get(iis328dq_i1_cfg_t *val);

  typedef enum
  {
    IIS328DQ_INT1_PULSED   = 0,
    IIS328DQ_INT1_LATCHED  = 1,
  } iis328dq_lir1_t;
  int32_t iis328dq_int1_notification_set(iis328dq_lir1_t val);
  int32_t iis328dq_int1_notification_get(iis328dq_lir1_t *val);

  typedef enum
  {
    IIS328DQ_PAD2_INT2_SRC           = 0,
    IIS328DQ_PAD2_INT1_OR_INT2_SRC   = 1,
    IIS328DQ_PAD2_DRDY               = 2,
    IIS328DQ_PAD2_BOOT               = 3,
  } iis328dq_i2_cfg_t;
  int32_t iis328dq_pin_int2_route_set(iis328dq_i2_cfg_t val);
  int32_t iis328dq_pin_int2_route_get(iis328dq_i2_cfg_t *val);

  typedef enum
  {
    IIS328DQ_INT2_PULSED   = 0,
    IIS328DQ_INT2_LATCHED  = 1,
  } iis328dq_lir2_t;
  int32_t iis328dq_int2_notification_set(iis328dq_lir2_t val);
  int32_t iis328dq_int2_notification_get(iis328dq_lir2_t *val);

  typedef enum
  {
    IIS328DQ_PUSH_PULL   = 0,
    IIS328DQ_OPEN_DRAIN  = 1,
  } iis328dq_pp_od_t;
  int32_t iis328dq_pin_mode_set(iis328dq_pp_od_t val);
  int32_t iis328dq_pin_mode_get(iis328dq_pp_od_t *val);

  typedef enum
  {
    IIS328DQ_ACTIVE_HIGH  = 0,
    IIS328DQ_ACTIVE_LOW   = 1,
  } iis328dq_ihl_t;
  int32_t iis328dq_pin_polarity_set(iis328dq_ihl_t val);
  int32_t iis328dq_pin_polarity_get(iis328dq_ihl_t *val);

  typedef struct
  {
    uint8_t int1_xlie             : 1;
    uint8_t int1_xhie             : 1;
    uint8_t int1_ylie             : 1;
    uint8_t int1_yhie             : 1;
    uint8_t int1_zlie             : 1;
    uint8_t int1_zhie             : 1;
  } int1_on_th_conf_t;
  int32_t iis328dq_int1_on_threshold_conf_set(int1_on_th_conf_t val);
  int32_t iis328dq_int1_on_threshold_conf_get(int1_on_th_conf_t *val);

  typedef enum
  {
    IIS328DQ_INT1_ON_THRESHOLD_OR   = 0,
    IIS328DQ_INT1_ON_THRESHOLD_AND  = 1,
  } iis328dq_int1_aoi_t;
  int32_t iis328dq_int1_on_threshold_mode_set(iis328dq_int1_aoi_t val);
  int32_t iis328dq_int1_on_threshold_mode_get(iis328dq_int1_aoi_t *val);

  int32_t iis328dq_int1_src_get(iis328dq_int1_src_t *val);

  int32_t iis328dq_int1_treshold_set(uint8_t val);
  int32_t iis328dq_int1_treshold_get(uint8_t *val);

  int32_t iis328dq_int1_dur_set(uint8_t val);
  int32_t iis328dq_int1_dur_get(uint8_t *val);

  typedef struct
  {
    uint8_t int2_xlie             : 1;
    uint8_t int2_xhie             : 1;
    uint8_t int2_ylie             : 1;
    uint8_t int2_yhie             : 1;
    uint8_t int2_zlie             : 1;
    uint8_t int2_zhie             : 1;
  } int2_on_th_conf_t;
  int32_t iis328dq_int2_on_threshold_conf_set(int2_on_th_conf_t val);
  int32_t iis328dq_int2_on_threshold_conf_get(int2_on_th_conf_t *val);

  typedef enum
  {
    IIS328DQ_INT2_ON_THRESHOLD_OR   = 0,
    IIS328DQ_INT2_ON_THRESHOLD_AND  = 1,
  } iis328dq_int2_aoi_t;
  int32_t iis328dq_int2_on_threshold_mode_set(iis328dq_int2_aoi_t val);
  int32_t iis328dq_int2_on_threshold_mode_get(iis328dq_int2_aoi_t *val);

  int32_t iis328dq_int2_src_get(iis328dq_int2_src_t *val);

  int32_t iis328dq_int2_treshold_set(uint8_t val);
  int32_t iis328dq_int2_treshold_get(uint8_t *val);

  int32_t iis328dq_int2_dur_set(uint8_t val);
  int32_t iis328dq_int2_dur_get(uint8_t *val);

  int32_t iis328dq_wkup_to_sleep_set(uint8_t val);
  int32_t iis328dq_wkup_to_sleep_get(uint8_t *val);

  typedef enum
  {
    IIS328DQ_6D_INT1_DISABLE   = 0,
    IIS328DQ_6D_INT1_MOVEMENT  = 1,
    IIS328DQ_6D_INT1_POSITION  = 3,
  } iis328dq_int1_6d_t;
  int32_t iis328dq_int1_6d_mode_set(iis328dq_int1_6d_t val);
  int32_t iis328dq_int1_6d_mode_get(iis328dq_int1_6d_t *val);

  int32_t iis328dq_int1_6d_src_get(iis328dq_int1_src_t *val);

  int32_t iis328dq_int1_6d_treshold_set(uint8_t val);
  int32_t iis328dq_int1_6d_treshold_get(uint8_t *val);

  typedef enum
  {
    IIS328DQ_6D_INT2_DISABLE   = 0,
    IIS328DQ_6D_INT2_MOVEMENT  = 1,
    IIS328DQ_6D_INT2_POSITION  = 3,
  } iis328dq_int2_6d_t;
  int32_t iis328dq_int2_6d_mode_set(iis328dq_int2_6d_t val);
  int32_t iis328dq_int2_6d_mode_get(iis328dq_int2_6d_t *val);

  int32_t iis328dq_int2_6d_src_get(iis328dq_int2_src_t *val);

  int32_t iis328dq_int2_6d_treshold_set(uint8_t val);
  int32_t iis328dq_int2_6d_treshold_get(uint8_t *val);

  // ********** Private Variables Semaphore ********** //

protected:
private:
  TwoWire *_wire;
  BinarySemaphore *_wireLock;
  uint8_t _i2c_address;
  int16_t _raw_scroll[3][ARR_REG_FILTER];
};

typedef struct
{
  uint8_t config_valid;                      //!< Inizialization Byte Config
  Accelerometer::iis328dq_dr_t module_power; //!< module updating frequency (enabled)
  float offset_x;                            //!< offset_x to 0
  float offset_y;                            //!< offset_y to 0
  float offset_z;                            //!< offset_z to 0
} accelerometer_t;

#endif /* _ACCELEROMETR_H */

/************************ (C) COPYRIGHT Digiteco s.r.l. *****END OF FILE****/
