/*
 *  Copyright (c) 2010 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#include <math.h>
#include "vpx_ports/config.h"
#include "vp8/common/idct.h"

#if CONFIG_HYBRIDTRANSFORM

#include "vp8/common/blockd.h"

// TODO: these transforms can be converted into integer forms to reduce
//       the complexity
float dct_4[16] = {
  0.500000000000000,  0.500000000000000,  0.500000000000000,  0.500000000000000,
  0.653281482438188,  0.270598050073099, -0.270598050073099, -0.653281482438188,
  0.500000000000000, -0.500000000000000, -0.500000000000000,  0.500000000000000,
  0.270598050073099, -0.653281482438188,  0.653281482438188, -0.270598050073099
};

float adst_4[16] = {
  0.228013428883779,  0.428525073124360,  0.577350269189626,  0.656538502008139,
  0.577350269189626,  0.577350269189626,  0.000000000000000, -0.577350269189626,
  0.656538502008139, -0.228013428883779, -0.577350269189626,  0.428525073124359,
  0.428525073124360, -0.656538502008139,  0.577350269189626, -0.228013428883779
};
#endif

#if CONFIG_HYBRIDTRANSFORM8X8
float dct_8[64] = {
  0.353553390593274,   0.353553390593274,   0.353553390593274,   0.353553390593274,
  0.353553390593274,   0.353553390593274,   0.353553390593274,   0.353553390593274,
  0.490392640201615,   0.415734806151273,   0.277785116509801,   0.097545161008064,
 -0.097545161008064,  -0.277785116509801,  -0.415734806151273,  -0.490392640201615,
  0.461939766255643,   0.191341716182545,  -0.191341716182545,  -0.461939766255643,
 -0.461939766255643,  -0.191341716182545,   0.191341716182545,   0.461939766255643,
  0.415734806151273,  -0.097545161008064,  -0.490392640201615,  -0.277785116509801,
  0.277785116509801,   0.490392640201615,   0.097545161008064,  -0.415734806151273,
  0.353553390593274,  -0.353553390593274,  -0.353553390593274,   0.353553390593274,
  0.353553390593274,  -0.353553390593274,  -0.353553390593274,   0.353553390593274,
  0.277785116509801,  -0.490392640201615,   0.097545161008064,   0.415734806151273,
 -0.415734806151273,  -0.097545161008064,   0.490392640201615,  -0.277785116509801,
  0.191341716182545,  -0.461939766255643,   0.461939766255643,  -0.191341716182545,
 -0.191341716182545,   0.461939766255643,  -0.461939766255643,   0.191341716182545,
  0.097545161008064,  -0.277785116509801,   0.415734806151273,  -0.490392640201615,
  0.490392640201615,  -0.415734806151273,   0.277785116509801,  -0.097545161008064
};

float adst_8[64] = {
  0.089131608307533,   0.175227946595735,   0.255357107325376,   0.326790388032145,
  0.387095214016349,   0.434217976756762,   0.466553967085785,   0.483002021635509,
  0.255357107325376,   0.434217976756762,   0.483002021635509,   0.387095214016349,
  0.175227946595735,  -0.089131608307533,  -0.326790388032145,  -0.466553967085785,
  0.387095214016349,   0.466553967085785,   0.175227946595735,  -0.255357107325376,
 -0.483002021635509,  -0.326790388032145,   0.089131608307533,   0.434217976756762,
  0.466553967085785,   0.255357107325376,  -0.326790388032145,  -0.434217976756762,
  0.089131608307533,   0.483002021635509,   0.175227946595735,  -0.387095214016348,
  0.483002021635509,  -0.089131608307533,  -0.466553967085785,   0.175227946595735,
  0.434217976756762,  -0.255357107325376,  -0.387095214016348,   0.326790388032145,
  0.434217976756762,  -0.387095214016348,  -0.089131608307533,   0.466553967085786,
 -0.326790388032145,  -0.175227946595735,   0.483002021635509,  -0.255357107325375,
  0.326790388032145,  -0.483002021635509,   0.387095214016349,  -0.089131608307534,
 -0.255357107325377,   0.466553967085785,  -0.434217976756762,   0.175227946595736,
  0.175227946595735,  -0.326790388032145,   0.434217976756762,  -0.483002021635509,
  0.466553967085785,  -0.387095214016348,   0.255357107325376,  -0.089131608307532
};
#endif

static const int xC1S7 = 16069;
static const int xC2S6 = 15137;
static const int xC3S5 = 13623;
static const int xC4S4 = 11585;
static const int xC5S3 =  9102;
static const int xC6S2 =  6270;
static const int xC7S1 =  3196;

#define SHIFT_BITS 14
#define DOROUND(X) X += (1<<(SHIFT_BITS-1));

#define FINAL_SHIFT 3
#define FINAL_ROUNDING (1<<(FINAL_SHIFT -1))
#define IN_SHIFT (FINAL_SHIFT+1)


void vp8_short_fdct8x8_c(short *InputData, short *OutputData, int pitch) {
  int loop;
  int short_pitch = pitch >> 1;
  int is07, is12, is34, is56;
  int is0734, is1256;
  int id07, id12, id34, id56;
  int irot_input_x, irot_input_y;
  int icommon_product1;      // Re-used product  (c4s4 * (s12 - s56))
  int icommon_product2;      // Re-used product  (c4s4 * (d12 + d56))
  int temp1, temp2;          // intermediate variable for computation

  int  InterData[64];
  int  *ip = InterData;
  short *op = OutputData;

  for (loop = 0; loop < 8; loop++) {
    // Pre calculate some common sums and differences.
    is07 = (InputData[0] + InputData[7]) << IN_SHIFT;
    is12 = (InputData[1] + InputData[2]) << IN_SHIFT;
    is34 = (InputData[3] + InputData[4]) << IN_SHIFT;
    is56 = (InputData[5] + InputData[6]) << IN_SHIFT;
    id07 = (InputData[0] - InputData[7]) << IN_SHIFT;
    id12 = (InputData[1] - InputData[2]) << IN_SHIFT;
    id34 = (InputData[3] - InputData[4]) << IN_SHIFT;
    id56 = (InputData[5] - InputData[6]) << IN_SHIFT;

    is0734 = is07 + is34;
    is1256 = is12 + is56;

    // Pre-Calculate some common product terms.
    icommon_product1 = xC4S4 * (is12 - is56);
    DOROUND(icommon_product1)
    icommon_product1 >>= SHIFT_BITS;

    icommon_product2 = xC4S4 * (id12 + id56);
    DOROUND(icommon_product2)
    icommon_product2 >>= SHIFT_BITS;


    ip[0] = (xC4S4 * (is0734 + is1256));
    DOROUND(ip[0]);
    ip[0] >>= SHIFT_BITS;

    ip[4] = (xC4S4 * (is0734 - is1256));
    DOROUND(ip[4]);
    ip[4] >>= SHIFT_BITS;

    // Define inputs to rotation for outputs 2 and 6
    irot_input_x = id12 - id56;
    irot_input_y = is07 - is34;

    // Apply rotation for outputs 2 and 6.
    temp1 = xC6S2 * irot_input_x;
    DOROUND(temp1);
    temp1 >>= SHIFT_BITS;
    temp2 = xC2S6 * irot_input_y;
    DOROUND(temp2);
    temp2 >>= SHIFT_BITS;
    ip[2] = temp1 + temp2;

    temp1 = xC6S2 * irot_input_y;
    DOROUND(temp1);
    temp1 >>= SHIFT_BITS;
    temp2 = xC2S6 * irot_input_x;
    DOROUND(temp2);
    temp2 >>= SHIFT_BITS;
    ip[6] = temp1 - temp2;

    // Define inputs to rotation for outputs 1 and 7
    irot_input_x = icommon_product1 + id07;
    irot_input_y = -(id34 + icommon_product2);

    // Apply rotation for outputs 1 and 7.
    temp1 = xC1S7 * irot_input_x;
    DOROUND(temp1);
    temp1 >>= SHIFT_BITS;
    temp2 = xC7S1 * irot_input_y;
    DOROUND(temp2);
    temp2 >>= SHIFT_BITS;
    ip[1] = temp1 - temp2;

    temp1 = xC7S1 * irot_input_x;
    DOROUND(temp1);
    temp1 >>= SHIFT_BITS;
    temp2 = xC1S7 * irot_input_y;
    DOROUND(temp2);
    temp2 >>= SHIFT_BITS;
    ip[7] = temp1 + temp2;

    // Define inputs to rotation for outputs 3 and 5
    irot_input_x = id07 - icommon_product1;
    irot_input_y = id34 - icommon_product2;

    // Apply rotation for outputs 3 and 5.
    temp1 = xC3S5 * irot_input_x;
    DOROUND(temp1);
    temp1 >>= SHIFT_BITS;
    temp2 = xC5S3 * irot_input_y;
    DOROUND(temp2);
    temp2 >>= SHIFT_BITS;
    ip[3] = temp1 - temp2;


    temp1 = xC5S3 * irot_input_x;
    DOROUND(temp1);
    temp1 >>= SHIFT_BITS;
    temp2 = xC3S5 * irot_input_y;
    DOROUND(temp2);
    temp2 >>= SHIFT_BITS;
    ip[5] = temp1 + temp2;

    // Increment data pointer for next row
    InputData += short_pitch;
    ip += 8;
  }

  // Performed DCT on rows, now transform the columns
  ip = InterData;
  for (loop = 0; loop < 8; loop++) {
    // Pre calculate some common sums and differences.
    is07 = ip[0 * 8] + ip[7 * 8];
    is12 = ip[1 * 8] + ip[2 * 8];
    is34 = ip[3 * 8] + ip[4 * 8];
    is56 = ip[5 * 8] + ip[6 * 8];

    id07 = ip[0 * 8] - ip[7 * 8];
    id12 = ip[1 * 8] - ip[2 * 8];
    id34 = ip[3 * 8] - ip[4 * 8];
    id56 = ip[5 * 8] - ip[6 * 8];

    is0734 = is07 + is34;
    is1256 = is12 + is56;

    // Pre-Calculate some common product terms
    icommon_product1 = xC4S4 * (is12 - is56);
    icommon_product2 = xC4S4 * (id12 + id56);
    DOROUND(icommon_product1)
    DOROUND(icommon_product2)
    icommon_product1 >>= SHIFT_BITS;
    icommon_product2 >>= SHIFT_BITS;


    temp1 = xC4S4 * (is0734 + is1256);
    temp2 = xC4S4 * (is0734 - is1256);
    DOROUND(temp1);
    DOROUND(temp2);
    temp1 >>= SHIFT_BITS;

    temp2 >>= SHIFT_BITS;
    op[0 * 8] = (temp1 + FINAL_ROUNDING) >> FINAL_SHIFT;
    op[4 * 8] = (temp2 + FINAL_ROUNDING) >> FINAL_SHIFT;

    // Define inputs to rotation for outputs 2 and 6
    irot_input_x = id12 - id56;
    irot_input_y = is07 - is34;

    // Apply rotation for outputs 2 and 6.
    temp1 = xC6S2 * irot_input_x;
    DOROUND(temp1);
    temp1 >>= SHIFT_BITS;
    temp2 = xC2S6 * irot_input_y;
    DOROUND(temp2);
    temp2 >>= SHIFT_BITS;
    op[2 * 8] = (temp1 + temp2 + FINAL_ROUNDING) >> FINAL_SHIFT;

    temp1 = xC6S2 * irot_input_y;
    DOROUND(temp1);
    temp1 >>= SHIFT_BITS;
    temp2 = xC2S6 * irot_input_x;
    DOROUND(temp2);
    temp2 >>= SHIFT_BITS;
    op[6 * 8] = (temp1 - temp2 + FINAL_ROUNDING) >> FINAL_SHIFT;

    // Define inputs to rotation for outputs 1 and 7
    irot_input_x = icommon_product1 + id07;
    irot_input_y = -(id34 + icommon_product2);

    // Apply rotation for outputs 1 and 7.
    temp1 = xC1S7 * irot_input_x;
    DOROUND(temp1);
    temp1 >>= SHIFT_BITS;
    temp2 = xC7S1 * irot_input_y;
    DOROUND(temp2);
    temp2 >>= SHIFT_BITS;
    op[1 * 8] = (temp1 - temp2 + FINAL_ROUNDING) >> FINAL_SHIFT;

    temp1 = xC7S1 * irot_input_x;
    DOROUND(temp1);
    temp1 >>= SHIFT_BITS;
    temp2 = xC1S7 * irot_input_y;
    DOROUND(temp2);
    temp2 >>= SHIFT_BITS;
    op[7 * 8] = (temp1 + temp2 + FINAL_ROUNDING) >> FINAL_SHIFT;

    // Define inputs to rotation for outputs 3 and 5
    irot_input_x = id07 - icommon_product1;
    irot_input_y = id34 - icommon_product2;

    // Apply rotation for outputs 3 and 5.
    temp1 = xC3S5 * irot_input_x;
    DOROUND(temp1);
    temp1 >>= SHIFT_BITS;
    temp2 = xC5S3 * irot_input_y;
    DOROUND(temp2);
    temp2 >>= SHIFT_BITS;
    op[3 * 8] = (temp1 - temp2 + FINAL_ROUNDING) >> FINAL_SHIFT;


    temp1 = xC5S3 * irot_input_x;
    DOROUND(temp1);
    temp1 >>= SHIFT_BITS;
    temp2 = xC3S5 * irot_input_y;
    DOROUND(temp2);
    temp2 >>= SHIFT_BITS;
    op[5 * 8] = (temp1 + temp2 + FINAL_ROUNDING) >> FINAL_SHIFT;

    // Increment data pointer for next column.
    ip++;
    op++;
  }
}

void vp8_short_fhaar2x2_c(short *input, short *output, int pitch) { // pitch = 8
  /* [1 1; 1 -1] orthogonal transform */
  /* use position: 0,1, 4, 8 */
  int i;
  short *ip1 = input;
  short *op1 = output;
  for (i = 0; i < 16; i++) {
    op1[i] = 0;
  }

  op1[0] = (ip1[0] + ip1[1] + ip1[4] + ip1[8] + 1) >> 1;
  op1[1] = (ip1[0] - ip1[1] + ip1[4] - ip1[8]) >> 1;
  op1[4] = (ip1[0] + ip1[1] - ip1[4] - ip1[8]) >> 1;
  op1[8] = (ip1[0] - ip1[1] - ip1[4] + ip1[8]) >> 1;

}

