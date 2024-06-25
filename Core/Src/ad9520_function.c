#include "ad9520_function.h"

extern SPI_HandleTypeDef hspi1;
// 初始化stm32f4到ad9520的spi接口

// #define spi1_cs_Pin GPIO_PIN_4
// #define spi1_cs_GPIO_Port GPIOC

ad9520_regs_t ad9520_regs = {
    .serial_port_conf = {0x00, 0x00,0x00},
    .part_id = {0x03, 0x00,0x00},
    .eeprom_custom_id_0 = {0x05, 0x00   ,0x00},
    .eeprom_custom_id_1 = {0x06, 0x00   ,0x00},

    .PFD_charge_pump= {0x010, 0x00  ,0x00},
    .R_counter_l = {0x011, 0x00 ,0x00},
    .R_counter_h = {0x012, 0x00 ,0x00},
    .A_counter = {0x013, 0x00   ,0x00},
    .B_counter_l = {0x014, 0x00 ,0x00},
    .B_counter_h = {0x015, 0x00 ,0x00},
    .pll_ctrl_1 = {0x016, 0x00  ,0x00},
    .pll_ctrl_2 = {0x017, 0x00  ,0x00},
    .pll_ctrl_3 = {0x018, 0x00  ,0x00},
    .pll_ctrl_4 = {0x019, 0x00  ,0x00},
    .pll_ctrl_5 = {0x01a, 0x00  ,0x00},
    .pll_ctrl_6 = {0x01b, 0x00  ,0x00},
    .pll_ctrl_7 = {0x01c, 0x00  ,0x00},
    .pll_ctrl_8 = {0x01d, 0x00  ,0x00},
    .pll_ctrl_9 = {0x01e, 0x00  ,0x00},
    .pll_readback = {0x01f, 0x00    ,0x00},

    .out0_control = {0x0f0, 0x00    ,0x00},
    .out1_control = {0x0f1, 0x00    ,0x00},
    .out2_control = {0x0f2, 0x00    ,0x00},
    .out3_control = {0x0f3, 0x00    ,0x00},
    .out4_control = {0x0f4, 0x00    ,0x00},
    .out5_control = {0x0f5, 0x00    ,0x00},
    .out6_control = {0x0f6, 0x00    ,0x00},
    .out7_control = {0x0f7, 0x00    ,0x00},
    .out8_control = {0x0f8, 0x00    ,0x00},
    .out9_control = {0x0f9, 0x00    ,0x00},
    .out10_control = {0x0fa, 0x00   ,0x00},
    .out11_control = {0x0fb, 0x00   ,0x00},
    .enable_output_on_csdld_l = {0x0fc, 0x00    ,0x00},
    .enable_output_on_csdld_h = {0x0fd, 0x00    ,0x00},

    .divider_0_0 = {0x190, 0x00 ,0x00},
    .divider_0_1 = {0x191, 0x00,0x00},
    .divider_0_2 = {0x192, 0x00,0x00},
    .divider_1_0 = {0x193, 0x00,0x00},
    .divider_1_1 = {0x194, 0x00,0x00},
    .divider_1_2 = {0x195, 0x00,0x00},
    .divider_2_0 = {0x196, 0x00,0x00},
    .divider_2_1 = {0x197, 0x00,0x00},
    .divider_2_2 = {0x198, 0x00,0x00},
    .divider_3_0 = {0x199, 0x00,0x00},
    .divider_3_1 = {0x19a, 0x00,0x00},
    .divider_3_2 = {0x19b, 0x00,0x00},

    .vco_divider = {0x1e0, 0x00,0x00},
    .input_clks = {0x1e1, 0x00,0x00},
    
    .power_down = {0x230, 0x00,0x00},
    .io_update = {0x232, 0x00,0x00},

};

void init_ad9520_spiport(void){
    // 初始化spi_cs引脚状态，设置这个引脚为高电平
    HAL_GPIO_WritePin(spi1_cs_GPIO_Port, spi1_cs_Pin, GPIO_PIN_SET);
}

