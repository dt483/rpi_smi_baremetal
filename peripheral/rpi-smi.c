#include <stdint.h>
#include <stdio.h>
#include "rpi-smi.h"
#include "rpi-base.h"
#include "rpi-gpio.h"



static inline void write_smi_reg(struct smi_instance *inst,
    uint32_t val, rpi_reg_rw_t reg)
{
    rpi_reg_rw_t *target_reg =inst->smi_regs_ptr + reg;
    *target_reg = val;
}

static inline uint32_t read_smi_reg(struct smi_instance *inst, rpi_reg_rw_t reg)
{
    rpi_reg_rw_t *target_reg = inst->smi_regs_ptr + reg;
    uint32_t reg_value = (uint32_t) *target_reg;
    return reg_value;
}

static inline void write_cm_smi_reg(struct smi_instance *inst, uint32_t val, rpi_reg_rw_t reg)
{
    rpi_reg_rw_t *target_reg =inst->cm_smi_regs_ptr + reg;
    *target_reg = val;
}

static inline uint32_t read_cm_smi_reg(struct smi_instance *inst, rpi_reg_rw_t reg)
{
    rpi_reg_rw_t *target_reg = inst->cm_smi_regs_ptr + reg;
   // printf ("target_reg: 0x%08x: 0x%08x \r\n", target_reg, *target_reg);
    uint32_t reg_value = (uint32_t) *target_reg;
   // printf ("reg_value: 0x%08x: \r\n", reg_value);
    return reg_value;
}

#define _CONCAT(x, y) x##y
#define CONCAT(x, y) _CONCAT(x, y)

#define SET_BIT_FIELD(dest, field, bits) ((dest) = \
    ((dest) & ~CONCAT(field, _MASK)) | (((bits) << CONCAT(field, _OFFS))& \
     CONCAT(field, _MASK)))
#define GET_BIT_FIELD(src, field) (((src) & \
    CONCAT(field, _MASK)) >> CONCAT(field, _OFFS))

void smi_dump_context_labelled(struct smi_instance *inst)
{
    //RPI_SetGpioPinFunction( RPI_GPIO14, FS_ALT5 );
    //RPI_SetGpioPinFunction( RPI_GPIO15, FS_ALT5 );

    printf("SMI context dump: \r\n");
    printf("CM_SMI_CTL:  0x%08x \r\n", read_cm_smi_reg(inst, CM_SMI_CTL));
    printf("CM_SMI_DIV:  0x%08x \r\n", read_cm_smi_reg(inst, CM_SMI_DIV));
    printf("SMICS:  0x%08x \r\n", read_smi_reg(inst, SMICS));
    printf("SMIL:   0x%08x \r\n", read_smi_reg(inst, SMIL));
    printf("SMIDSR: 0x%08x \r\n", read_smi_reg(inst, SMIDSR0));
    printf("SMIDSW: 0x%08x \r\n", read_smi_reg(inst, SMIDSW0));
    printf("SMIDC:  0x%08x \r\n", read_smi_reg(inst, SMIDC));
    printf("SMIFD:  0x%08x \r\n", read_smi_reg(inst, SMIFD));
    printf(" \r\n");
    RPI_WaitMicroSeconds(2000);

   // RPI_SetGpioPinFunction( RPI_GPIO14, FS_ALT1 );
   // RPI_SetGpioPinFunction( RPI_GPIO15, FS_ALT1 );
}

void smi_init(struct smi_instance *inst)
{
    inst->cm_smi_regs_ptr = CM_SMI_BASE_ADDRESS ;
    inst->smi_regs_ptr = SMI_BASE_ADDRESS ; //Pointer for base smi physical address

    printf ("Smi address inititalized: \r\n");
    printf ("CM_SMI base: 0x%08x: 0x%08x \r\n", inst->cm_smi_regs_ptr, *inst->cm_smi_regs_ptr);
    printf ("SMI base: 0x%08x : 0x%08x \r\n", inst->smi_regs_ptr, *inst->smi_regs_ptr);
    printf(" \r\n");

    //smi_dump_context_labelled(inst);
}

