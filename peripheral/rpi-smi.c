#include <stdint.h>
#include <stdio.h>
#include "rpi-smi.h"
#include "rpi-base.h"
#include "rpi-gpio.h"




static inline void write_smi_reg(uint32_t val, rpi_reg_rw_t reg_offset)
{
    rpi_reg_rw_t *target_reg = (uint32_t*) SMI_BASE_ADDRESS + reg_offset/4;
    *target_reg = val;
}

static inline uint32_t read_smi_reg(rpi_reg_rw_t reg_offset)
{
    rpi_reg_rw_t *target_reg = (uint32_t*) SMI_BASE_ADDRESS + reg_offset/4;
    uint32_t reg_value = (uint32_t) *target_reg;
    return reg_value;
}

static inline void write_cm_smi_reg(uint32_t val, rpi_reg_rw_t reg_offset)
{
    rpi_reg_rw_t *target_reg = (uint32_t*) CM_SMI_BASE_ADDRESS  + reg_offset/4;
    *target_reg = val;
}

static inline uint32_t read_cm_smi_reg(rpi_reg_rw_t reg_offset)
{
    rpi_reg_rw_t *target_reg = (uint32_t*) CM_SMI_BASE_ADDRESS + reg_offset/4;
    uint32_t reg_value = (uint32_t) *target_reg;
    return reg_value;
}
static inline rpi_reg_rw_t *get_reg(uint32_t reg_offset)
{
    return (rpi_reg_rw_t *) CM_SMI_BASE_ADDRESS + reg_offset/4;
}

#define _CONCAT(x, y) x##y
#define CONCAT(x, y) _CONCAT(x, y)

#define SET_BIT_FIELD(dest, field, bits) ((dest) = \
    ((dest) & ~CONCAT(field, _MASK)) | (((bits) << CONCAT(field, _OFFS))& \
     CONCAT(field, _MASK)))
#define GET_BIT_FIELD(src, field) (((src) & \
    CONCAT(field, _MASK)) >> CONCAT(field, _OFFS))

void smi_dump_context_labelled()
{
    printf("SMI context dump: \r\n");
    printf("CM_SMI_CTL:  0x%08x \r\n", read_cm_smi_reg(CM_SMI_CTL));
    printf("CM_SMI_DIV:  0x%08x \r\n", read_cm_smi_reg(CM_SMI_DIV));
    printf("SMICS:  0x%08x \r\n", read_smi_reg(SMICS));
    printf("SMIL:   0x%08x \r\n", read_smi_reg(SMIL));
    printf("SMIDSR: 0x%08x \r\n", read_smi_reg(SMIDSR0));
    printf("SMIDSW: 0x%08x \r\n", read_smi_reg(SMIDSW0));
    printf("SMIDC:  0x%08x \r\n", read_smi_reg(SMIDC));
    printf("SMIFD:  0x%08x \r\n", read_smi_reg(SMIFD));
    printf(" \r\n");
    RPI_WaitMicroSeconds(200000);
}

