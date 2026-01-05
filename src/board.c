#include "board.h"
#include <xc.h>
#include <sys/attribs.h>

// =====================
// API
// =====================
void board_init(void)
{
    DDPCONbits.JTAGEN = 0;
#ifdef AD1PCFG
    AD1PCFG = 0xFFFF;
#endif
    
    __builtin_disable_interrupts();

    /* Multi-vector mode ON (necessario per __ISR) */
    INTCONbits.MVEC = 1;

    /* init periferiche varie... */

    board_int4_btnc_init();

    __builtin_enable_interrupts();
}

/* flag software settato dall'ISR */
static volatile bool g_btnc_int4_flag = false;

void board_int4_btnc_init(void)
{
    /* BTNC = RF0 (Basys MX3) */
    TRISFbits.TRISF0 = 1;   // input  :contentReference[oaicite:4]{index=4}

    /* Mappa INT4 su RPF0:
       INT4R<3:0> = 0100 -> RPF0  :contentReference[oaicite:5]{index=5} */
    INT4R = 0x4;

    /* Edge polarity:
       INT4EP=1 -> interrupt su fronte negativo (high->low)
       Se non ti triggera, prova INT4EP=0 (low->high). */
    INTCONbits.INT4EP = 1;

    /* Priority/subpriority (scegli tu; qui 5/0) */
    IPC4bits.INT4IP = 5;
    IPC4bits.INT4IS = 0;

    /* Clear flag + enable */
    IFS0CLR = _IFS0_INT4IF_MASK;
    IEC0SET = _IEC0_INT4IE_MASK;
}

bool board_int4_btnc_fired(void)
{
    return g_btnc_int4_flag;
}

void board_int4_btnc_clear(void)
{
    g_btnc_int4_flag = false;
}

/* ISR INT4 */
void __ISR(_EXTERNAL_4_VECTOR, IPL5SOFT) isr_int4_btnc(void)
{
    /* clear flag hw */
    IFS0CLR = _IFS0_INT4IF_MASK;

    /* set flag sw (NO logica pesante qui dentro) */
    g_btnc_int4_flag = true;
}