#if CONFIG_HYBRIDTRANSFORM
void vp8_fht4x4_c(short *input, short *output, int pitch, TX_TYPE tx_type) {
  int i, j, k;
  float bufa[16], bufb[16]; // buffers are for floating-point test purpose
                             // the implementation could be simplified in
                             // conjunction with integer transform
  short *ip = input;
  short *op = output;

  float *pfa = &bufa[0];
  float *pfb = &bufb[0];

  // pointers to vertical and horizontal transforms
  float *ptv, *pth;

  // load and convert residual array into floating-point
  for(j = 0; j < 4; j++) {
    for(i = 0; i < 4; i++) {
      pfa[i] = (float)ip[i];
    }
    pfa += 4;
    ip  += pitch / 2;
  }

  // vertical transformation
  pfa = &bufa[0];
  pfb = &bufb[0];

  switch(tx_type) {
    case ADST_ADST :
    case ADST_DCT  :
      ptv = &adst_4[0];
      break;

    default :
      ptv = &dct_4[0];
      break;
  }

  for(j = 0; j < 4; j++) {
    for(i = 0; i < 4; i++) {
      pfb[i] = 0;
      for(k = 0; k < 4; k++) {
        pfb[i] += ptv[k] * pfa[(k<<2)];
      }
      pfa += 1;
    }
    pfb += 4;
    ptv += 4;
    pfa = &bufa[0];
  }

  // horizontal transformation
  pfa = &bufa[0];
  pfb = &bufb[0];

  switch(tx_type) {
    case ADST_ADST :
    case  DCT_ADST :
      pth = &adst_4[0];
      break;

    default :
      pth = &dct_4[0];
      break;
  }

  for(j = 0; j < 4; j++) {
    for(i = 0; i < 4; i++) {
      pfa[i] = 0;
      for(k = 0; k < 4; k++) {
        pfa[i] += pfb[k] * pth[k];
      }
      pth += 4;
     }

    pfa += 4;
    pfb += 4;

    switch(tx_type) {
      case ADST_ADST :
      case  DCT_ADST :
        pth = &adst_4[0];
        break;

      default :
        pth = &dct_4[0];
        break;
    }
  }

  // convert to short integer format and load BLOCKD buffer
  op  = output ;
  pfa = &bufa[0] ;

  for(j = 0; j < 4; j++) {
    for(i = 0; i < 4; i++) {
      op[i] = (pfa[i] > 0 ) ? (short)( 8 * pfa[i] + 0.49) :
                                   -(short)(- 8 * pfa[i] + 0.49);
    }
    op  += 4;
    pfa += 4;
  }
}
#endif