void smi_set_default_settings(struct smi_settings *settings)
{

//smidsw_temp = 0x3f3f7f7f; //   < ----- select this
/* Device write settings:
 * WIDTH	: Write transfer width. 00 = 8bit, 01 = 16bit,
 *			  10= 18bit, 11 = 9bit.
 * SETUP	: Number of cycles between CS assert and write strobe.
 *			  Min 1, max 64.
 * FORMAT	: Pixel format of input. 0 = 16bit RGB 565,
 *			  1 = 32bit RGBA 8888
 * SWAP		: 1 = swap pixel data bits. (Use with SMICS_PXLDAT)
 * HOLD		: Time between WE deassert and CS deassert. 1 to 64
 * PACEALL	: 1: this device's WPACE will be used for the next
 *			  transfer, regardless of that transfer's device.
 * PACE		: Cycles between CS deassert and next CS assert.
 *			  Min 1, max 128
 * DREQ		: Use external DREQ on pin 17 to pace writes. DMAP must
 *			  be set in SMICS.
 * STROBE	: Number of cycles to assert the write strobe.
 *			  Min 1, max 128
 */

//  31    30   29 28 27 26 25 24      23        22      21 20 19 18 17 16
//  0     0    1  1  1  1  1  1       0         0       1  1  1  1  1  1
//  |WIDTH|    |     SETUP     |   |FORMAT|   |SWAP|   |      HOLD      |

//  15        14 13 12 11 10 9 8       7         6 5 4 3 2 1 0
//  0         1  1  1  1  1  1 1       0         1 1 1 1 1 1 1
// |PACEALL|  |     PACE       |     |DREQ|      |  STROBE   |

/* Timing for writes
 *
 * OE ----------+	   +--------------------
 *		|	   |
 *		+----------+
 * SD -<==============================>-----------
 * SA -<=========================================>-
 *    <-setup->  <-strobe ->  <-hold ->  <- pace ->
 */


    settings->data_width = SMI_WIDTH_8BIT;//SMI_WIDTH_16BIT;
    settings->pack_data = false;          //true;
    settings->pixels_format = 0;
    settings->pixels_swap = false;
    settings->data_request = false;
    settings->read_setup_time = 0x3F;     //1;
    settings->read_hold_time = 0x3F;      //1;
    settings->read_pace_time = 0x3F;      //1;
    settings->read_strobe_time = 0x3F;    //3;

    settings->write_setup_time = settings->read_setup_time;
    settings->write_hold_time = settings->read_hold_time;
    settings->write_pace_time = settings->read_pace_time;
    settings->write_strobe_time = settings->read_strobe_time;

     //still not dma...
    settings->dma_enable = false;

//	settings->dma_enable = true;
//	settings->dma_passthrough_enable = false;
//	settings->dma_read_thresh = 0x01;
//	settings->dma_write_thresh = 0x3f;
//	settings->dma_panic_read_thresh = 0x20;
//	settings->dma_panic_write_thresh = 0x20;
}

