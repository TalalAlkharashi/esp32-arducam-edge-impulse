// ArduCAM demo (C)2020 Lee
// web: http://www.ArduCAM.com
// This program is a demo of how to use most of the functions of the library with a supported camera modules.
// This demo can only work on ARDUCAM_SHIELD_V2 platform.
// This demo will run the MT9D111 with JPEG mode, it is compatible with autofocus and non-autofocus.
// The demo sketch will do the following tasks:
// 1. Set the sensor to JPEG output mode.
// 2. Capture and buffer the image to FIFO. 
// 3. Store the image to Micro SD/TF card with JPEG format.
// This program requires the ArduCAM V4.0.0 (or later) library and ArduCAM shield V2
// and use Arduino IDE 1.8.9 compiler or above.

#include <UTFT_SPI.h>
#include <SD.h>
#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#if defined(__arm__)
#include <itoa.h>
#endif

#define SD_CS 9
#define JPEGIMAGEOFFSET 625

  //  w1 = 0x0140;  320
  //  h1 = 0x00f0;  240
  //  w2 = 0x0320;  800
  //  h2 = 0x0258;  600
  //  w2 = 0x0640;  1600
  //  h2 = 0x04B0;  1200
  
int jpeg = 1;
unsigned long jpeg_length = 0;
// Set the offset to adjust the error data at the end of the image.
int16_t OFFSET = 50;
//set resolution 
const uint16_t width = 1600;
const uint16_t height = 1200;
// set pin 10 as the slave select for the ArduCAM shiel:
const int CS = 10;

const char JPEG_header[625] PROGMEM =
{
  0xff, 0xd8, 0xff, 0xe0, 0x00, 0x10, 0x4a, 0x46, 0x49, 0x46, 0x00, 0x01, 0x02, 0x00, 0x00, 0x01,
  0x00, 0x01, 0x00, 0x00, 0xff, 0xdb, 0x00, 0x84, 0x00, 0x03, 0x02, 0x02, 0x03, 0x02, 0x02, 0x03,
  0x03, 0x02, 0x03, 0x03, 0x03, 0x03, 0x04, 0x05, 0x08, 0x05, 0x05, 0x04, 0x04, 0x05, 0x09, 0x07,
  0x07, 0x05, 0x08, 0x0b, 0x0a, 0x0b, 0x0b, 0x0b, 0x0a, 0x0b, 0x0a, 0x0c, 0x0e, 0x11, 0x0f, 0x0c,
  0x0d, 0x10, 0x0d, 0x0a, 0x0b, 0x0f, 0x14, 0x0f, 0x10, 0x12, 0x12, 0x13, 0x14, 0x13, 0x0c, 0x0e,
  0x15, 0x17, 0x15, 0x13, 0x17, 0x11, 0x13, 0x13, 0x13, 0x01, 0x03, 0x03, 0x03, 0x05, 0x04, 0x05,
  0x09, 0x05, 0x05, 0x09, 0x13, 0x0c, 0x0b, 0x0c, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
  0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
  0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
  0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0xff, 0xc0, 0x00, 0x11, 0x08, 0x04,
  0x80, 0x06, 0x40, 0x03, 0x00, 0x21, 0x00, 0x01, 0x11, 0x01, 0x02, 0x11, 0x01, 0xff, 0xc4, 0x00,
  0x1f, 0x00, 0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0xff, 0xc4,
  0x00, 0xb5, 0x10, 0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03, 0x05, 0x05, 0x04, 0x04, 0x00,
  0x00, 0x01, 0x7d, 0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12, 0x21, 0x31, 0x41, 0x06, 0x13,
  0x51, 0x61, 0x07, 0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08, 0x23, 0x42, 0xb1, 0xc1, 0x15,
  0x52, 0xd1, 0xf0, 0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x25,
  0x26, 0x27, 0x28, 0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46,
  0x47, 0x48, 0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64, 0x65, 0x66,
  0x67, 0x68, 0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x83, 0x84, 0x85, 0x86,
  0x87, 0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4,
  0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2,
  0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9,
  0xda, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5,
  0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xff, 0xc4, 0x00, 0x1f, 0x01, 0x00, 0x03, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
  0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0xff, 0xc4, 0x00, 0xb5, 0x11, 0x00, 0x02, 0x01, 0x02, 0x04,
  0x04, 0x03, 0x04, 0x07, 0x05, 0x04, 0x04, 0x00, 0x01, 0x02, 0x77, 0x00, 0x01, 0x02, 0x03, 0x11,
  0x04, 0x05, 0x21, 0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71, 0x13, 0x22, 0x32, 0x81, 0x08,
  0x14, 0x42, 0x91, 0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0, 0x15, 0x62, 0x72, 0xd1, 0x0a,
  0x16, 0x24, 0x34, 0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x35,
  0x36, 0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x53, 0x54, 0x55,
  0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x73, 0x74, 0x75,
  0x76, 0x77, 0x78, 0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x92, 0x93,
  0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa,
  0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8,
  0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6,
  0xe7, 0xe8, 0xe9, 0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xff, 0xdd, 0x00,
  0x04, 0x00, 0x20, 0xff, 0xda, 0x00, 0x0c, 0x03, 0x00, 0x00, 0x01, 0x11, 0x02, 0x11, 0x00, 0x3f,
  0x00
};

