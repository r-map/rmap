/* u8g2_d_setup.c */
/* generated code, codebuild, u8g2 project */

#include "u8g2.h"

/* sh1108 f */
void u8g2_Setup_sh1108_i2c_128x160_f(u8g2_t *u8g2, const u8g2_cb_t *rotation, u8x8_msg_cb byte_cb, u8x8_msg_cb gpio_and_delay_cb)
{
  uint8_t tile_buf_height;
  uint8_t *buf;
  u8g2_SetupDisplay(u8g2, u8x8_d_sh1108_128x160, u8x8_cad_ssd13xx_i2c, byte_cb, gpio_and_delay_cb);
  buf = u8g2_m_16_20_f(&tile_buf_height);
  u8g2_SetupBuffer(u8g2, buf, tile_buf_height, u8g2_ll_hvline_vertical_top_lsb, rotation);
}
/* sh1108 */
/* end of generated code */