// 写ad9520寄存器
void write_ad9520_reg(uint16_t reg_addr, uint16_t reg_data){
    // 1. 设置spi_cs引脚为低电平
    HAL_Delay(2);
    HAL_GPIO_WritePin(spi1_cs_GPIO_Port, spi1_cs_Pin, GPIO_PIN_RESET);
    // 2. 发送寄存器地址
    HAL_Delay(3);
    HAL_SPI_Transmit(&hspi1, (uint8_t *)&reg_addr, 1, 100);
    // 3. 发送寄存器数据
    HAL_SPI_Transmit(&hspi1, (uint8_t *)&reg_data, 1, 100);
    // 4. 设置spi_cs引脚为高电平
    HAL_Delay(3);
    HAL_GPIO_WritePin(spi1_cs_GPIO_Port, spi1_cs_Pin, GPIO_PIN_SET);
    HAL_Delay(2);
}
// 读ad9520寄存器
void read_ad9520_reg(uint16_t reg_addr, uint16_t *reg_data){
    //uint16_t read_data;
    // 1. 设置spi_cs引脚为低电平
    //HAL_Delay(2);
    HAL_GPIO_WritePin(spi1_cs_GPIO_Port, spi1_cs_Pin, GPIO_PIN_RESET);
    // 2. 发送寄存器地址
    //HAL_Delay(1);
    HAL_SPI_Transmit(&hspi1, (uint8_t *)&reg_addr, 1, 100);
    // 3. 发送寄存器数据
    HAL_SPI_Receive(&hspi1, (uint8_t *)reg_data, 1, 100);
    // 4. 设置spi_cs引脚为高电平
    //HAL_Delay(1);
    HAL_GPIO_WritePin(spi1_cs_GPIO_Port, spi1_cs_Pin, GPIO_PIN_SET);
   // HAL_Delay(2);
    //return read_data;
}

// 读ad9520所有寄存器的默认值,并保存到ad9520_regs数组中的reg_setval成员中
// setDefaultVal: 0:不保存默认值，1:保存默认值
#define SAVE_DEFAULT_VAL 1
#define NOT_SAVE_DEFAULT_VAL 0
void read_ad9520_all_regs_setdefaultVal(uint32_t setDefaultVal){
    int32_t i;
    int32_t sizes = sizeof(ad9520_regs)/sizeof(struct ad9520_reg);
    uint16_t tmp_add_read1byte, tmp_data_read1byte;
    ad9520_reg_t *pad9520_reg = (ad9520_reg_t *)(&ad9520_regs);
    for(i=0; i<sizes; i++){
        tmp_add_read1byte = (pad9520_reg+i)->reg_addr | RD_OPR2;
        
        read_ad9520_reg(tmp_add_read1byte, &tmp_data_read1byte);
        (pad9520_reg+i)->reg_data = tmp_data_read1byte>>8;
        if(setDefaultVal == 1){
            (pad9520_reg+i)->reg_setval = (pad9520_reg+i)->reg_data;
        }

    }
}



void update_ad9520_regs(void){
    //write_ad9520_reg(ad9520_regs.serial_port_conf.reg_addr|WR_OPR2, 0xa0a0); //set spi sdo mode
    //HAL_Delay(30);
    //write_ad9520_reg(ad9520_regs.io_update.reg_addr|WR_OPR2, 0x0000);
    //HAL_Delay(30);
    write_ad9520_reg(ad9520_regs.io_update.reg_addr|WR_OPR2, 0x0100);
    //HAL_Delay(30);
    write_ad9520_reg(ad9520_regs.io_update.reg_addr|WR_OPR2, 0x0000);
}

//设置ad9520的spi端口
void set_ad9520_spiport(void){

    write_ad9520_reg(ad9520_regs.serial_port_conf.reg_addr|WR_OPR2, 0x0202); //soft reset
    //HAL_Delay(30);
    write_ad9520_reg(ad9520_regs.serial_port_conf.reg_addr|WR_OPR2, 0xa0a0); //set spi sdo mode
    //HAL_Delay(30);
    write_ad9520_reg(ad9520_regs.io_update.reg_addr|WR_OPR2, 0x0000);
    //HAL_Delay(30);
    write_ad9520_reg(ad9520_regs.io_update.reg_addr|WR_OPR2, 0x0100);
    //HAL_Delay(30);
    write_ad9520_reg(ad9520_regs.io_update.reg_addr|WR_OPR2, 0x0000);
    //HAL_Delay(30);
    

}