#if CONFIG_HYBRIDTRANSFORM8X8
void vp8_fht8x8_c(short *input, short *output, int pitch, TX_TYPE tx_type) {
  int i, j, k;
  float bufa[64], bufb[64]; // buffers are for floating-point test purpose
                             // the implementation could be simplified in
                             // conjunction with integer transform
  short *ip = input;
  short *op = output;

  float *pfa = &bufa[0];
  float *pfb = &bufb[0];

  // pointers to vertical and horizontal transforms
  float *ptv, *pth;

  // load and convert residual array into floating-point
  for(j = 0; j < 8; j++) {
    for(i = 0; i < 8; i++) {
      pfa[i] = (float)ip[i];
    }
    pfa += 8;
    ip  += pitch / 2;
  }

  // vertical transformation
  pfa = &bufa[0];
  pfb = &bufb[0];

  switch(tx_type) {
    case ADST_ADST :
    case ADST_DCT  :
      ptv = &adst_8[0];
      break;

    default :
      ptv = &dct_8[0];
      break;
  }

  for(j = 0; j < 8; j++) {
    for(i = 0; i < 8; i++) {
      pfb[i] = 0;
      for(k = 0; k < 8; k++) {
        pfb[i] += ptv[k] * pfa[(k<<3)];
      }
      pfa += 1;
    }
    pfb += 8;
    ptv += 8;
    pfa = &bufa[0];
  }

  // horizontal transformation
  pfa = &bufa[0];
  pfb = &bufb[0];

  switch(tx_type) {
    case ADST_ADST :
    case  DCT_ADST :
      pth = &adst_8[0];
      break;

    default :
      pth = &dct_8[0];
      break;
  }

  for(j = 0; j < 8; j++) {
    for(i = 0; i < 8; i++) {
      pfa[i] = 0;
      for(k = 0; k < 8; k++) {
        pfa[i] += pfb[k] * pth[k];
      }
      pth += 8;
     }

    pfa += 8;
    pfb += 8;

    switch(tx_type) {
      case ADST_ADST :
      case  DCT_ADST :
        pth = &adst_8[0];
        break;

      default :
        pth = &dct_8[0];
        break;
    }
  }

  // convert to short integer format and load BLOCKD buffer
  op  = output ;
  pfa = &bufa[0] ;

  for(j = 0; j < 8; j++) {
    for(i = 0; i < 8; i++) {
      op[i] = (pfa[i] > 0 ) ? (short)( 8 * pfa[i] + 0.49) :
                                   -(short)(- 8 * pfa[i] + 0.49);
    }
    op  += 8;
    pfa += 8;
  }
}
#endif

