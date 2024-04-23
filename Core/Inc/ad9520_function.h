#ifndef __AD9520_FUNCTION_H
#define __AD9520_FUNCTION_H


#include "main.h"
#include <stdint.h>

struct ad9520_reg{
    uint16_t reg_addr;
    uint16_t reg_data;
    uint16_t reg_setval;
};

typedef struct ad9520_reg ad9520_reg_t;

struct _ad9520_regs{
    ad9520_reg_t serial_port_conf;
    ad9520_reg_t part_id ;
    ad9520_reg_t eeprom_custom_id_0;
    ad9520_reg_t eeprom_custom_id_1;

    ad9520_reg_t PFD_charge_pump;
    ad9520_reg_t R_counter_l;
    ad9520_reg_t R_counter_h;
    ad9520_reg_t A_counter;
    ad9520_reg_t B_counter_l;
    ad9520_reg_t B_counter_h;
    ad9520_reg_t pll_ctrl_1;
    ad9520_reg_t pll_ctrl_2;
    ad9520_reg_t pll_ctrl_3;
    ad9520_reg_t pll_ctrl_4;
    ad9520_reg_t pll_ctrl_5;
    ad9520_reg_t pll_ctrl_6;
    ad9520_reg_t pll_ctrl_7;
    ad9520_reg_t pll_ctrl_8;
    ad9520_reg_t pll_ctrl_9;
    ad9520_reg_t pll_readback;  //0x1f

    //output Driver control
    ad9520_reg_t out0_control;
    ad9520_reg_t out1_control;
    ad9520_reg_t out2_control;
    ad9520_reg_t out3_control;
    ad9520_reg_t out4_control;
    ad9520_reg_t out5_control;
    ad9520_reg_t out6_control;
    ad9520_reg_t out7_control;
    ad9520_reg_t out8_control;
    ad9520_reg_t out9_control;
    ad9520_reg_t out10_control;
    ad9520_reg_t out11_control;
    ad9520_reg_t enable_output_on_csdld_l;
    ad9520_reg_t enable_output_on_csdld_h;

    //output mode and dividers
    ad9520_reg_t divider_0_0;
    ad9520_reg_t divider_0_1;
    ad9520_reg_t divider_0_2;  //0x192
    ad9520_reg_t divider_1_0;
    ad9520_reg_t divider_1_1;
    ad9520_reg_t divider_1_2;  //0x195
    ad9520_reg_t divider_2_0;
    ad9520_reg_t divider_2_1;
    ad9520_reg_t divider_2_2;  //0x198
    ad9520_reg_t divider_3_0;
    ad9520_reg_t divider_3_1;
    ad9520_reg_t divider_3_2;  //0x19b


    ad9520_reg_t vco_divider; //0x1e0
    ad9520_reg_t input_clks; //0x1e1

    ad9520_reg_t power_down; //0x230
    ad9520_reg_t io_update; //0x232


};
typedef struct _ad9520_regs ad9520_regs_t;

#define WR_OPR1 0x0000
#define WR_OPR2 0X2000
#define WR_OPRs 0X6000
#define RD_OPR2 0Xa000
#define RD_OPR1 0X8000


void init_ad9520(void);
void check_ad9520_status(void);
void read_ad9520_reg(uint16_t reg_addr, uint16_t *reg_data);
#endif /* __AD9520_FUNCTION_H */
