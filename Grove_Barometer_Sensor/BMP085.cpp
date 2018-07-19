/*
 * Barometer.cpp
 * A library for barometer
 *
 * Copyright (c) 2012 seeed technology inc.
 * Website    : www.seeed.cc
 * Author     : LG
 * Create Time:
 * Change Log :
 * 
 * loovee 9-24-2014
 * Change all int to short, all unsigned int to unsigned short to fit some 32bit system
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <math.h>
#include <unistd.h>
#include "BMP085.h"
#include "i2crw.h"


void BMP085::init(unsigned int nr ,unsigned int saveAddr)
{
    i2ccrw.begin(nr ,saveAddr);
    ac1 = bmp085ReadInt(0xAA);
    ac2 = bmp085ReadInt(0xAC);
    ac3 = bmp085ReadInt(0xAE);
    ac4 = bmp085ReadInt(0xB0);
    ac5 = bmp085ReadInt(0xB2);
    ac6 = bmp085ReadInt(0xB4);
    b1 = bmp085ReadInt(0xB6);
    b2 = bmp085ReadInt(0xB8);
    mb = bmp085ReadInt(0xBA);
    mc = bmp085ReadInt(0xBC);
    md = bmp085ReadInt(0xBE);
}

// Read 1 byte from the BMP085 at 'address'
// Return: the read byte;
char BMP085::bmp085Read(unsigned char address)
{
   return i2ccrw.i2c_read_wdata(address);
}

// Read 2 bytes from the BMP085
// First byte will be from 'address'
// Second byte will be from 'address'+1
short BMP085::bmp085ReadInt(unsigned char address)
{
	return i2ccrw.i2c_read_wdata(address);
}

// Read the uncompensated temperature value
unsigned short BMP085::bmp085ReadUT()
{
    unsigned short ut;
   
    i2ccrw.i2c_write_bdata(0xF4,0x2E);
	usleep(10000);
    ut = bmp085ReadInt(0xF6);
    return ut;
}
// Read the uncompensated pressure value
unsigned long BMP085::bmp085ReadUP()
{
    unsigned long up = 0;

	i2ccrw.i2c_write_bdata(0xF4,0x34 + (OSS<<6));
    // Read register 0xF6 (MSB), 0xF7 (LSB), and 0xF8 (XLSB)
    //usleep(50000);
   
	unsigned char values[3];
	i2ccrw.i2c_read_block_data(0xF6, 3,values);
    up = (((unsigned long) values[0] << 16) | ((unsigned long) values[1] << 8) | (unsigned long) values[2]) >> (8-OSS);
    return up;
}

//1013.25hPa
float BMP085::calcAltitude(float seaLevelPressure)
{
    float pressure = bmp085GetPressure(bmp085ReadUP());//Get the temperature
    float altitude = 44330.0 * (1.0 - pow(pressure / seaLevelPressure, 0.1903));
	//a pressure change of ∆p = 1hPa corresponds to 8.43m at sea level.
    return altitude/8.43;
   
}

float BMP085::bmp085GetTemperature(unsigned short ut)
{
    long x1, x2;

    x1 = (((long)ut - (long)ac6)*(long)ac5) >> 15;
    x2 = ((long)mc << 11)/(x1 + md);
    PressureCompensate = x1 + x2;

    float temp = ((PressureCompensate + 8)>>4);
    temp = temp /10;

    return temp;
}

long BMP085::bmp085GetPressure(unsigned long up)
{
    long x1, x2, x3, b3, b6, p;
    unsigned long b4, b7;
    b6 = PressureCompensate - 4000;
    x1 = (b2 * (b6 * b6)>>12)>>11;
    x2 = (ac2 * b6)>>11;
    x3 = x1 + x2;
    b3 = (((((long)ac1)*4 + x3)<<OSS) + 2)>>2;

    // Calculate B4
    x1 = (ac3 * b6)>>13;
    x2 = (b1 * ((b6 * b6)>>12))>>16;
    x3 = ((x1 + x2) + 2)>>2;
    b4 = (ac4 * (unsigned long)(x3 + 32768))>>15;

    b7 = ((unsigned long)(up - b3) * (50000>>OSS));
    if (b7 < 0x80000000)
    p = (b7<<1)/b4;
    else
    p = (b7/b4)<<1;

    x1 = (p>>8) * (p>>8);
    x1 = (x1 * 3038)>>16;
    x2 = (-7357 * p)>>16;
    p += (x1 + x2 + 3791)>>4;

    long temp = p;
    return temp;
}