void vp8_short_fdct4x4_c(short *input, short *output, int pitch) {
  int i;
  int a1, b1, c1, d1;
  short *ip = input;
  short *op = output;

  for (i = 0; i < 4; i++) {
    a1 = ((ip[0] + ip[3]) << 5);
    b1 = ((ip[1] + ip[2]) << 5);
    c1 = ((ip[1] - ip[2]) << 5);
    d1 = ((ip[0] - ip[3]) << 5);

    op[0] = a1 + b1;
    op[2] = a1 - b1;

    op[1] = (c1 * 2217 + d1 * 5352 +  14500) >> 12;
    op[3] = (d1 * 2217 - c1 * 5352 +   7500) >> 12;

    ip += pitch / 2;
    op += 4;

  }
  ip = output;
  op = output;
  for (i = 0; i < 4; i++) {
    a1 = ip[0] + ip[12];
    b1 = ip[4] + ip[8];
    c1 = ip[4] - ip[8];
    d1 = ip[0] - ip[12];

    op[0]  = (a1 + b1 + 7) >> 4;
    op[8]  = (a1 - b1 + 7) >> 4;

    op[4]  = ((c1 * 2217 + d1 * 5352 +  12000) >> 16) + (d1 != 0);
    op[12] = (d1 * 2217 - c1 * 5352 +  51000) >> 16;

    ip++;
    op++;
  }
}