void smi_get_default_settings(struct smi_instance *inst)
{
    struct smi_settings *settings = &inst->settings;

    settings->data_width = SMI_WIDTH_16BIT;
    settings->pack_data = true;

    settings->read_setup_time = 1;
    settings->read_hold_time = 1;
    settings->read_pace_time = 1;
    settings->read_strobe_time = 3;

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

void smi_set_regs_from_settings(struct smi_instance *inst)
{
    struct smi_settings *settings = &inst->settings;
    int smidsr_temp = 0, smidsw_temp = 0, smics_temp,
        smidcs_temp, smidc_temp = 0;

    //spin_lock(&inst->transaction_lock);

    /* temporarily disable the peripheral: */
    smics_temp = read_smi_reg(inst, SMICS);
    write_smi_reg(inst, 0, SMICS);
    smidcs_temp = read_smi_reg(inst, SMIDCS);
    write_smi_reg(inst, 0, SMIDCS);

    if (settings->pack_data)
        smics_temp |= SMICS_PXLDAT;
    else
        smics_temp &= ~SMICS_PXLDAT;

    SET_BIT_FIELD(smidsr_temp, SMIDSR_RWIDTH, settings->data_width);
    SET_BIT_FIELD(smidsr_temp, SMIDSR_RSETUP, settings->read_setup_time);
    SET_BIT_FIELD(smidsr_temp, SMIDSR_RHOLD, settings->read_hold_time);
    SET_BIT_FIELD(smidsr_temp, SMIDSR_RPACE, settings->read_pace_time);
    SET_BIT_FIELD(smidsr_temp, SMIDSR_RSTROBE, settings->read_strobe_time);
    write_smi_reg(inst, smidsr_temp, SMIDSR0);

    SET_BIT_FIELD(smidsw_temp, SMIDSW_WWIDTH, settings->data_width);
    if (settings->data_width == SMI_WIDTH_8BIT)
        smidsw_temp |= SMIDSW_WSWAP;
    else
        smidsw_temp &= ~SMIDSW_WSWAP;
    SET_BIT_FIELD(smidsw_temp, SMIDSW_WSETUP, settings->write_setup_time);
    SET_BIT_FIELD(smidsw_temp, SMIDSW_WHOLD, settings->write_hold_time);
    SET_BIT_FIELD(smidsw_temp, SMIDSW_WPACE, settings->write_pace_time);
    SET_BIT_FIELD(smidsw_temp, SMIDSW_WSTROBE,
            settings->write_strobe_time);
    write_smi_reg(inst, smidsw_temp, SMIDSW0);


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

    write_smi_reg(inst, smidc_temp, SMIDC);

    /* re-enable (if was previously enabled) */
    write_smi_reg(inst, smics_temp, SMICS);
    write_smi_reg(inst, smidcs_temp, SMIDCS);

//	spin_unlock(&inst->transaction_lock);
}

inline void smi_set_address(struct smi_instance *inst,
    unsigned int address)
{
    int smia_temp = 0, smida_temp = 0;

    SET_BIT_FIELD(smia_temp, SMIA_ADDR, address);
    SET_BIT_FIELD(smida_temp, SMIDA_ADDR, address);

    /* Write to both address registers - user doesn't care whether we're
       doing programmed or direct transfers. */
    write_smi_reg(inst, smia_temp, SMIA);
    write_smi_reg(inst, smida_temp, SMIDA);
}


void smi_setup_regs(struct smi_instance *inst)
{

//    dev_dbg(inst->dev, "Initialising SMI registers...");
    /* Disable the peripheral if already enabled */
    write_smi_reg(inst, 0, SMICS);
    write_smi_reg(inst, 0, SMIDCS);

    smi_get_default_settings(inst);
    smi_set_regs_from_settings(inst);
    smi_set_address(inst, 0);

    write_smi_reg(inst, read_smi_reg(inst, SMICS) | SMICS_ENABLE, SMICS);
    write_smi_reg(inst, read_smi_reg(inst, SMIDCS) | SMIDCS_ENABLE,
        SMIDCS);
}


inline void smi_write_single_word(struct smi_instance *inst,
    uint32_t data)
{
    int timeout = 0;

    write_smi_reg(inst, SMIDCS_ENABLE | SMIDCS_WRITE, SMIDCS);
    write_smi_reg(inst, data, SMIDD);
    write_smi_reg(inst, SMIDCS_ENABLE | SMIDCS_WRITE | SMIDCS_START,
        SMIDCS);

    while (!(read_smi_reg(inst, SMIDCS) & SMIDCS_DONE) &&
        ++timeout < 10000)
        ;
    if (timeout >= 10000)
        printf("SMI direct write timed out (is the clock set up correctly?)");
}

void smi_setup_clock(struct smi_instance *inst, int divi, int divf)
{
    int  cm_smi_ctl_temp = 0,  cm_smi_div_temp = 0;
    cm_smi_ctl_temp = read_cm_smi_reg(inst, CM_SMI_CTL);
    cm_smi_div_temp = read_cm_smi_reg(inst, CM_SMI_DIV);

    SET_BIT_FIELD(cm_smi_ctl_temp, CM_SMI_CTL_SRC, 0x6);

    write_cm_smi_reg(inst, cm_smi_ctl_temp | CM_PWD, CM_SMI_CTL);

    SET_BIT_FIELD(cm_smi_div_temp, CM_SMI_DIV_DIVI, divi);
    SET_BIT_FIELD(cm_smi_div_temp, CM_SMI_DIV_DIVF, divf);
    write_cm_smi_reg(inst, cm_smi_div_temp, CM_SMI_DIV);

}
