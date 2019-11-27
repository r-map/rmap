#ifndef _INTERRUPT_H
#define _INTERRUPT_H
#ifdef    __cplusplus
extern "C" {
#endif
void interrupt_handler_high(void);
void interrupt_handler_low(void);
void RESET_INTERRUPT_FLAG(void);
#ifdef    __cplusplus
}
#endif
#endif