#if CONFIG_HYBRIDTRANSFORM
void vp8_fht8x4_c(short *input, short *output, int pitch,
                  TX_TYPE tx_type) {
  vp8_fht4x4_c(input,     output,      pitch, tx_type);
  vp8_fht4x4_c(input + 4, output + 16, pitch, tx_type);
}
#endif

void vp8_short_fdct8x4_c(short *input, short *output, int pitch)
{
    vp8_short_fdct4x4_c(input,   output,    pitch);
    vp8_short_fdct4x4_c(input + 4, output + 16, pitch);
}

void vp8_short_walsh4x4_c(short *input, short *output, int pitch) {
  int i;
  int a1, b1, c1, d1;
  short *ip = input;
  short *op = output;
  int pitch_short = pitch >> 1;

  for (i = 0; i < 4; i++) {
    a1 = ip[0 * pitch_short] + ip[3 * pitch_short];
    b1 = ip[1 * pitch_short] + ip[2 * pitch_short];
    c1 = ip[1 * pitch_short] - ip[2 * pitch_short];
    d1 = ip[0 * pitch_short] - ip[3 * pitch_short];

    op[0] = (a1 + b1 + 1) >> 1;
    op[4] = (c1 + d1) >> 1;
    op[8] = (a1 - b1) >> 1;
    op[12] = (d1 - c1) >> 1;

    ip++;
    op++;
  }
  ip = output;
  op = output;

  for (i = 0; i < 4; i++) {
    a1 = ip[0] + ip[3];
    b1 = ip[1] + ip[2];
    c1 = ip[1] - ip[2];
    d1 = ip[0] - ip[3];

    op[0] = (a1 + b1 + 1) >> 1;
    op[1] = (c1 + d1) >> 1;
    op[2] = (a1 - b1) >> 1;
    op[3] = (d1 - c1) >> 1;

    ip += 4;
    op += 4;
  }
}

