#ifndef _TWI_DRIVER_BMP180_H
#define _TWI_DRIVER_BMP180_H

#define BMP180_ADDR                 0x77

#define BMP180_CHIP_ID              0x55
// Register definitions
#define BMP180_PROM_START__ADDR     0xAA
#define BMP180_CHIP_ID_REG          0xD0
#define BMP180_CTRL_MEAS_REG        0xF4
#define BMP180_ADC_OUT_MSB_REG      0xF6
//
#define BMP180_T_MEASURE            0x2E    // temperature measurent 
#define BMP180_P_MEASURE            0x34    // pressure measurement

#define SMD500_PARAM_MG             3038    // calibration parameter
#define SMD500_PARAM_MH            -7357    // calibration parameter
#define SMD500_PARAM_MI             3791    // calibration parameter

typedef struct
{
    // Calibration parameters
    int16_t  ac1;
    int16_t  ac2; 
    int16_t  ac3; 
    uint16_t ac4;
    uint16_t ac5;
    uint16_t ac6;
    int16_t  b1; 
    int16_t  b2;
    int16_t  mb;
    int16_t  mc;
    int16_t  md;
}s_bmp180_calib_t;

uint8_t twi_BMP180_Config(void);

#endif