const uint16_t patch_addr_0400[206] PROGMEM = {
  0x0400,
  0x00cc,
  0xce06, 0x10ed, 0x0230, 0xec00, 0xce06, 0x10ed, 0x00ce, 0x0610,
  0xec04, 0xc300, 0x01ed, 0x04ce, 0x1070, 0xc603, 0xe70f, 0xc611,
  0xe740, 0xc680, 0xe742, 0xc603, 0xe701, 0xc6fe, 0xe74c, 0xc600,
  0xe71a, 0xe71b, 0xe710, 0xe712, 0xc680, 0xe711, 0xc680, 0xe713,
  0xc601, 0xe744, 0xc6fe, 0xe746, 0x39ce, 0x0620, 0xed02, 0x30ec,
  0x00ce, 0x0620, 0xed00, 0xce06, 0x20e6, 0x05cb, 0x01c4, 0x7fe7,
  0x05ce, 0x0720, 0x3a18, 0xce06, 0x2018, 0xe603, 0xe700, 0x3c34,
  0xce01, 0x0de6, 0x03e7, 0x04ce, 0x0620, 0xe603, 0xce01, 0x0de7,
  0x03ce, 0x1070, 0xc602, 0xe703, 0xc6ff, 0xe746, 0xc601, 0xe745,
  0x18ce, 0x010d, 0x18e6, 0x034f, 0xc300, 0x0105, 0xed10, 0x18e6,
  0x034f, 0x1830, 0x18ed, 0x01cc, 0x0101, 0x18a3, 0x0105, 0xed12,
  0xc600, 0xe745, 0xc6fe, 0xe746, 0xc602, 0xe703, 0x4fcc, 0x0050,   // 50 is time needed for lens setling
  0xbd9b, 0x1118, 0xce01, 0x0d18, 0xe605, 0xca02, 0x18e7, 0x0530,
  0xc603, 0x3a35, 0x39ce, 0x0630, 0xed02, 0x30ec, 0x00ce, 0x0630,
  0xed00, 0xce06, 0x30ec, 0x08c3, 0x0001, 0xed08, 0x39ce, 0x0640,
  0xed02, 0x30ec, 0x00ce, 0x0640, 0xed00, 0xcc06, 0x40ec, 0x08c3,
  0x0001, 0xed08, 0xbd9c, 0x43c1, 0x0126, 0x1218, 0xce01, 0x0d18,
  0xe605, 0xf400, 0xfd18, 0xe705, 0xce06, 0x40e7, 0x04ce, 0x0640,
  0xe704, 0x18ce, 0x010d, 0x18e6, 0x0539, 0x3c3c, 0x3c3c, 0x34cc,
  0x02c4, 0x30ed, 0x06fe, 0x1050, 0xec0c, 0xfd02, 0xc0fe, 0x02c0,
  0xec00, 0xfd02, 0xc230, 0x6f08, 0xe608, 0x4f05, 0xf302, 0xc28f,
  0xec00, 0x30ed, 0x00e6, 0x084f, 0x05e3, 0x0618, 0x8fec, 0x0018,
  0xed00, 0x6c08, 0xe608, 0xc104, 0x25de, 0x30ee, 0x06cc, 0x0400,
  0xed00, 0x30ee, 0x06cc, 0x0449, 0xed02, 0x30ee, 0x06cc, 0x04d5,
  0xed04, 0x30ee, 0x06cc, 0x04ed, 0xed06, 0xcc02, 0xc4fe, 0x010d,
  0xed00, 0x30c6, 0x093a, 0x3539
};