#if CONFIG_LOSSLESS
void vp8_short_walsh4x4_lossless_c(short *input, short *output, int pitch) {
  int i;
  int a1, b1, c1, d1;
  short *ip = input;
  short *op = output;
  int pitch_short = pitch >> 1;

  for (i = 0; i < 4; i++) {
    a1 = (ip[0 * pitch_short] + ip[3 * pitch_short]) >> Y2_WHT_UPSCALE_FACTOR;
    b1 = (ip[1 * pitch_short] + ip[2 * pitch_short]) >> Y2_WHT_UPSCALE_FACTOR;
    c1 = (ip[1 * pitch_short] - ip[2 * pitch_short]) >> Y2_WHT_UPSCALE_FACTOR;
    d1 = (ip[0 * pitch_short] - ip[3 * pitch_short]) >> Y2_WHT_UPSCALE_FACTOR;

    op[0] = (a1 + b1 + 1) >> 1;
    op[4] = (c1 + d1) >> 1;
    op[8] = (a1 - b1) >> 1;
    op[12] = (d1 - c1) >> 1;

    ip++;
    op++;
  }
  ip = output;
  op = output;

  for (i = 0; i < 4; i++) {
    a1 = ip[0] + ip[3];
    b1 = ip[1] + ip[2];
    c1 = ip[1] - ip[2];
    d1 = ip[0] - ip[3];

    op[0] = ((a1 + b1 + 1) >> 1) << Y2_WHT_UPSCALE_FACTOR;
    op[1] = ((c1 + d1) >> 1) << Y2_WHT_UPSCALE_FACTOR;
    op[2] = ((a1 - b1) >> 1) << Y2_WHT_UPSCALE_FACTOR;
    op[3] = ((d1 - c1) >> 1) << Y2_WHT_UPSCALE_FACTOR;

    ip += 4;
    op += 4;
  }
}