static void smi_set_regs_from_settings(struct smi_settings *settings)
{
    int smidsr_temp = 0, smidsw_temp = 0, smics_temp,
        smidcs_temp, smidc_temp = 0;

    //spin_lock(&inst->transaction_lock);

    /* temporarily disable the peripheral: */
    smics_temp = read_smi_reg(SMICS);
    write_smi_reg( 0, SMICS);
    smidcs_temp = read_smi_reg(SMIDCS);
    write_smi_reg(0, SMIDCS);

    if (settings->pack_data)        smics_temp |= SMICS_PXLDAT;
    else        smics_temp &= ~SMICS_PXLDAT;

    //Read regs
    if (settings->pixels_swap)        smidsw_temp |= SMIDSW_WSWAP;
    else        smidsw_temp &= ~SMIDSW_WSWAP;

    if (settings->data_request)        smidsr_temp |= SMIDSW_WDREQ;
    else        smidsr_temp &= ~SMIDSW_WDREQ;

    smidsr_temp |= settings->pixels_format;
    SET_BIT_FIELD(smidsr_temp, SMIDSR_RWIDTH, settings->data_width);
    SET_BIT_FIELD(smidsr_temp, SMIDSR_RSETUP, settings->read_setup_time);
    SET_BIT_FIELD(smidsr_temp, SMIDSR_RHOLD, settings->read_hold_time);
    SET_BIT_FIELD(smidsr_temp, SMIDSR_RPACE, settings->read_pace_time);
    SET_BIT_FIELD(smidsr_temp, SMIDSR_RSTROBE, settings->read_strobe_time);
    write_smi_reg(smidsr_temp, SMIDSR0);
    write_smi_reg(smidsr_temp, SMIDSR1);
    write_smi_reg(smidsr_temp, SMIDSR2);
    write_smi_reg(smidsr_temp, SMIDSR3);

   //Write regs
    if (settings->pixels_swap)        smidsw_temp |= SMIDSW_WSWAP;
    else        smidsw_temp &= ~SMIDSW_WSWAP;

    if (settings->data_request)        smidsw_temp |= SMIDSW_WDREQ;
    else        smidsw_temp &= ~SMIDSW_WDREQ;

    smidsw_temp |= settings->pixels_format;
    SET_BIT_FIELD(smidsw_temp, SMIDSW_WWIDTH, settings->data_width);
    SET_BIT_FIELD(smidsw_temp, SMIDSW_WSETUP, settings->write_setup_time);
    SET_BIT_FIELD(smidsw_temp, SMIDSW_WHOLD, settings->write_hold_time);
    SET_BIT_FIELD(smidsw_temp, SMIDSW_WPACE, settings->write_pace_time);
    SET_BIT_FIELD(smidsw_temp, SMIDSW_WSTROBE, settings->write_strobe_time);
    write_smi_reg(smidsw_temp, SMIDSW0);
    write_smi_reg(smidsw_temp, SMIDSW1);
    write_smi_reg(smidsw_temp, SMIDSW2);
    write_smi_reg(smidsw_temp, SMIDSW3);



    //still not dma...
//    SET_BIT_FIELD(smidc_temp, SMIDC_REQR, settings->dma_read_thresh);
//    SET_BIT_FIELD(smidc_temp, SMIDC_REQW, settings->dma_write_thresh);
//    SET_BIT_FIELD(smidc_temp, SMIDC_PANICR,
//              settings->dma_panic_read_thresh);
//    SET_BIT_FIELD(smidc_temp, SMIDC_PANICW,
//              settings->dma_panic_write_thresh);
//    if (settings->dma_passthrough_enable) {
//        smidc_temp |= SMIDC_DMAP;
//        smidsr_temp |= SMIDSR_RDREQ;
//        write_smi_reg(inst, smidsr_temp, SMIDSR0);
//        smidsw_temp |= SMIDSW_WDREQ;
//        write_smi_reg(inst, smidsw_temp, SMIDSW0);
//    } else
//        smidc_temp &= ~SMIDC_DMAP;
   if (settings->dma_enable)
        smidc_temp |= SMIDC_DMAEN;
    else
        smidc_temp &= ~SMIDC_DMAEN;
    write_smi_reg(smidc_temp, SMIDC);

    /* re-enable (if was previously enabled) */
   write_smi_reg(smics_temp, SMICS);
   write_smi_reg(smidcs_temp, SMIDCS);

//	spin_unlock(&inst->transaction_lock);
}

inline void smi_set_address(unsigned int address)
{
    int smia_temp = 0, smida_temp = 0;

    SET_BIT_FIELD(smia_temp, SMIA_ADDR, address);
    SET_BIT_FIELD(smida_temp, SMIDA_ADDR, address);

    /* Write to both address registers - user doesn't care whether we're
       doing programmed or direct transfers. */
    write_smi_reg(smia_temp, SMIA);
    write_smi_reg(smida_temp, SMIDA);
}


void smi_setup(struct smi_settings *settings)
{

//    dev_dbg(inst->dev, "Initialising SMI registers...");
    /* Disable the peripheral if already enabled */
    write_smi_reg(0, SMICS);
    write_smi_reg(0, SMIDCS);

    smi_set_regs_from_settings(settings);
    smi_set_address(0);

   write_smi_reg(read_smi_reg(SMICS) | SMICS_ENABLE, SMICS);
  //  write_smi_reg(read_smi_reg(SMIDCS) | SMIDCS_ENABLE, SMIDCS);
}


inline void smi_write_single_word(uint32_t data)
{
    int timeout = 0;
    write_smi_reg(SMIDCS_ENABLE | SMIDCS_WRITE, SMIDCS);
    write_smi_reg(data, SMIDD);
    write_smi_reg(SMIDCS_ENABLE | SMIDCS_WRITE | SMIDCS_START,
        SMIDCS);

    while (!(read_smi_reg(SMIDCS) & SMIDCS_DONE) &&  (++timeout < 10000)) {};

    if (timeout >= 10000)
    {
        printf("SMI direct write timed out (is the clock set up correctly?) \r\n");
    }

}