ArduCAM myCAM(MT9D111_B, CS);
UTFT myGLCD(CS);

void AF_refocus() {
  Serial.println("af_refocus");
  myCAM.wrSensorReg8_16(0xF0, 0x01);
  myCAM.wrSensorReg8_16(0xC6, 0xA102);
  myCAM.wrSensorReg8_16(0xC8, 0x31);
  myCAM.wrSensorReg8_16(0xC6, 0xA504);
  myCAM.wrSensorReg8_16(0xC8, 0x01);
}

void AF_mode() {
  uint16_t value, v;

  myCAM.wrSensorReg8_16( 0xF0, 0x02);
  myCAM.rdSensorReg8_16(0x4C, &v);
  v |= 0x000B;
  myCAM.wrSensorReg8_16(0x4C, v);
  myCAM.rdSensorReg8_16(0x56, &v);
  v |= 0x000B;
  myCAM.wrSensorReg8_16(0x56, v);

  myCAM.wrSensorReg8_16( 0xF0, 0x01);
  myCAM.wrSensorReg8_16( 0xC6, 0xA102);
  myCAM.rdSensorReg8_16( 0xC8, &value);
  value |= 0x0010;                        // enable AF driver
  myCAM.wrSensorReg8_16( 0xC6, 0xA102);
  myCAM.wrSensorReg8_16( 0xC8, value);
  myCAM.wrSensorReg8_16( 0xC6, 0xA102);
  myCAM.rdSensorReg8_16( 0xC8, &value);



  if (jpeg == 1 )  {
    value = 0x01;
    myCAM.wrSensorReg8_16( 0xC6, 0xA12C);
    myCAM.wrSensorReg8_16( 0xC8, value);

    refresh();

    value = 0x00;
    myCAM.wrSensorReg8_16( 0xC6, 0xA12C);
    myCAM.wrSensorReg8_16( 0xC8, value);

    value = 0x01;
    myCAM.wrSensorReg8_16( 0xC6, 0xA13A);
    myCAM.wrSensorReg8_16( 0xC8, value);

    myCAM.wrSensorReg8_16( 0xC6, 0xA504);
    myCAM.wrSensorReg8_16( 0xC8, 0X00);


    //    myCAM.wrSensorReg8_16( 0xC6, 0xA505);
    //    myCAM.wrSensorReg8_16( 0xC8, 0x80);

    //    myCAM.wrSensorReg8_16( 0xC6, 0xA502);
    //    myCAM.wrSensorReg8_16( 0xC8, 0x17);
    //    myCAM.wrSensorReg8_16( 0xC6, 0xA503);
    //    myCAM.wrSensorReg8_16( 0xC8, 0x11);
  }
  else {
    value = 0x01;
    myCAM.wrSensorReg8_16( 0xC6, 0xA12C);
    myCAM.wrSensorReg8_16( 0xC8, value);


    //    myCAM.wrSensorReg8_16( 0xC6, 0xA504);
    //    myCAM.wrSensorReg8_16( 0xC8, 0X00);

    //    myCAM.wrSensorReg8_16( 0xC6, 0xA505);
    //    myCAM.wrSensorReg8_16( 0xC8, 0x80);

    //    myCAM.wrSensorReg8_16( 0xC6, 0xA502);
    //    myCAM.wrSensorReg8_16( 0xC8, 0x17);
    //    myCAM.wrSensorReg8_16( 0xC6, 0xA503);
    //    myCAM.wrSensorReg8_16( 0xC8, 0x11);

    refresh();
    refreshMode();
  }
}