void vp8_short_walsh4x4_x8_c(short *input, short *output, int pitch) {
  int i;
  int a1, b1, c1, d1;
  short *ip = input;
  short *op = output;
  int pitch_short = pitch >> 1;

  for (i = 0; i < 4; i++) {
    a1 = ip[0 * pitch_short] + ip[3 * pitch_short];
    b1 = ip[1 * pitch_short] + ip[2 * pitch_short];
    c1 = ip[1 * pitch_short] - ip[2 * pitch_short];
    d1 = ip[0 * pitch_short] - ip[3 * pitch_short];

    op[0] = (a1 + b1 + 1) >> 1;
    op[4] = (c1 + d1) >> 1;
    op[8] = (a1 - b1) >> 1;
    op[12] = (d1 - c1) >> 1;

    ip++;
    op++;
  }
  ip = output;
  op = output;

  for (i = 0; i < 4; i++) {
    a1 = ip[0] + ip[3];
    b1 = ip[1] + ip[2];
    c1 = ip[1] - ip[2];
    d1 = ip[0] - ip[3];

    op[0] = ((a1 + b1 + 1) >> 1) << WHT_UPSCALE_FACTOR;
    op[1] = ((c1 + d1) >> 1) << WHT_UPSCALE_FACTOR;
    op[2] = ((a1 - b1) >> 1) << WHT_UPSCALE_FACTOR;
    op[3] = ((d1 - c1) >> 1) << WHT_UPSCALE_FACTOR;

    ip += 4;
    op += 4;
  }
}

void vp8_short_walsh8x4_x8_c(short *input, short *output, int pitch) {
  vp8_short_walsh4x4_x8_c(input,   output,    pitch);
  vp8_short_walsh4x4_x8_c(input + 4, output + 16, pitch);
}
#endif