void smi_write_n_words(uint32_t *data, int n)
{
    volatile unsigned int i, timeout = 0;
    write_smi_reg(SMIDCS_ENABLE | SMIDCS_WRITE, SMIDCS);

    for (i=0;i<n;i++)
    {
//        timeout = 0;
        write_smi_reg(SMIDCS_ENABLE | SMIDCS_WRITE | SMIDCS_START, SMIDCS);
        // printf( "Transmitting: %i  \r\n\n", data[i]);
        write_smi_reg(data[i], SMIDD);

    //   while (!(read_smi_reg(SMIDCS) & SMIDCS_DONE) &&  (++timeout < 10000)) {};
//        if (timeout >= 10000) break;
    }


    if (timeout >= 10000)
    {
        printf("SMI direct write timed out (is the clock set up correctly?) \r\n");
    }

}

/* Initiates a programmed write sequence, using data from the write FIFO.
 * It is up to the caller to initiate a DMA transfer before calling,
 * or use another method to keep the write FIFO topped up.
 * SMICS_ACTIVE will go low upon completion.
 */
static void smi_init_programmed_write(int num_transfers)
{
    int smics_temp, smidcs_temp = 0 ;


    //write_smi_reg(smidcs_temp, SMIDCS);

    /* Disable the peripheral: */
    smics_temp = read_smi_reg(SMICS) & ~SMICS_ENABLE;
    write_smi_reg(smics_temp, SMICS);
    while (read_smi_reg(SMICS) & SMICS_ENABLE)
        ;

    /* Program the transfer count: */
    write_smi_reg(num_transfers, SMIL);
   // printf ("DEBUG: SMIL: 0x%08x\r\n", read_smi_reg(SMIL) );

    /* setup, re-enable and start: */
    smics_temp |= SMICS_WRITE | SMICS_DONE | SMICS_ENABLE;
    write_smi_reg(smics_temp, SMICS);
    smics_temp |= SMICS_START;
    write_smi_reg(smics_temp, SMICS);
}

/* Initiate a write, and then keep the FIFO topped up. */
void smi_write_fifo(uint32_t *src, int n_bytes)
{
    int i, timeout = 0;



    /* Empty FIFOs if not already so */
    if (!(read_smi_reg(SMICS) & SMICS_TXE)) {
        printf("WARNING: write fifo not empty at start of write call.");
        write_smi_reg(read_smi_reg(SMICS) | SMICS_CLEAR, SMICS);
    }

    /* Initiate the transfer */
//    if (settings.data_width == SMI_WIDTH_8BIT)
       smi_init_programmed_write(n_bytes);
//    else if (inst->settings.data_width == SMI_WIDTH_16BIT)
//        smi_init_programmed_write(n_bytes / 2);
//    else {
//        printf("Unsupported data width for write. \r\n ");
//        return;
//    }
    /* Fill the FIFO: */
    for (i = 0; i < (n_bytes - 1) / 4 + 1; ++i) {

        while (!(read_smi_reg(SMICS) & SMICS_TXD))
            ;
        write_smi_reg(*src++, SMID);
       //  global_counter++;
        //printf ("DEBUG: Stored: %i words in FIFO \r\n", i );
    }
    /* Busy wait... */
/*    while (!(read_smi_reg(SMICS) & SMICS_DONE) && ++timeout <
        10000000)
        ;
    if (timeout >= 10000000)
        printf ("Timed out on write operation! \r\n");
    if (!(read_smi_reg(SMICS) & SMICS_TXE))
        printf( "WARNING: FIFO not empty at end of write operation. \r\n");*/

     while (!(read_smi_reg(SMICS) & SMICS_TXE))
         ;

}