void load_patch_segment()
{
  uint16_t  start, size, data, offreg;
  uint16_t  pdata;
  int offset;

  //start = patch[0];   // start MCU_ADDRESS
  start = pgm_read_word(&patch_addr_0400[0]);
  //size = patch[1];    // data size
  size = pgm_read_word(&patch_addr_0400[1]);
  pdata = 2;     // data pointer
  offset = 0;         // MCU_DATA_0,7

  myCAM.wrSensorReg8_16( 0xF0, 0x01);
  while (size > 0)
  {
    if (offset == 0)
    {
      //mi2010soc_reg_write(0x1C6, start);  // write MCU_ADDRESS
      myCAM.wrSensorReg8_16( 0xC6, start);
      //printk(KERN_INFO "REG=1, 0xC6, 0x%04X t// MCU_ADDRESSn", start);
    }
    //data = *pdata++;
    data = pgm_read_word(&patch_addr_0400[pdata++]);
    //offreg = 0x1C8 + offset;
    //mi2010soc_reg_write(offreg, data);  // write MCU_ADDRESS

    //addr = 0x2003;
    //myCAM.wrSensorReg8_16( 0xC6, addr);
    myCAM.wrSensorReg8_16( 0xC8 + offset, data);
    //printk(KERN_INFO "REG=1, 0x%02X, 0x%04X t// MCU_DATA_%dn", offreg&0xff, data, offset);
    start += 2;
    size--;
    offset++;
    if (offset == 8)
      offset = 0;
  }
  return;
}

void call_vmt(uint16_t base) {
  uint16_t value, addr, v;

  myCAM.wrSensorReg8_16( 0xF0, 0x01);
  addr = 0x2003;
  value = base;
  myCAM.wrSensorReg8_16( 0xC6, addr);
  myCAM.wrSensorReg8_16( 0xC8, value);

  myCAM.wrSensorReg8_16( 0xF0, 0x01);
  addr = 0xA002;
  value = 0x0001;
  myCAM.wrSensorReg8_16( 0xC6, addr);
  myCAM.wrSensorReg8_16( 0xC8, value);

  addr = 0xA002;
  myCAM.wrSensorReg8_16(0xC6, addr);
  myCAM.rdSensorReg8_16(0xC8, &v);
  while ( v != 0) {
    addr = 0xA002;
    myCAM.wrSensorReg8_16(0xC6, addr);
    myCAM.rdSensorReg8_16(0xC8, &v);
  }
}

void call_init(uint16_t base) {
  uint16_t value, v, addr;

  myCAM.wrSensorReg8_16( 0xF0, 0x01);
  addr = 0x2003;
  value = base;
  myCAM.wrSensorReg8_16( 0xC6, addr);
  myCAM.wrSensorReg8_16( 0xC8, value);

  myCAM.wrSensorReg8_16( 0xF0, 0x01);
  addr = 0xA002;
  value = 0x0001;
  myCAM.wrSensorReg8_16( 0xC6, addr);
  myCAM.wrSensorReg8_16( 0xC8, value);

  addr = 0xA002;
  myCAM.wrSensorReg8_16(0xC6, addr);
  myCAM.rdSensorReg8_16(0xC8, &v);
  while ( v != 0) {
    addr = 0xA002;
    myCAM.wrSensorReg8_16(0xC6, addr);
    myCAM.rdSensorReg8_16(0xC8, &v);
  }
}