#if CONFIG_TX16X16
static void dct16x16_1d(double input[16], double output[16]) {
  double step[16];
  double intermediate[16];
  double temp1, temp2;

  const double PI = 3.1415926535898;
  const double C1 = cos(1*PI/(double)32);
  const double C2 = cos(2*PI/(double)32);
  const double C3 = cos(3*PI/(double)32);
  const double C4 = cos(4*PI/(double)32);
  const double C5 = cos(5*PI/(double)32);
  const double C6 = cos(6*PI/(double)32);
  const double C7 = cos(7*PI/(double)32);
  const double C8 = cos(8*PI/(double)32);
  const double C9 = cos(9*PI/(double)32);
  const double C10 = cos(10*PI/(double)32);
  const double C11 = cos(11*PI/(double)32);
  const double C12 = cos(12*PI/(double)32);
  const double C13 = cos(13*PI/(double)32);
  const double C14 = cos(14*PI/(double)32);
  const double C15 = cos(15*PI/(double)32);

  // step 1
  step[ 0] = input[0] + input[15];
  step[ 1] = input[1] + input[14];
  step[ 2] = input[2] + input[13];
  step[ 3] = input[3] + input[12];
  step[ 4] = input[4] + input[11];
  step[ 5] = input[5] + input[10];
  step[ 6] = input[6] + input[ 9];
  step[ 7] = input[7] + input[ 8];
  step[ 8] = input[7] - input[ 8];
  step[ 9] = input[6] - input[ 9];
  step[10] = input[5] - input[10];
  step[11] = input[4] - input[11];
  step[12] = input[3] - input[12];
  step[13] = input[2] - input[13];
  step[14] = input[1] - input[14];
  step[15] = input[0] - input[15];

  // step 2
  output[0] = step[0] + step[7];
  output[1] = step[1] + step[6];
  output[2] = step[2] + step[5];
  output[3] = step[3] + step[4];
  output[4] = step[3] - step[4];
  output[5] = step[2] - step[5];
  output[6] = step[1] - step[6];
  output[7] = step[0] - step[7];

  temp1 = step[ 8]*C7;
  temp2 = step[15]*C9;
  output[ 8] = temp1 + temp2;

  temp1 = step[ 9]*C11;
  temp2 = step[14]*C5;
  output[ 9] = temp1 - temp2;

  temp1 = step[10]*C3;
  temp2 = step[13]*C13;
  output[10] = temp1 + temp2;

  temp1 = step[11]*C15;
  temp2 = step[12]*C1;
  output[11] = temp1 - temp2;

  temp1 = step[11]*C1;
  temp2 = step[12]*C15;
  output[12] = temp2 + temp1;

  temp1 = step[10]*C13;
  temp2 = step[13]*C3;
  output[13] = temp2 - temp1;

  temp1 = step[ 9]*C5;
  temp2 = step[14]*C11;
  output[14] = temp2 + temp1;

  temp1 = step[ 8]*C9;
  temp2 = step[15]*C7;
  output[15] = temp2 - temp1;

  // step 3
  step[ 0] = output[0] + output[3];
  step[ 1] = output[1] + output[2];
  step[ 2] = output[1] - output[2];
  step[ 3] = output[0] - output[3];

  temp1 = output[4]*C14;
  temp2 = output[7]*C2;
  step[ 4] = temp1 + temp2;

  temp1 = output[5]*C10;
  temp2 = output[6]*C6;
  step[ 5] = temp1 + temp2;

  temp1 = output[5]*C6;
  temp2 = output[6]*C10;
  step[ 6] = temp2 - temp1;

  temp1 = output[4]*C2;
  temp2 = output[7]*C14;
  step[ 7] = temp2 - temp1;

  step[ 8] = output[ 8] + output[11];
  step[ 9] = output[ 9] + output[10];
  step[10] = output[ 9] - output[10];
  step[11] = output[ 8] - output[11];

  step[12] = output[12] + output[15];
  step[13] = output[13] + output[14];
  step[14] = output[13] - output[14];
  step[15] = output[12] - output[15];

  // step 4
  output[ 0] = (step[ 0] + step[ 1]);
  output[ 8] = (step[ 0] - step[ 1]);

  temp1 = step[2]*C12;
  temp2 = step[3]*C4;
  temp1 = temp1 + temp2;
  output[ 4] = 2*(temp1*C8);

  temp1 = step[2]*C4;
  temp2 = step[3]*C12;
  temp1 = temp2 - temp1;
  output[12] = 2*(temp1*C8);

  output[ 2] = 2*((step[4] + step[ 5])*C8);
  output[14] = 2*((step[7] - step[ 6])*C8);

  temp1 = step[4] - step[5];
  temp2 = step[6] + step[7];
  output[ 6] = (temp1 + temp2);
  output[10] = (temp1 - temp2);

  intermediate[8] = step[8] + step[14];
  intermediate[9] = step[9] + step[15];

  temp1 = intermediate[8]*C12;
  temp2 = intermediate[9]*C4;
  temp1 = temp1 - temp2;
  output[3] = 2*(temp1*C8);

  temp1 = intermediate[8]*C4;
  temp2 = intermediate[9]*C12;
  temp1 = temp2 + temp1;
  output[13] = 2*(temp1*C8);

  output[ 9] = 2*((step[10] + step[11])*C8);

  intermediate[11] = step[10] - step[11];
  intermediate[12] = step[12] + step[13];
  intermediate[13] = step[12] - step[13];
  intermediate[14] = step[ 8] - step[14];
  intermediate[15] = step[ 9] - step[15];

  output[15] = (intermediate[11] + intermediate[12]);
  output[ 1] = -(intermediate[11] - intermediate[12]);

  output[ 7] = 2*(intermediate[13]*C8);

  temp1 = intermediate[14]*C12;
  temp2 = intermediate[15]*C4;
  temp1 = temp1 - temp2;
  output[11] = -2*(temp1*C8);

  temp1 = intermediate[14]*C4;
  temp2 = intermediate[15]*C12;
  temp1 = temp2 + temp1;
  output[ 5] = 2*(temp1*C8);
}

void vp8_short_fdct16x16_c(short *input, short *out, int pitch) {
    int shortpitch = pitch >> 1;
    int i, j;
    double output[256];
    // First transform columns
    for (i = 0; i < 16; i++) {
        double temp_in[16], temp_out[16];
        for (j = 0; j < 16; j++)
            temp_in[j] = input[j*shortpitch + i];
        dct16x16_1d(temp_in, temp_out);
        for (j = 0; j < 16; j++)
            output[j*16 + i] = temp_out[j];
    }
    // Then transform rows
    for (i = 0; i < 16; ++i) {
        double temp_in[16], temp_out[16];
        for (j = 0; j < 16; ++j)
            temp_in[j] = output[j + i*16];
        dct16x16_1d(temp_in, temp_out);
        for (j = 0; j < 16; ++j)
            output[j + i*16] = temp_out[j];
    }
    // Scale by some magic number
    for (i = 0; i < 256; i++)
        out[i] = (short)round(output[i]/2);
}
#endif
