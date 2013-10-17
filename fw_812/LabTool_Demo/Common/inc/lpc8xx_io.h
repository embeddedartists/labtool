/****************************************************************************
 *   $Id:: lpc8xx_io.h            
 *   Project: NXP LPC8xx validation   
 *
 *   Description:
 *     Include file for IOCON and SWM functionality
 *
 ****************************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
****************************************************************************/
#ifndef IO_H_FILE
#define IO_H_FILE

// tbdajs
#if 0
#include "systemlevel.h"
#else
#endif

#include "stdint.h"

typedef unsigned int   uint32_reg_t  ;
typedef enum {false, true} bool;

//////////////////////////////////////////////
//
// IO Allocaion style
//
////////////////////////////////////////////

#define  IO_ALLOC_IOCON  0ULL
#define  IO_ALLOC_SWM8   1ULL
#define  IO_ALLOC_SWM4   2ULL
#define  IO_ALLOC_SWM2   3ULL
#define  IO_ALLOC_SWM1   4ULL


//////////////////////////////////////////////
//
// Pad IDs
//
////////////////////////////////////////////

#define IO_BLNK     0xffULL    // Unimplemented 
#define SWM_RES     0xfeULL    // Allocated for SWM

//       Name            ID       Func0           Func1           Func2           Func3           Func4           Func5           Func6             

#define PAD_P0_0         0UL       +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56)    
#define PAD_P0_1         1UL       +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56)     
#define PAD_P0_2         2UL       +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56)     
#define PAD_P0_3         3UL       +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56)     
#define PAD_P0_4         4UL       +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56)     
#define PAD_P0_5         5UL       +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56)     
#define PAD_P0_6         6UL       +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56)     
#define PAD_P0_7         7UL       +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56)     
#define PAD_P0_8         8UL       +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56)     
#define PAD_P0_9         9UL       +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56)     
#define PAD_P0_10        10UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56)     
#define PAD_P0_11        11UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56)     
#define PAD_P0_12        12UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56)     
#define PAD_P0_13        13UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56)    
#define PAD_P0_14        14UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56)                                                                                                                       
#define PAD_P0_15        15UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56)                                                                                                                       
#define PAD_P0_16        16UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56)                                                                                                                       
#define PAD_P0_17        17UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56)                                                                                                                       
#define PAD_P0_18        18UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56)                                                                                                                       
#define PAD_P0_19        19UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56)                                                                                                                       
#define PAD_P0_20        20UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56)                                                                                                                       
#define PAD_P0_21        21UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56)                                                                                                                       
#define PAD_P0_22        22UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56)                                                                                                                       
#define PAD_P0_23        23UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56)                                                                                                                       
#define PAD_P0_24        24UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56)                                                                                                                       
#define PAD_P0_25        25UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56)                                                                                                                       
#define PAD_P0_26        26UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56)                                                                                                                       
#define PAD_P0_27        27UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56)                                                                                                                       
#define PAD_P0_28        28UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56)                                                                                                                       
#define PAD_P0_29        29UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56)                                                                                                                       
#define PAD_P0_30        30UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56)                                                                                                                       
#define PAD_P0_31        31UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56)                                                                                                                       

#define PAD_P1_0         32UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56) 
#define PAD_P1_1         33UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56) 
#define PAD_P1_2         34UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56) 
#define PAD_P1_3         35UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56) 
#define PAD_P1_4         36UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56) 
#define PAD_P1_5         37UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56) 
#define PAD_P1_6         38UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56) 
#define PAD_P1_7         39UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56) 
#define PAD_P1_8         40UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56) 
#define PAD_P1_9         41UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56) 
#define PAD_P1_10        42UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56) 
#define PAD_P1_11        43UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56) 
#define PAD_P1_12        44UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56) 
#define PAD_P1_13        45UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56) 
#define PAD_P1_14        46UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56) 
#define PAD_P1_15        47UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56) 
#define PAD_P1_16        48UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56) 
#define PAD_P1_17        49UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56) 
#define PAD_P1_18        50UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56) 
#define PAD_P1_19        51UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56) 
#define PAD_P1_20        52UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56) 
#define PAD_P1_21        53UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56) 
#define PAD_P1_22        54UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56) 
#define PAD_P1_23        55UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56) 
#define PAD_P1_24        56UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56) 
#define PAD_P1_25        57UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56) 
#define PAD_P1_26        58UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56) 
#define PAD_P1_27        59UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56) 
#define PAD_P1_28        60UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56) 
#define PAD_P1_29        61UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56) 
#define PAD_P1_30        62UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56) 
#define PAD_P1_31        63UL      +(SWM_RES<<8)  +(IO_BLNK<<24)   +(IO_BLNK<<24)  +(IO_BLNK<<32)  +(IO_BLNK<<40)  +(IO_BLNK<<48)  +(IO_BLNK<<56) 


////////////////////////////////////////////////////////////////////////////////////////////
//
//     Func IDs
//
////////////////////////////////////////////////////////////////////////////////////////////



//       Name             Alloc                    Func-num                    F2 - position0    F2 - position1  F2 - position2  

// GPIOs are allocated through attaching SWM and using unimplmented option, so no code needed


// F8 Function IDs