void resetCAM() {
  uint16_t state;
  Serial.println("reset CAM");

  myCAM.wrSensorReg8_16( 0xF0, 0x00);
  myCAM.wrSensorReg8_16( 0x65, 0xA000);

  myCAM.wrSensorReg8_16( 0xF0, 0x01);
  myCAM.wrSensorReg8_16( 0xC3, 0x0501);

  myCAM.wrSensorReg8_16( 0xF0, 0x00);
  myCAM.wrSensorReg8_16( 0x0D, 0x0021);
  myCAM.wrSensorReg8_16( 0x0D, 0x0000);


  delayMicroseconds(10000);

  myCAM.wrSensorReg8_16( 0xF0, 0x01);
  myCAM.wrSensorReg8_16( 0xC6, 0xA104);
  myCAM.rdSensorReg8_16( 0xC8, &state);

  Serial.print("mod.state : ");
  Serial.println(state);



  delayMicroseconds(200000);
  myCAM.wrSensorReg8_16( 0xF0, 0x00);
  myCAM.wrSensorReg8_16( 0x65, 0xA000);
  myCAM.wrSensorReg8_16( 0x66, 0x1001);
  myCAM.wrSensorReg8_16( 0x67, 0x0503);

  delayMicroseconds(15000);

  myCAM.wrSensorReg8_16( 0xF0, 0x00);
  myCAM.wrSensorReg8_16( 0x65, 0x2000);

  delayMicroseconds(15000);



  if (state != 3)
  {
    myCAM.wrSensorReg8_16( 0xF0, 0x01);
    myCAM.wrSensorReg8_16( 0xC6, 0xA103);
    myCAM.wrSensorReg8_16( 0xC8, 0X0001);
    // Waiting state to standby mode
    unsigned int count = 500000;
    while (state != 3 && count > 0) {
      myCAM.wrSensorReg8_16( 0xF0, 0x01);
      myCAM.wrSensorReg8_16( 0xC6, 0xA104);
      myCAM.rdSensorReg8_16( 0xC8, &state);
      count--;
    }
    Serial.print("mod.state : ");
    Serial.println(state);
    Serial.print("mod.state : ");
    Serial.println(500000 - count);
  }
  Serial.print("mod.state : ");
  Serial.println(state);

  //  myCAM.wrSensorReg8_16( 0xF0, 0x01);      //clear possible testpattern setting
  //  myCAM.wrSensorReg8_16( 0x48, 0x0000);
  //  refresh();
}

void refresh()
{
  uint16_t cmd;
  int i;
  myCAM.wrSensorReg8_16( 0xF0, 0x01);
  myCAM.wrSensorReg8_16( 0xC6, 0xA103);
  myCAM.wrSensorReg8_16( 0xC8, 0X05);
  myCAM.wrSensorReg8_16( 0xC6, 0xA103);
  myCAM.rdSensorReg8_16( 0xC8, &cmd);
  while (( cmd != 0) && (i < 65535))
  {
    myCAM.wrSensorReg8_16( 0xC6, 0xA103);
    myCAM.rdSensorReg8_16( 0xC8, &cmd);
    i++;
  }
  if ( i == 65535)
    Serial.println("refresh time out");
}

void refreshMode()
{
  uint16_t cmd;
  int i;
  myCAM.wrSensorReg8_16( 0xF0, 0x01);
  myCAM.wrSensorReg8_16( 0xC6, 0xA103);
  myCAM.wrSensorReg8_16( 0xC8, 0X06);
  while (( cmd != 0) && (i < 65535))
  {
    myCAM.wrSensorReg8_16( 0xC6, 0xA103);
    myCAM.rdSensorReg8_16( 0xC8, &cmd);
    i++;
  }
  if ( i == 65535)
    Serial.println("refreshMode time out");
}

void setSizes(uint16_t w1, uint16_t h1, uint16_t w2, uint16_t h2 ) {
  //  uint16_t w1,h1;
  //  uint16_t w2,h2;

  //  w1 = 0x0140;  320
  //  h1 = 0x00f0;  240
  //  w2 = 0x0320;  800
  //  h2 = 0x0258;  600
  //  w2 = 0x0640;  1600
  //  h2 = 0x04B0;  1200
  //msg("setSizes");

  // A context

  myCAM.wrSensorReg8_16( 0xF0, 0x01);
  myCAM.wrSensorReg8_16( 0xC6, 0x2703);
  myCAM.wrSensorReg8_16( 0xC8, w1);
  myCAM.wrSensorReg8_16( 0xF0, 0x01);
  myCAM.wrSensorReg8_16( 0xC6, 0x2705);
  myCAM.wrSensorReg8_16( 0xC8, h1);


  // B context

  myCAM.wrSensorReg8_16( 0xF0, 0x01);
  myCAM.wrSensorReg8_16( 0xC6, 0x2707);
  myCAM.wrSensorReg8_16( 0xC8, w2);

  myCAM.wrSensorReg8_16( 0xF0, 0x01);
  myCAM.wrSensorReg8_16( 0xC6, 0x2709);
  myCAM.wrSensorReg8_16( 0xC8, h2);

  myCAM.wrSensorReg8_16( 0xF0, 0x01);
  myCAM.wrSensorReg8_16( 0xC6, 0x2779);
  myCAM.wrSensorReg8_16( 0xC8, w2);

  myCAM.wrSensorReg8_16( 0xF0, 0x01);
  myCAM.wrSensorReg8_16( 0xC6, 0x277B);
  myCAM.wrSensorReg8_16( 0xC8, h2);

  refreshMode();
  refresh();
}

