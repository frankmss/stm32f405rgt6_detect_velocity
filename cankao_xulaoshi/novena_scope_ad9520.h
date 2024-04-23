#ifndef __AD9520_H__
#define __AD9520_H__

//SERIAL_PORT_CONFIGURATION
#define AD9520_SERIAL_PORT_CONFIG 0x000 

//0x001 AD9520_UNUSED
//0x002 AD9520_RESERVED
#define AD9520_PART_ID            0x003 
#define AD9520_READBACK_CONTROL	  0x004 

//EEPROM_ID
#define AD9520_EEPROM_CUSTOMER_VERSION_ID_LSB  0x005 
#define AD9520_EEPROM_CUSTOMER_VERSION_ID_MSB  0x006 

// AD9520_0x007_TO_0x00F_UNUSED

// AD9520_PLL
#define AD9520_PFD_CHARGE_PUMP   0x010 
#define AD9520_R_COUNTER_LSB	 0x011 
#define AD9520_R_COUNTER_MSB	 0x012 
#define AD9520_A_COUNTER	 0x013 
#define AD9520_B_COUNTER_LSB     0x014 
#define AD9520_B_COUNTER_MSB     0x015
#define AD9520_PLL_CTRL_1               0x016 
#define AD9520_PLL_CTRL_2		0x017 
#define AD9520_PLL_CTRL_3		0x018 
#define AD9520_PLL_CTRL_4		0x019 
#define AD9520_PLL_CTRL_5		0x01A 
#define AD9520_PLL_CTRL_6		0x01B 
#define AD9520_PLL_CTRL_7		0x01C 
#define AD9520_PLL_CTRL_8		0x01D 
#define AD9520_PLL_CTRL_9		0x01E 
#define AD9520_PLL_READBACK_READ_ONLY	0x01F 


// OUTPUT_DRIVER_CONTROL
#define AD9520_OUT0_CONTROL                 0x0F0 
#define AD9520_OUT1_CONTROL		    0x0F1 
#define AD9520_OUT2_CONTROL		    0x0F2 
#define AD9520_OUT3_CONTROL		    0x0F3 
#define AD9520_OUT4_CONTROL		    0x0F4 
#define AD9520_OUT5_CONTROL		    0x0F5 
#define AD9520_OUT6_CONTROL		    0x0F6 
#define AD9520_OUT7_CONTROL		    0x0F7 
#define AD9520_OUT8_CONTROL		    0x0F8 
#define AD9520_OUT9_CONTROL		    0x0F9 
#define AD9520_OUT10_CONTROL		    0x0FA 
#define AD9520_OUT11_CONTROL		    0x0FB 
#define AD9520_ENABLE_OUTPUT_ON_CSDLD_LSB   0x0FC 
#define AD9520_ENABLE_OUTPUT_ON_CSDLD_MSB   0x0FD

//0x0FE_TO_0x18F_UNUSED

//LVPECL_CHANNEL_DIVIDERS 
#define AD9520_DIVIDER_0_PECL    0x190 
#define AD9520_DIVIDER_0_BYPASS	 0x191 
#define AD9520_DIVIDER_0_POWER   0x192
#define AD9520_DIVIDER_1_PECL    0x193 
#define AD9520_DIVIDER_1_BYPASS	 0x194 
#define AD9520_DIVIDER_1_POWER   0x195
#define AD9520_DIVIDER_2_PECL    0x196 
#define AD9520_DIVIDER_2_BYPASS	 0x197 
#define AD9520_DIVIDER_2_POWER   0x198
#define AD9520_DIVIDER_3_PECL    0x199 
#define AD9520_DIVIDER_3_BYPASS	 0x19A 
#define AD9520_DIVIDER_3_POWER   0x19B

//0x19C AD9520_TO_0x1DF_UNUSED


// VCO_DIVIDER_AND_CLK_INPUT
#define AD9520_VCO_DIVIDER  0x1E0 
#define AD9520_INPUT_CLKS   0x1E1 
//0x1E2_TO_0x22A_UNUSED

// SYSTEM
#define AD9520_POWER_DOWN_AND_SYNC 0x230 
//0x231 AD9520_UNUSED

// UPDATE_ALL_REGISTERS
#define AD9520_IO_UPDATE  0x232 
// 0x233_TO_0x9FF_UNUSED

// EEPROM_BUFFER_SEGMENT
#define AD9520_EEPROM_SERIAL_PORT_CONFIGURATION  0xA00 
//#define 0xA01 STARTING_ADDRESS:_ADDRESS_0x000
//#define 0xA02 0x00
#define AD9520_EEPROM_CUSTOMER_VERSION_ID  0xA03 
//#define 0xA04 STARTING_ADDRESS:_ADDRESS_0x004
//#define 0xA05 0x04
#define AD9520_EEPROM_PLL_SETTINGS  0xA06 
//#define 0xA07 STARTING_ADDRESS:_ADDRESS_0x010
//#define 0xA08 0x10
#define AD9520_EEPROM_OUTPUT_DRIVER_CONTROL  0xA09 
//#define 0xA0A STARTING_ADDRESS:_ADDRESS_0x0F0
//#define 0xA0B 0xF0
#define AD9520_EEPROM_LVPECL_CHANNEL_DIVIDERS   0xA0C 
//#define 0xA0D STARTING_ADDRESS:_ADDRESS_0x190
//#define 0xA0E 0x90
#define AD9520_EEPROM_VCO_DIVIDER_AND_CLK_INPUT   0xA0F 
//#define 0xA10 STARTING_ADDRESS:_ADDRESS_0x1E0
//#define 0xA11 0xE0
#define AD9520_EEPROM_POWER_DOWN_AND_SYNC   0xA12 
//#define 0xA13 STARTING_ADDRESS:_ADDRESS_0x230
//#define 0xA14 0x30
#define AD9520_EEPROM_IO_UPDATE    0xA15 
#define AD9520_EEPROM_END_OF_DATA  0xA16 
//#define 0xA17 AD9520_TO_0xAFF_UNUSED

// EEPROM_CONTROL
#define AD9520_EEPROM_STATUS_READ_ONLY          0xB00 
#define AD9520_EEPROM_ERROR_CHECKING_READ_ONLY	0xB01 
#define AD9520_EEPROM_CONTROL_1			0xB02 
#define AD9520_EEPROM_CONTROL_2			0xB03 

class Ad9520 {

public:
    enum speed {
        Speed500Mhz,
        Speed1GHz,
    };

    Ad9520();
    ~Ad9520();

    const char *id(void);
    int selfConfig(enum speed speed);
    void printStatus(void);

private:
    /* I2C routines */
    int write(quint16 address, quint8 byte);
    int writeBuffer(quint16 address, quint8 *bytes, int count);
    int read(quint16 address, quint8 *byte);
    int readBuffer(quint16 address, quint8 *bytes, int count);
    int i2cOpen(void);

    int i2c_fd;
};

#endif /* __AD9520_H__ */