//设置ad9520的EEPROM custom id reg
void set_ad9520_eeprom_custom_id_reg(uint16_t custom_id){

    write_ad9520_reg(ad9520_regs.eeprom_custom_id_1.reg_addr|WR_OPR2, custom_id);

}

//设置ad9520寄存器的用户值
// setPumpUpDown: 0:设置Pump Up, 1:设置Pump Down
#define PUMP_UP 0
#define PUMP_DOWN 1
#define PUMP_NAL 2
void set_custom_val(uint32_t setPumpUpDown){
    
   //#if 0 //ppt实验记录1 1.验证clk单端输入，output9A输出
    //set clk ditribution < 1600MHz mode 1 ppt实验记录1 1.验证clk单端输入，output9A输出
   // ad9520_regs.PFD_charge_pump.reg_setval = ad9520_regs.PFD_charge_pump.reg_setval | ((0x1)); // 0x010[1:0] = 01b
    ad9520_regs.input_clks.reg_setval = ad9520_regs.input_clks.reg_setval | (0x1); // 0x1e1[0] = 1
    ad9520_regs.input_clks.reg_setval = ad9520_regs.input_clks.reg_setval & (~(0x2)); // 0x1e1[4] = 0
    ad9520_regs.out9_control.reg_setval = 0xee;
    ad9520_regs.out0_control.reg_setval = 0xee;

    ad9520_regs.divider_0_0.reg_setval = 0x00;
    ad9520_regs.divider_0_1.reg_setval = ad9520_regs.divider_0_1.reg_setval | 0x80;
	
	//2024.3.8 for output9pin clk 
	ad9520_regs.divider_3_0.reg_setval = 0x10;
    //2024.5.13 test output9-pin
	//ad9520_regs.divider_3_1.reg_setval = ad9520_regs.divider_3_1.reg_setval | 0x80;
    ad9520_regs.divider_3_1.reg_setval = ad9520_regs.divider_3_1.reg_setval | 0x40;
//    ad9520_regs.divider_3_0.reg_setval = 0x00;
//    ad9520_regs.divider_3_1.reg_setval = ad9520_regs.divider_3_1.reg_setval | 0x80;

    ad9520_regs.vco_divider.reg_setval = 0x00;
    
   //#endif
   

    /*#if 0 //2023 8月之前的配置
    //set clk ditribution < 1600MHz mode 1 ppt实验记录1 1.验证clk单端输入，output9A输出
    ad9520_regs.PFD_charge_pump.reg_setval = ad9520_regs.PFD_charge_pump.reg_setval | ((0x1)); // 0x010[1:0] = 01b
    ad9520_regs.input_clks.reg_setval = ad9520_regs.input_clks.reg_setval | (0x1); // 0x1e1[0] = 1
    ad9520_regs.input_clks.reg_setval = ad9520_regs.input_clks.reg_setval & (~(0x2)); // 0x1e1[4] = 0
    ad9520_regs.out9_control.reg_setval = 0xee;
    ad9520_regs.divider_3_0.reg_setval = 0x00;
    ad9520_regs.vco_divider.reg_setval = 0x06;
    //bypass divider3
    ad9520_regs.divider_3_1.reg_setval = ad9520_regs.divider_3_1.reg_setval | 0x80;
    //此时，output9A的频率为 8MHz
    //bypass divider1
    ad9520_regs.divider_1_0.reg_setval = ad9520_regs.divider_1_0.reg_setval | 0x80;
    ad9520_regs.out0_control.reg_setval = 0xee;
    //此时，output0的频率应该为 8MHz

    //设置mux3，选择internal feedback path, mux1 选择zero delay path
    ad9520_regs.pll_ctrl_9.reg_setval = ad9520_regs.pll_ctrl_9.reg_setval & 0b11111001;
    ad9520_regs.pll_ctrl_9.reg_setval = ad9520_regs.pll_ctrl_9.reg_setval | 0b00000010; //0x01e[2:1] = 01b

    //设置AB寄存器，p=1，a=0，b=1，clk=(pxb)+a = 1
    ad9520_regs.pll_ctrl_1.reg_setval = ad9520_regs.pll_ctrl_1.reg_setval & 0b11111000;
    ad9520_regs.pll_ctrl_1.reg_setval = ad9520_regs.pll_ctrl_1.reg_setval | 0b00000001; //0x016[2:0] = 001b p=1
    ad9520_regs.B_counter_l.reg_setval = ad9520_regs.B_counter_l.reg_setval & 0b00000000;
    ad9520_regs.B_counter_l.reg_setval = ad9520_regs.B_counter_l.reg_setval | 0b00000001; //0x014[7:0] = 00000001b b=1
    ad9520_regs.A_counter.reg_setval = ad9520_regs.A_counter.reg_setval & 0b00000000; //0x013[7:0] = 00000000b a=0

    //设置R寄存器，r=1
    ad9520_regs.R_counter_l.reg_setval = ad9520_regs.R_counter_l.reg_setval & 0b00000000;
    ad9520_regs.R_counter_l.reg_setval = ad9520_regs.R_counter_l.reg_setval | 0b00000001; //0x012[7:0] = 00000001b r=1

    //设置PDF寄存器，
    ad9520_regs.PFD_charge_pump.reg_setval = ad9520_regs.PFD_charge_pump.reg_setval & 0b00000000;
    //    使能pll ， cp current = 4.8mA，
    ad9520_regs.PFD_charge_pump.reg_setval = ad9520_regs.PFD_charge_pump.reg_setval | 0b01110000; //0x010[1:0] = 01b
    //ad9520_regs.PFD_charge_pump.reg_setval = ad9520_regs.PFD_charge_pump.reg_setval | 0b00000000; //0x010[1:0] = 01b
    if(setPumpUpDown == PUMP_UP){
        ad9520_regs.PFD_charge_pump.reg_setval = ad9520_regs.PFD_charge_pump.reg_setval | 0b00000100; //source current up;   
    }else if(setPumpUpDown == PUMP_DOWN){
        ad9520_regs.PFD_charge_pump.reg_setval = ad9520_regs.PFD_charge_pump.reg_setval | 0b00001000; //source current down;
    }else if(setPumpUpDown == PUMP_NAL){
        ad9520_regs.PFD_charge_pump.reg_setval = ad9520_regs.PFD_charge_pump.reg_setval | 0b00001100; //source current down;
    }
    //Sets the PFD polarity
    ad9520_regs.PFD_charge_pump.reg_setval = ad9520_regs.PFD_charge_pump.reg_setval & 0b01111111; //0x010[1:0] = 01b 
    ad9520_regs.PFD_charge_pump.reg_setval = ad9520_regs.PFD_charge_pump.reg_setval | 0b10000000; //Sets the PFD polarity negative
    //for test set cp=vcp/2
    // ad9520_regs.pll_ctrl_1.reg_setval = ad9520_regs.pll_ctrl_1.reg_setval & 0b01111111;
    // ad9520_regs.pll_ctrl_1.reg_setval = ad9520_regs.pll_ctrl_1.reg_setval | 0b10000000; //0x016[7] = 1
    #endif
		*/



        //设置PDF寄存器，
    ad9520_regs.PFD_charge_pump.reg_setval = ad9520_regs.PFD_charge_pump.reg_setval & 0X00;
    //    使能pll ， cp current = 4.8mA，
		//ad9520_regs.PFD_charge_pump.reg_setval = ad9520_regs.PFD_charge_pump.reg_setval | 0b01110000; //0x010[1:0] = 01b 4.8mA
		ad9520_regs.PFD_charge_pump.reg_setval = ad9520_regs.PFD_charge_pump.reg_setval | 0b00110000; //0x010[1:0] = 01b 2.4mA
    //ad9520_regs.PFD_charge_pump.reg_setval = ad9520_regs.PFD_charge_pump.reg_setval | 0b00000000; //0x010[1:0] = 01b 0.6mA
    if(setPumpUpDown == PUMP_UP){
        ad9520_regs.PFD_charge_pump.reg_setval = ad9520_regs.PFD_charge_pump.reg_setval | 0b00000100; //source current up;   
    }else if(setPumpUpDown == PUMP_DOWN){
        ad9520_regs.PFD_charge_pump.reg_setval = ad9520_regs.PFD_charge_pump.reg_setval | 0b00001000; //source current down;
    }else if(setPumpUpDown == PUMP_NAL){
        ad9520_regs.PFD_charge_pump.reg_setval = ad9520_regs.PFD_charge_pump.reg_setval | 0b00001100; //source current down;
    }
    //Sets the PFD polarity
    ad9520_regs.PFD_charge_pump.reg_setval = ad9520_regs.PFD_charge_pump.reg_setval & 0b01111111; //0x010[1:0] = 01b 
    //ad9520_regs.PFD_charge_pump.reg_setval = ad9520_regs.PFD_charge_pump.reg_setval | 0b10000000; //Sets the PFD polarity negative
    //for test set cp=vcp/2
    // ad9520_regs.pll_ctrl_1.reg_setval = ad9520_regs.pll_ctrl_1.reg_setval & 0b01111111;
    // ad9520_regs.pll_ctrl_1.reg_setval = ad9520_regs.pll_ctrl_1.reg_setval | 0b10000000; //0x016[7] = 1

        //设置R counter
    ad9520_regs.R_counter_l.reg_setval = ad9520_regs.R_counter_l.reg_setval & 0b00000000;
    ad9520_regs.R_counter_h.reg_setval = ad9520_regs.R_counter_h.reg_setval & 0b00000000;
    ad9520_regs.R_counter_l.reg_setval = ad9520_regs.R_counter_l.reg_setval | 0b00000001; //R counter = 1
		//do{ //set R diver 13bit 8192 20231120
		//	ad9520_regs.R_counter_l.reg_setval = ad9520_regs.R_counter_l.reg_setval | 0b00000011;
		//	ad9520_regs.R_counter_h.reg_setval = ad9520_regs.R_counter_h.reg_setval | 0b00000000;
		//}while(0);
            //R=2700=0xA8C  l_byte=0x8c h_byte=0x0a
    //ad9520_regs.R_counter_l.reg_setval = ad9520_regs.R_counter_l.reg_setval | (2700&0xff); //R counter = 0x3FFA
    //ad9520_regs.R_counter_h.reg_setval = ad9520_regs.R_counter_h.reg_setval | (2700>>8); //R counter = 
    //ad9520_regs.R_counter_l.reg_setval = ad9520_regs.R_counter_l.reg_setval | (7904&0xff); //R counter = 0x3FFA
    //ad9520_regs.R_counter_h.reg_setval = ad9520_regs.R_counter_h.reg_setval | (7904>>8); //R counter = 
        //设置B counter
    ad9520_regs.B_counter_l.reg_setval = ad9520_regs.B_counter_l.reg_setval & 0b00000000;
    ad9520_regs.B_counter_h.reg_setval = ad9520_regs.B_counter_h.reg_setval & 0b00000000;
    ad9520_regs.B_counter_l.reg_setval = ad9520_regs.B_counter_l.reg_setval | 0b00000001; //B counter = 1
		//do{ //set B=8192, 20231120
		//	ad9520_regs.B_counter_l.reg_setval = ad9520_regs.B_counter_l.reg_setval | 0b11111111;
		//	ad9520_regs.B_counter_h.reg_setval = ad9520_regs.B_counter_h.reg_setval | 0b00011111;
		//}while(0);

        //设置A counter
    ad9520_regs.A_counter.reg_setval = ad9520_regs.A_counter.reg_setval & 0x00;
    //ad9520_regs.A_counter.reg_setval = ad9520_regs.A_counter.reg_setval | 0b00000001;

            //B=2805=0xAF5 l_byte=0xf5 h_byte=0x0a
            //ad9520_regs.B_counter_l.reg_setval = ad9520_regs.B_counter_l.reg_setval |4;
    //ad9520_regs.B_counter_l.reg_setval = ad9520_regs.B_counter_l.reg_setval | (4050&0xff); //B counter = 2805
    //ad9520_regs.B_counter_h.reg_setval = ad9520_regs.B_counter_h.reg_setval | 4050>>8; //B counter = 2805
    //ad9520_regs.B_counter_l.reg_setval = ad9520_regs.B_counter_l.reg_setval | (1350&0xff); //B counter = 2805
    //ad9520_regs.B_counter_h.reg_setval = ad9520_regs.B_counter_h.reg_setval | 1350>>8; //B counter = 2805

            //A=60=0x3c 
    //ad9520_regs.A_counter.reg_setval = ad9520_regs.A_counter.reg_setval | 60; //A counter = 60

        //设置 pll_ctrl_1 0x16
        //1: B counter is set to divide-by-1. 
        //This allows the prescaler setting to determine the divide for the N divider.. 
        //Divide-by-2.
		//ad9520_regs.pll_ctrl_1.reg_setval = 0x01; //p=1,dived2, 20231120
    ad9520_regs.pll_ctrl_1.reg_setval = 0x08; //ad9520_regs.pll_ctrl_1.reg_setval & 0b00000000; 
            
    //ad9520_regs.pll_ctrl_1.reg_setval = 0x05 ; //set prescaler P= 16/17
    //ad9520_regs.pll_ctrl_1.reg_setval = 0x02; //Divide-by-2 and divide-by-3 when A ≠ 0; divide-by-2 when A = 0.
    //ad9520_regs.pll_ctrl_1.reg_setval = 0x06; //Divide-by-32 and divide-by-33 when A ≠ 0; divide-by-32 when A = 0 (default).
    //ad9520_regs.pll_ctrl_1.reg_setval = 0x01; //fix 1/2.
        //设置 status pin to function
    ad9520_regs.pll_ctrl_2.reg_setval = ad9520_regs.pll_ctrl_2.reg_setval & 0b00;
    ad9520_regs.pll_ctrl_2.reg_setval = ad9520_regs.pll_ctrl_2.reg_setval | 0b0100; // to N divider output
		ad9520_regs.pll_ctrl_2.reg_setval = ad9520_regs.pll_ctrl_2.reg_setval | 0b10; //set Antibacklash Pulse Width = 6ns 20231130,0b10=6.0, 0b01=1.3, 0b11=2.9=0b00=
    //ad9520_regs.pll_ctrl_2.reg_setval = ad9520_regs.pll_ctrl_2.reg_setval | 0b10000100;//REF1 clock (differential reference when in differential mode).
   // ad9520_regs.pll_ctrl_2.reg_setval = ad9520_regs.pll_ctrl_2.reg_setval | 0b000001000;//R divider output (after the delay).    
    //ad9520_regs.pll_ctrl_2.reg_setval = ad9520_regs.pll_ctrl_2.reg_setval   | 0b11100100; //(Status of REF1 frequency) AND (status of REF2 frequency)
    //ad9520_regs.pll_ctrl_2.reg_setval = ad9520_regs.pll_ctrl_2.reg_setval   | 0b10011100; //Status of REF1 frequency; active high.
    // enable status_pin
    ad9520_regs.pll_ctrl_8.reg_setval = ad9520_regs.pll_ctrl_2.reg_setval & 0b01111111; //bit7 = 0  


        //设置 pll_ctrl_3 0x018 = 0x06
        //0: normal lock detect operation (default).16 (default). 
        //This setting is fine for any PFD frequency, 
        //but it also results in the longest VCO calibration time,VCO calibration=0
    ad9520_regs.pll_ctrl_3.reg_setval = 0x06|0x80;

        //设置pll_ctrl_7 0x01c = 0x02
    ad9520_regs.pll_ctrl_7.reg_setval = 0x02;

        //设置pll_ctrl_9 0x01e = 0x12
        //Selects Channel Divider 2. 
        //0: enables internal zero delay mode if Register 0x01E[1] = 1. 1: 
        //enables zero delay function.
    ad9520_regs.pll_ctrl_9.reg_setval = 0x16;

        //设置input_clks 0x1e1 = 0x02
    ad9520_regs.input_clks.reg_setval = ad9520_regs.input_clks.reg_setval & 0b11111101;

    //2024.6.21 加入设置ad9520的sync管脚设置,设置sync pin = 0，当同步时
    //*这个好像没有起作用，
    ad9520_regs.pll_ctrl_4.reg_setval = ad9520_regs.pll_ctrl_4.reg_setval | 0b01000000;

    //烧写寄存器
    //program PFD & PLL 0X010 ---- 0X01F:PFD_charge_pump --- pll_readback
    //write 8X16bits, wirthe 8 times
    ad9520_reg_t *pad9520_reg = (ad9520_reg_t *)(&(ad9520_regs.pll_readback));
    do{
        //write addr|stream mode
        HAL_Delay(2);
        HAL_GPIO_WritePin(spi1_cs_GPIO_Port, spi1_cs_Pin, GPIO_PIN_RESET);
        // 2. 发送寄存器地址
        HAL_Delay(3);
        uint16_t tmpAdr = (pad9520_reg->reg_addr)|WR_OPRs;
        HAL_SPI_Transmit(&hspi1, (uint8_t *)&(tmpAdr), 1, 100);
        // 3. 发送寄存器数据
        for(int i=0; i<8; i++){
            HAL_Delay(3);
            uint16_t tmpData = ((pad9520_reg-i*2)->reg_setval << 8) | ((pad9520_reg-i*2-1)->reg_setval );
            //tmpData = 0x3c3c;
            HAL_SPI_Transmit(&hspi1, (uint8_t *)&(tmpData), 1, 100);
        }
        // 4. 设置spi_cs引脚为高电平
        HAL_Delay(3);
        HAL_GPIO_WritePin(spi1_cs_GPIO_Port, spi1_cs_Pin, GPIO_PIN_SET);
        HAL_Delay(2);
    }while(0);
    //update_ad9520_regs();


    //program output control 0x0F0 ---- 0x0FD : out0_control --- enable_output_on_csdld_h
    //write 7X16bits, wirthe 7 times
    pad9520_reg = (ad9520_reg_t *)(&(ad9520_regs.enable_output_on_csdld_h));
    do{
        HAL_Delay(2);
        HAL_GPIO_WritePin(spi1_cs_GPIO_Port, spi1_cs_Pin, GPIO_PIN_RESET);
        // 2. 发送寄存器地址
        HAL_Delay(3);
        uint16_t tmpAdr = (pad9520_reg->reg_addr)|WR_OPRs;
        HAL_SPI_Transmit(&hspi1, (uint8_t *)&(tmpAdr), 1, 100);
        // 3. 发送寄存器数据
        for(int i=0; i<7; i++){
            HAL_Delay(3);
            uint16_t tmpData = ((pad9520_reg-i*2)->reg_setval << 8) | ((pad9520_reg-i*2-1)->reg_setval );
            //tmpData = 0x3c3c;
            HAL_SPI_Transmit(&hspi1, (uint8_t *)&(tmpData), 1, 100);
        }
        // 4. 设置spi_cs引脚为高电平
        HAL_Delay(3);
        HAL_GPIO_WritePin(spi1_cs_GPIO_Port, spi1_cs_Pin, GPIO_PIN_SET);
        HAL_Delay(2);
    }while(0);
    
    //program output mode and dividers 0x190 --- 0x19b : Divider_0 --- Divider_3_2
    //write 6X16bits, wirthe 6 times
    pad9520_reg = (ad9520_reg_t *)(&(ad9520_regs.divider_3_2));
    do{
        HAL_Delay(2);
        HAL_GPIO_WritePin(spi1_cs_GPIO_Port, spi1_cs_Pin, GPIO_PIN_RESET);
        // 2. 发送寄存器地址
        HAL_Delay(3);
        uint16_t tmpAdr = (pad9520_reg->reg_addr)|WR_OPRs;
        HAL_SPI_Transmit(&hspi1, (uint8_t *)&(tmpAdr), 1, 100);
        // 3. 发送寄存器数据
        for(int i=0; i<6; i++){
            HAL_Delay(3);
            uint16_t tmpData = ((pad9520_reg-i*2)->reg_setval << 8) | ((pad9520_reg-i*2-1)->reg_setval );
            //tmpData = 0x3c3c;
            HAL_SPI_Transmit(&hspi1, (uint8_t *)&(tmpData), 1, 100);
        }
        // 4. 设置spi_cs引脚为高电平
        HAL_Delay(3);
        HAL_GPIO_WritePin(spi1_cs_GPIO_Port, spi1_cs_Pin, GPIO_PIN_SET);
        HAL_Delay(2);
    }while(0);

    //program VCO divider 0x1e0 --- 0x1e1 : VCO_drivider --- input clk
    //write 1X16bits, wirthe 1 times
    pad9520_reg = (ad9520_reg_t *)(&(ad9520_regs.input_clks));
    do{
        HAL_Delay(2);
        HAL_GPIO_WritePin(spi1_cs_GPIO_Port, spi1_cs_Pin, GPIO_PIN_RESET);
        // 2. 发送寄存器地址
        HAL_Delay(3);
        uint16_t tmpAdr = (pad9520_reg->reg_addr)|WR_OPRs;
        HAL_SPI_Transmit(&hspi1, (uint8_t *)&(tmpAdr), 1, 100);
        // 3. 发送寄存器数据
        for(int i=0; i<1; i++){
            HAL_Delay(3);
            uint16_t tmpData = ((pad9520_reg-i*2)->reg_setval << 8) | ((pad9520_reg-i*2-1)->reg_setval );
            //tmpData = 0x3c3c;
            HAL_SPI_Transmit(&hspi1, (uint8_t *)&(tmpData), 1, 100);
        }
        // 4. 设置spi_cs引脚为高电平
        HAL_Delay(3);
        HAL_GPIO_WritePin(spi1_cs_GPIO_Port, spi1_cs_Pin, GPIO_PIN_SET);
        HAL_Delay(2);
    }while(0);

    //program system power control 0x230 ---  : system_power_control --- 
    //write 1X16bits, wirthe 1 times
    pad9520_reg = (ad9520_reg_t *)(&(ad9520_regs.power_down));
    do{
        HAL_Delay(2);
        HAL_GPIO_WritePin(spi1_cs_GPIO_Port, spi1_cs_Pin, GPIO_PIN_RESET);
        // 2. 发送寄存器地址
        HAL_Delay(3);
        uint16_t tmpAdr = (pad9520_reg->reg_addr)|WR_OPRs;
        HAL_SPI_Transmit(&hspi1, (uint8_t *)&(tmpAdr), 1, 100);
        // 3. 发送寄存器数据
        for(int i=0; i<1; i++){
            HAL_Delay(3);
            uint16_t tmpData = ((pad9520_reg-i)->reg_setval << 8) | ((pad9520_reg-i)->reg_setval );
            //tmpData = 0x3c3c;
            HAL_SPI_Transmit(&hspi1, (uint8_t *)&(tmpData), 1, 100);
        }
        // 4. 设置spi_cs引脚为高电平
        HAL_Delay(3);
        HAL_GPIO_WritePin(spi1_cs_GPIO_Port, spi1_cs_Pin, GPIO_PIN_SET);
        HAL_Delay(2);
    }while(0);
    update_ad9520_regs();
}
uint32_t tmpx=0;
// ad9520寄存器初始化
void init_ad9520(){
    init_ad9520_spiport();
    set_ad9520_spiport();
    set_ad9520_eeprom_custom_id_reg(0xeb90);
    read_ad9520_all_regs_setdefaultVal(SAVE_DEFAULT_VAL);
    
    HAL_Delay(30);
    set_custom_val(PUMP_NAL);
    HAL_Delay(30);
    read_ad9520_all_regs_setdefaultVal(NOT_SAVE_DEFAULT_VAL);
    HAL_Delay(30);
    // read_ad9520_all_regs_setdefaultVal(SAVE_DEFAULT_VAL);
    // HAL_Delay(30);
}


void check_ad9520_status(){
    read_ad9520_all_regs_setdefaultVal(NOT_SAVE_DEFAULT_VAL);
}