void setFormats() {
  Serial.println("setFormat");

  // A context
  myCAM.wrSensorReg8_16( 0xF0, 0x01);
  myCAM.wrSensorReg8_16( 0xC6, 0xA77D);
  myCAM.wrSensorReg8_16( 0xC8, 0X00020);

  // B context

  myCAM.wrSensorReg8_16( 0xF0, 0x01);
  myCAM.wrSensorReg8_16( 0xC6, 0xA77E);
  myCAM.wrSensorReg8_16( 0xC8, 0X0000);

  // Refresh Sequencer

  //  refreshMode();
  //  refresh();


  myCAM.wrSensorReg8_16( 0xF0, 0x01);
  myCAM.wrSensorReg8_16( 0xC6, 0xA103);
  myCAM.wrSensorReg8_16( 0xC8, 0X0006);

  myCAM.wrSensorReg8_16( 0xF0, 0x01);
  myCAM.wrSensorReg8_16( 0xC6, 0xA103);
  myCAM.wrSensorReg8_16( 0xC8, 0X0005);

}

void doCapture(unsigned short restart_int) {

  Serial.println("doCapture");

  myCAM.wrSensorReg8_16( 0xF0, 0x01);
  myCAM.wrSensorReg8_16( 0x48, 0X0000);
  myCAM.wrSensorReg8_16( 0x49, 0X0000);
  myCAM.wrSensorReg8_16( 0x4A, 0X007f);
  myCAM.wrSensorReg8_16( 0x4B, 0X0000);

  myCAM.wrSensorReg8_16( 0xF0, 0x02);
  myCAM.wrSensorReg8_16( 0x00, 0X0001);
  //
  //  myCAM.wrSensorReg8_16( 0xF0, 0x01);      // set format B
  //  myCAM.wrSensorReg8_16( 0xC6, 0xA77E);
  //  myCAM.wrSensorReg8_16( 0xC8, 0X0000);


  myCAM.wrSensorReg8_16( 0xF0, 0x01);      // switch to JPEG settings
  myCAM.wrSensorReg8_16( 0xC6, 0x270B);
  myCAM.wrSensorReg8_16( 0xC8, 0X0010);


  myCAM.wrSensorReg8_16( 0xF0, 0x01);
  myCAM.wrSensorReg8_16( 0xC6, 0x2772);
  myCAM.wrSensorReg8_16( 0xC8, 0X0067);

  myCAM.wrSensorReg8_16( 0xF0, 0x01);
  myCAM.wrSensorReg8_16( 0xC6, 0x2774);
  myCAM.wrSensorReg8_16( 0xC8, 0X0406);


  myCAM.wrSensorReg8_16( 0xF0, 0x01);
  myCAM.wrSensorReg8_16( 0xC6, 0xA776);
  myCAM.wrSensorReg8_16( 0xC8, 0X0002);



  myCAM.wrSensorReg8_16( 0xF0, 0x01);
  myCAM.wrSensorReg8_16( 0xC6, 0x2908);
  myCAM.wrSensorReg8_16( 0xC8, restart_int);  // set restart interval


  myCAM.wrSensorReg8_16( 0xF0, 0x01);
  myCAM.wrSensorReg8_16( 0xC6, 0xA907);
  myCAM.wrSensorReg8_16( 0xC8, 0X0010);     // enable scaled quantization


  refreshMode();
  refresh();

  myCAM.wrSensorReg8_16( 0xF0, 0x01);
  myCAM.wrSensorReg8_16( 0xC6, 0xA120);
  myCAM.wrSensorReg8_16( 0xC8, 0X0002);


  myCAM.wrSensorReg8_16( 0xF0, 0x01);
  myCAM.wrSensorReg8_16( 0xC6, 0xA103);
  myCAM.wrSensorReg8_16( 0xC8, 0X0002);

}