#define F_UART0_TXD         0UL         +(IO_ALLOC_SWM8<<56)     
#define F_UART0_RXD         1UL         +(IO_ALLOC_SWM8<<56)     
#define F_UART0_RTS_N       2UL         +(IO_ALLOC_SWM8<<56)     
#define F_UART0_CTS_N       3UL         +(IO_ALLOC_SWM8<<56)     
#define F_UART0_SCLK        4UL         +(IO_ALLOC_SWM8<<56)     
#define F_UART1_TXD         5UL         +(IO_ALLOC_SWM8<<56)     
#define F_UART1_RXD         6UL         +(IO_ALLOC_SWM8<<56)     
#define F_UART1_RTS_N       7UL         +(IO_ALLOC_SWM8<<56)     
#define F_UART1_CTS_N       8UL         +(IO_ALLOC_SWM8<<56)     
#define F_UART1_SCLK        9UL         +(IO_ALLOC_SWM8<<56)     
#define F_UART2_TXD         10UL        +(IO_ALLOC_SWM8<<56)     
#define F_UART2_RXD         11UL        +(IO_ALLOC_SWM8<<56)     
#define F_UART2_RTS_N       12UL        +(IO_ALLOC_SWM8<<56)     
#define F_UART2_CTS_N       13UL        +(IO_ALLOC_SWM8<<56)     
#define F_UART2_SCLK        14UL        +(IO_ALLOC_SWM8<<56)     
#define F_LSPI0_SCK         15UL        +(IO_ALLOC_SWM8<<56)     
#define F_LSPI0_MOSI        16UL        +(IO_ALLOC_SWM8<<56)     
#define F_LSPI0_MISO        17UL        +(IO_ALLOC_SWM8<<56)     
#define F_LSPI0_SSEL        18UL        +(IO_ALLOC_SWM8<<56)     
#define F_LSPI1_SCK         19UL        +(IO_ALLOC_SWM8<<56)     
#define F_LSPI1_MOSI        20UL        +(IO_ALLOC_SWM8<<56)     
#define F_LSPI1_MISO        21UL        +(IO_ALLOC_SWM8<<56)     
#define F_LSPI1_SSEL        22UL        +(IO_ALLOC_SWM8<<56)     
#define F_SCT0_IN0          23UL        +(IO_ALLOC_SWM8<<56)     
#define F_SCT0_IN1          24UL        +(IO_ALLOC_SWM8<<56)     
#define F_SCT0_IN2          25UL        +(IO_ALLOC_SWM8<<56)     
#define F_SCT0_IN3          26UL        +(IO_ALLOC_SWM8<<56)     
#define F_SCT0_OUT0         27UL        +(IO_ALLOC_SWM8<<56)     
#define F_SCT0_OUT1         28UL        +(IO_ALLOC_SWM8<<56)     
#define F_SCT0_OUT2         29UL        +(IO_ALLOC_SWM8<<56)     
#define F_SCT0_OUT3         30UL        +(IO_ALLOC_SWM8<<56)     
#define F_I2C0_SDA          31UL        +(IO_ALLOC_SWM8<<56)     
#define F_I2C0_SCL          32UL        +(IO_ALLOC_SWM8<<56)     
#define F_COMP0_OUT         33UL        +(IO_ALLOC_SWM8<<56)     
#define F_CLKOUT            34UL        +(IO_ALLOC_SWM8<<56)     
#define F_GPIO_INT_BMATCH   35UL        +(IO_ALLOC_SWM8<<56)     


// F1 Function IDs
#define F_COMP_0A           0UL         +(IO_ALLOC_SWM1<<56)      
#define F_COMP_0B           1UL         +(IO_ALLOC_SWM1<<56)      
#define F_SWCLK             2UL         +(IO_ALLOC_SWM1<<56)      
#define F_SWDIO             3UL         +(IO_ALLOC_SWM1<<56)      
#define F_XTAL1             4UL         +(IO_ALLOC_SWM1<<56)      
#define F_XTAL2             5UL         +(IO_ALLOC_SWM1<<56)      
#define F_RESETN            6UL         +(IO_ALLOC_SWM1<<56)      
#define F_CLKIN             7UL         +(IO_ALLOC_SWM1<<56)      
#define F_COMP VREF         8UL         +(IO_ALLOC_SWM1<<56)      


////////////////////////////////////////////////////////////////////////////////////////////
//
//     IOCON regs/format 
//
////////////////////////////////////////////////////////////////////////////////////////////


typedef struct {
  __IO uint32_reg_t pio[78]        ; //(`IOCON + 32'h000) 
  __IO uint32_reg_t reserved[0x2e] ; //(`IOCON + 32'h134)
  __IO uint32_reg_t ovp ;            //(`IOCON + 32'h1f0) 
} LPC_IOCON_Type;

// bit definitions
#define IOCON_FUNCTION(d)       (d<<0)
#define IOCON_PARAMETER(d)      (d<<3)

////////////////////////////////////////////////////////////////////////////////////////////
//
//    SWM regs/format 
//
////////////////////////////////////////////////////////////////////////////////////////////


typedef struct {
  __IO uint32_reg_t f8_set[64]      ; //(`SWM + 32'h000)   - continuous until 0x1000   (in bits 8 downto 5)
  __IO uint32_reg_t f4_set[32]      ; //(`SWM + 32'h000)   - continuous until 0x1100   (in bits 8 downto 5)
  __IO uint32_reg_t f2_set[16]       ; //(`SWM + 32'h000)   - continuous until 0x1110   (in bits 8 downto 5)
  __IO uint32_reg_t f1_set[8]       ; //(`SWM + 32'h000)   - continuous until 0x1111   (in bits 8 downto 5)
} LPC_SWM_Type;



////////////////////////////////////////////////////////////////////////////////////////////
//
//     IO service
//
////////////////////////////////////////////////////////////////////////////////////////////

  int attachIO (uint64_t func, uint64_t pad, bool enable);   // Function for configuring IO for a chip function
  int cfgIO    (uint64_t pad, uint32_t param);                     // Function for setting pad parameters



#endif //IO_H_FILE
  
