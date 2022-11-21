/****************************************************************************
** Copyright (C) 2020 MikroElektronika d.o.o.
** Contact: https://www.mikroe.com/contact
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
** The above copyright notice and this permission notice shall be
** included in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
** OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
** DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
** OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
**  USE OR OTHER DEALINGS IN THE SOFTWARE.
****************************************************************************/

/*!
 * @file ultralowpress.c
 * @brief Ultra-Low Press Click Driver.
 */
#include "ultralowpress.h"
    
extern APP_SENSORS_DATA APP_SENSORS_data;

ultralowpress_return_value_t ULTRALOWPRESS_status;

int16_t ULTRALOWPRESS_2sCompToDecimal(uint16_t twos_compliment_val)
{
    // [0x0000; 0x7FFF] corresponds to [0; 32,767]
    // [0x8000; 0xFFFF] corresponds to [-32,768; -1]
    // int16_t has the range [-32,768; 32,767]

    uint16_t sign_mask = 0x8000;

    // if positive
    if ( (twos_compliment_val & sign_mask) == 0 ) {
        return twos_compliment_val;
    //  if negative
    } else {
        // invert all bits, add one, and make negative
        return -(~twos_compliment_val + 1);
    }
}

uint16_t ULTRALOWPRESS_reorderBytes(uint16_t word)
{
    uint16_t bytes_swapped;
    
    bytes_swapped = ( ((word << 8 ) & 0xFF00) | ((word >> 8) & 0x00FF) );
    
    return (bytes_swapped);
}

uint32_t ULTRALOWPRESS_init(void)
{
    uint32_t serial_number = 0;

    APP_SENSORS_data.i2c.rxBuffWords[0] = 0;
    APP_SENSORS_data.i2c.rxBuffWords[1] = 0;
    
    // Read the 32-bit serial number from the SM8436
    APP_SENSORS_writeReadWords(ULTRALOWPRESS_I2CADDR, ULTRALOWPRESS_REG_SERIAL_NUM_L, 4);

    serial_number = APP_SENSORS_data.i2c.rxBuffWords[1] << 16;
    serial_number |= APP_SENSORS_data.i2c.rxBuffWords[0];
    
    if (serial_number == 0)
    {
        ULTRALOWPRESS_status = ULTRALOWPRESS_ERROR;
        printf("<ULP Click> SM8436 was not found during initialization\r\n");
    }
    else
    {
        ULTRALOWPRESS_status = ULTRALOWPRESS_OK;        
        printf("<ULP Click> SM8436 Serial Number = %u\r\n", serial_number);
    }
    
    return (serial_number);
}

bool ULTRALOWPRESS_isReady(void)
{   
    uint16_t status_reg;
    
    APP_SENSORS_writeReadWords(ULTRALOWPRESS_I2CADDR, ULTRALOWPRESS_REG_STATUS, 2);
    status_reg = APP_SENSORS_data.i2c.rxBuffWords[0];
    
    return ( status_reg & ULTRALOWPRESS_STATUS_TEMP_MASK ) && 
            ( status_reg & ULTRALOWPRESS_STATUS_PRESS_MASK );
}

void ULTRALOWPRESS_clearStatus (void)
{
    APP_SENSORS_writeWord_LSB_b4_MSB(ULTRALOWPRESS_I2CADDR, ULTRALOWPRESS_REG_STATUS, ULTRALOWPRESS_STATUS_CLEAR);
}

float ULTRALOWPRESS_getTemperature(void)
{
    int16_t decimal;
    
    APP_SENSORS_writeReadWords(ULTRALOWPRESS_I2CADDR, ULTRALOWPRESS_REG_TEMP, 2);
    decimal = ULTRALOWPRESS_2sCompToDecimal(APP_SENSORS_data.i2c.rxBuffWords[0]);

    return ( (decimal - ULTRALOWPRESS_B0) / ULTRALOWPRESS_B1 );
}

float ULTRALOWPRESS_getPressure(void)
{
    int16_t decimal;

    APP_SENSORS_writeReadWords(ULTRALOWPRESS_I2CADDR, ULTRALOWPRESS_REG_PRESS, 2);
    decimal = ULTRALOWPRESS_2sCompToDecimal(APP_SENSORS_data.i2c.rxBuffWords[0]);
    
    return ULTRALOWPRESS_P_MIN + ( ( decimal - ULTRALOWPRESS_OUT_MIN ) / 
            ( ULTRALOWPRESS_OUT_MAX - ULTRALOWPRESS_OUT_MIN ) ) * 
            ( ULTRALOWPRESS_P_MAX - ULTRALOWPRESS_P_MIN );
}

// ------------------------------------------------------------------------- END