void mirror() {
  uint16_t value;

  Serial.println("Mirror");

  myCAM.wrSensorReg8_16( 0xF0, 0x00);
  myCAM.rdSensorReg8_16( 0x20, &value);
  value |= 0x0001;
  myCAM.wrSensorReg8_16( 0x20, value);

  refresh();
}


void toggleTestpattern() {
  uint16_t value;

  Serial.println("Toggle testpattern");

  myCAM.wrSensorReg8_16( 0xF0, 0x01);
  myCAM.rdSensorReg8_16( 0x48, &value);
  Serial.print("setting : ");
  Serial.println(value, HEX);
  if ( value == 0x0000) {
    myCAM.wrSensorReg8_16( 0x48, 0x0003);
  }
  else {
    myCAM.wrSensorReg8_16( 0x48, 0x0000);
  }
  myCAM.rdSensorReg8_16( 0x48, &value);
  Serial.print("setting : ");
  Serial.println(value, HEX);

  refresh();
}


void setup()
{
#if defined (__AVR__)
  Wire.begin();
#endif
#if defined(__arm__)
  Wire1.begin();
#endif
  Serial.begin(115200);
  Serial.println("hello");

  // set the slaveSelectPin as an output:
  pinMode(CS, OUTPUT);

  // initialize SPI:
  SPI.begin();
  myCAM.write_reg(ARDUCHIP_MODE, 0x00);
  myGLCD.InitLCD();

  myCAM.InitCAM();

  resetCAM();
  setFormats();
  //setSizes(0x140, 0x0f0, width, height);
  setSizes(width, height, width, height + OFFSET);
  load_patch_segment();
  call_vmt(0x52a);


  //    int mclk = 25000000;
  //    int scale = 65536;
  uint16_t addr, v;
  addr = 0x2611;
  myCAM.wrSensorReg8_16(0xF0, 0x01);
  myCAM.wrSensorReg8_16(0xC6, addr);
  //    v = (uint16_t) (mclk/scale);
  v = 0x17D;
  Serial.println(v, HEX);
  myCAM.wrSensorReg8_16(0xC8, v);
  myCAM.wrSensorReg8_16(0xC6, addr);
  myCAM.rdSensorReg8_16(0xC8, &v);
  Serial.println(v, HEX);


  delayMicroseconds(600000);;  //minimum delay 600 msec

  AF_mode();

  myCAM.wrSensorReg8_16(0xf0, 0x01);
  myCAM.wrSensorReg8_16(0xc6, 0x810D);
  myCAM.wrSensorReg8_16(0xc8, 0x0002);
  myCAM.wrSensorReg8_16(0xc6, 0x810E);
  myCAM.wrSensorReg8_16(0xc8, 0x00c4);


  call_init(0x400);

  Serial.println("end patch loading");

  doCapture(0x20);

  //Initialize SD Card
  if (!SD.begin(SD_CS))
  {
    //while (1);		//If failed, stop here
    Serial.println("SD card failed");
  }
  Serial.println("init done");
}

void loop()
{
  char str[8];
  static int k = 0;
  uint8_t temp;
  uint16_t value, length_low, length_high;
  uint32_t length;
  int time = 0;
  myCAM.write_reg(ARDUCHIP_MODE, 0x01);		 			//Switch to CAM

  while (1)
  {
    if (Serial.available()) {
      temp = Serial.read();
      if (temp == 't') {
        toggleTestpattern();
      }
      if (temp == 'f') {
        AF_refocus();
        Serial.println("AF refocus");
      }
    }

    temp = myCAM.read_reg(ARDUCHIP_TRIG);
    time++;
    if (!(temp & VSYNC_MASK))				 			//New Frame is coming
    {
      //      Serial.print("VSYNC ");
      //      Serial.println(time);
      time = 0;
      myCAM.write_reg(ARDUCHIP_MODE, 0x00);    		//Switch to MCU
      myGLCD.resetXY();
      myCAM.write_reg(ARDUCHIP_MODE, 0x01);    		//Switch to CAM
      while (!(myCAM.read_reg(ARDUCHIP_TRIG) & 0x01)) {
        time++;
      } 	                                        //Wait for VSYNC is gone
      //      Serial.print("VSYNC gone ");
      //      Serial.println(time);
      time = 0;
    }
    else if (temp & SHUTTER_MASK)
    {
      AF_refocus();
      delay(4000);
      Serial.println("trigger");
      k = k + 1;
      strcat(str, "mt9d111");
      itoa(k, str, 10);
      strcat(str, ".jpg");								//Generate file name
      myCAM.write_reg(ARDUCHIP_MODE, 0x00);    		//Switch to MCU, freeze the screen

      GrabImage(str);
      Serial.println("grab done");
    }
  }
}