void smi_setup_clock(int divi, int divf)
{
    uint32_t  cm_smi_ctl_temp = 0,  cm_smi_div_temp = 0;
    cm_smi_ctl_temp = 0;//read_cm_smi_reg(CM_SMI_CTL);
    cm_smi_div_temp = 0;//read_cm_smi_reg(CM_SMI_DIV);

    write_cm_smi_reg(CM_PWD, CM_SMI_CTL);
    SET_BIT_FIELD(cm_smi_ctl_temp, CM_SMI_CTL_SRC, 6);
    //SET_BIT_FIELD(cm_smi_ctl_temp, CM_SMI_CTL_MASH, 0x2);

    cm_smi_ctl_temp |= CM_PWD | CM_SMI_CTL_ENAB;
    printf ("try to set: cm_smi_ctl_temp: 0x%08x\r\n",cm_smi_ctl_temp);
    write_cm_smi_reg(cm_smi_ctl_temp, CM_SMI_CTL);

    SET_BIT_FIELD(cm_smi_div_temp, CM_SMI_DIV_DIVI, divi);
    SET_BIT_FIELD(cm_smi_div_temp, CM_SMI_DIV_DIVF, divf);
    cm_smi_div_temp|= CM_PWD;
    //printf ("DIV: 0x%08x\r\n",cm_smi_div_temp);
    //printf(" \r\n");

    //unsigned int *div_addr = (unsigned int *)0x3F101074;

    write_cm_smi_reg(cm_smi_div_temp, CM_SMI_DIV);
   //*div_addr = (0x5a01E000);
    //*((unsigned int*) (CM_SMI_BASE_ADDRESS+CM_SMI_DIV)) = (cm_smi_div_temp|= CM_PWD);
}



static inline void write_cm_gpioclk_reg(uint32_t val, rpi_reg_rw_t reg_offset)
{
    rpi_reg_rw_t *target_reg = (uint32_t*) CM_GPIO_BASE_ADDRESS + reg_offset/4;
    *target_reg = val;
}

static inline uint32_t read_cm_gpioclk_reg(rpi_reg_rw_t reg_offset)
{
    rpi_reg_rw_t *target_reg = (uint32_t*) CM_GPIO_BASE_ADDRESS + reg_offset/4;
    uint32_t reg_value = (uint32_t) *target_reg;
    return reg_value;
}
void gpioclk_setup_clock(int divi, int divf)
{
    uint32_t  cm_gpioclk_ctl_temp = 0,  cm_gpioclk_div_temp = 0;
    //cm_gpioclk_ctl_temp = read_cm_gpioclk_reg(CM_SMI_CTL);
    cm_gpioclk_div_temp = read_cm_gpioclk_reg(CM_SMI_DIV);

    SET_BIT_FIELD(cm_gpioclk_ctl_temp, CM_SMI_CTL_SRC, 0x6);
    SET_BIT_FIELD(cm_gpioclk_ctl_temp, CM_SMI_CTL_MASH, 0x1);
    cm_gpioclk_ctl_temp|= CM_PWD | CM_SMI_CTL_ENAB;
    printf ("try to set: cm_gpioclk_ctl_temp : 0x%08x\r\n",cm_gpioclk_ctl_temp);
    write_cm_gpioclk_reg(cm_gpioclk_ctl_temp, CM_SMI_CTL);

    SET_BIT_FIELD(cm_gpioclk_div_temp, CM_SMI_DIV_DIVI, divi);
    SET_BIT_FIELD(cm_gpioclk_div_temp, CM_SMI_DIV_DIVF, divf);
    cm_gpioclk_div_temp|= CM_PWD;

    //printf(" \r\n");

//    unsigned int *div_addr = (unsigned int *)0x3F101074;
//    unsigned int *con_addr = (unsigned int *)0x3F101070;
//    *con_addr = 0x5a000216;
//    RPI_WaitMicroSeconds(2000);
//    *div_addr = 0x5a01E000;


    write_cm_gpioclk_reg(cm_gpioclk_div_temp, CM_SMI_DIV);

    printf("GPIOCLK_SMI_CTL:  0x%08x \r\n", read_cm_gpioclk_reg(CM_SMI_CTL));
    printf("GPIOCLK_SMI_DIV:  0x%08x \r\n", read_cm_gpioclk_reg(CM_SMI_DIV));
   //uint32_t *div_addr = (0x5a01E000);
   // *((unsigned int*) (CM_SMI_BASE_ADDRESS+CM_SMI_DIV)) = (cm_smi_div_temp|= CM_PWD);
}