void GrabImage(char* str)
{
  File outFile;
  int i;
  uint16_t k1 = 0;
  uint16_t k2 = 0;
  uint16_t length_high1;
  uint16_t length_low1;
  char ch;
  
  myCAM.wrSensorReg8_16( 0xF0, 0x01);
  myCAM.wrSensorReg8_16( 0xC6, 0xA90f);
  myCAM.rdSensorReg8_16( 0xC8, &length_high1);
  //Serial.print("jpeg image length high ");
  //Serial.println(length_high1, HEX);

  myCAM.wrSensorReg8_16( 0xF0, 0x01);
  myCAM.wrSensorReg8_16( 0xC6, 0x2910);
  myCAM.rdSensorReg8_16( 0xC8, &length_low1);
  //Serial.print("jpeg image length low ");
  //Serial.println(length_low1, HEX);
  
  Serial.println(str);
  outFile = SD.open(str, O_WRITE | O_CREAT | O_TRUNC );  // overwrite if file exists
  if (! outFile)
  {
    Serial.println("Open File Error");
    return;
  }

  //Switch to FIFO Mode
  myCAM.write_reg(ARDUCHIP_TIM, 0x10);
  //Flush the FIFO
  myCAM.flush_fifo();
  //Start capture
  myCAM.start_capture();

  //Polling the capture done flag
  while (!(myCAM.read_reg(ARDUCHIP_TRIG) & CAP_DONE_MASK));

  //Write the JPEG header
  for ( i = 0; i < 0x9f; i++)
  {
    ch = pgm_read_byte(&JPEG_header[i]);
    outFile.write((uint8_t*)&ch, 1);
    Serial.write(ch);
  }

  ch = (uint8_t) (height >> 8);
  outFile.write((uint8_t*)&ch, 1);
  Serial.write(ch);
  ch = (uint8_t) (height & 0xFF);
  outFile.write((uint8_t*)&ch, 1);
  Serial.write(ch);
  ch = (uint8_t) (width >> 8);
  outFile.write((uint8_t*)&ch, 1);
  Serial.write(ch);
  ch = (uint8_t) (width & 0xFF);
  outFile.write((uint8_t*)&ch, 1);
  Serial.write(ch);
  for ( i = 0xA3; i < JPEGIMAGEOFFSET; i++)
  {
    ch = pgm_read_byte(&JPEG_header[i]);
    outFile.write((uint8_t*)&ch, 1);
    Serial.write(ch);
  }

  for ( k2 = 0; k2 < length_high1; k2++)
  {
    for ( k1 = 0; k1 < 0xffff; k1++) {
      ch = myCAM.read_fifo();
      outFile.write((uint8_t*)&ch, 1);
      Serial.write(ch);
    }
  }
  for ( k1 = 0; k1 < length_low1; k1++) {
    ch = myCAM.read_fifo();
    outFile.write((uint8_t*)&ch, 1);
    Serial.write(ch);
  }
  ch = (uint8_t) (0xFF);
  outFile.write((uint8_t*)&ch, 1);
  Serial.write(ch);
  ch = (uint8_t) (0xD9);
  outFile.write((uint8_t*)&ch, 1);
  Serial.write(ch);
  //Close the file
  outFile.close();

  //Clear the capture done flag
  myCAM.clear_fifo_flag();

  //Switch to LCD Mode
  myCAM.write_reg(ARDUCHIP_TIM, 0);
  Serial.println("file written");
  return;
}