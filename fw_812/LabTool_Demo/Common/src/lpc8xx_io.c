/****************************************************************************
 *   $Id:: lpc8xx_io.c            
 *   Project: NXP LPC8xx validation   
 *
 *   Description:
 *     Routines for IOCON and SWM functionality
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
#include "lpc8xx.h"
#include "lpc8xx_io.h"


#undef LPC_IOCON
#undef LPC_SWM
#define LPC_IOCON                 ((LPC_IOCON_Type          *) LPC_IOCON_BASE )
#define LPC_SWM                   ((LPC_SWM_Type            *) LPC_SWM_BASE  )




int attachIO(uint64_t func, uint64_t pad, bool enable){
      uint32_t tmp;
      uint32_t tmp_to_xor;
      uint32_t i;
      uint32_t padnum;
      
      padnum = pad & 0xff;
      
       switch (func>>56) {
           case  IO_ALLOC_IOCON : 
                                   // IOCON Allocation Style
                                   tmp =  LPC_IOCON->pio[padnum];                                 // read current values for desired PIO (to be able to preserve parameters)
                                   for (i=0;(i<=6) && ((pad & (0xff<<((i+1)*8)))>>((i+1)*8) !=  (func & 0xff)) ;i++) { // parse pad to find func number ot be programmed
                                   }                                
                                   tmp = (tmp & 0xfffffff8) | i ;                                // OR in the function number
                                   LPC_IOCON->pio[padnum]  = tmp ;                               // make function selection
                                  break;
           case  IO_ALLOC_SWM8  : 
                                   // Configure IOCon for pad to choose SWM
                                   // Make Sure IOCON is selecting SWM position
                                   tmp =  LPC_IOCON->pio[padnum];                                 // read current values for desired PIO (to be able to preserve parameters)
                                   for (i=0;i<=6 && ((pad & (0xff<<((i+1)*8)))>>((i+1)*8) !=  (SWM_RES & 0xff)) ;i++) { // parse pad to find func number ot be programmed
                                   }           
                                   tmp = (tmp & 0xfffffff8) | i ;                                // OR in the function number
                                   LPC_IOCON->pio[padnum]  = tmp ;                                // make function selection
           
                                   // attach function to pad (swm style)
                                   
                                  tmp = LPC_SWM->f8_set[(func & 0xff)>>2];                       // Read previous value to preserve other 3 funcs assigned in same reg
                                  
                                  tmp_to_xor = tmp & (0xff<<((func & 0x3)*8));
                                  tmp = tmp ^ tmp_to_xor;                                        // Blow away byte to be written
                                  
                                  padnum = (enable)? padnum : IO_BLNK;                           // If enable is false, we disable the IO driver
                                  
                                  tmp = tmp | padnum<<((func & 0x3)*8);                          // shift pad into appropriate position in reg    
                                  LPC_SWM->f8_set[(func & 0xff)>>2] = tmp;                       // Apply setting
                                  break;

           case  IO_ALLOC_SWM4  :  return 1;                                                     // This option has been retired from IP

           case  IO_ALLOC_SWM2  : 
                                   // Make Sure IOCON is selecting SWM position
                                   tmp =  LPC_IOCON->pio[padnum];                                 // read current values for desired PIO (to be able to preserve parameters)
                                   for (i=0;i<=6 && ((pad & (0xff<<((i+1)*8)))>>((i+1)*8) !=  (SWM_RES & 0xff)) ;i++) { // parse pad to find func number ot be programmed
                                   }                                
                                   tmp = (tmp & 0xfffffff8) | i ;                                // OR in the function number
                                   LPC_IOCON->pio[padnum]  = tmp ;                                // make function selection
           
                                   // attach function to pad (swm style)
                                   
                                  tmp = LPC_SWM->f2_set[(func & 0xff)>>4];                       // Read previous value to preserve other 3 funcs assigned in same reg
                                  
                                  // Browse to match pad option
                                  for (i=0;i<=2 && ((func & (0xff<<((i+1)*8)))>>((i+1)*8) !=  padnum) ;i++) { // parse func to find appropriate number to be programmed for 2 bit function
                                  }                                
                                  
                                  tmp_to_xor = tmp & (0x3<<((func & 0xf)*2));
                                  tmp = tmp ^ tmp_to_xor;                                        // Blow away nibble to be written
                                  i = (enable)? i : 0x3;                                         // If enable is false, we disable the IO driver
                                  tmp = tmp | i<<((func & 0xf)*2);                               // shift pad into appropriate position in reg    
                                  LPC_SWM->f2_set[(func & 0xff)>>4] = tmp;                       // Apply setting
                                  break;
       
           case  IO_ALLOC_SWM1  :                                    // Make Sure IOCON is selecting SWM position
                                  tmp =  LPC_IOCON->pio[padnum];                                 // read current values for desired PIO (to be able to preserve parameters)
                                  for (i=0;i<=6 && ((pad & (0xff<<((i+1)*8)))>>((i+1)*8) !=  (SWM_RES & 0xff)) ;i++) { // parse pad to find func number ot be programmed
                                  }                                
                                   tmp = (tmp & 0xfffffff8) | i ;                                // OR in the function number
                                  LPC_IOCON->pio[padnum]  = tmp ;                                // make function selection
           
                                   // attach function to pad (swm style)
                                   
                                  tmp = LPC_SWM->f1_set[(func & 0xff)>>5];                       // Read previous value to preserve other 3 funcs assigned in same reg
                                  if (enable) {
                                     tmp = tmp | 1<<(func & 0x1f);                               // shift pad into appropriate position in reg   
                                  } else {
                                     tmp = tmp ^ (tmp & 1<<(func & 0x1f));                       // clear 1bit function if enable is false
                                  }
                                  LPC_SWM->f1_set[(func & 0xff)>>5] = tmp;                       // Apply setting
                                  break;
           default              : return 1; 
       }
     return 0; 
};

int cfgIO(uint64_t pad, uint32_t param){
     LPC_IOCON->pio[pad & 0xff]  |= IOCON_PARAMETER(param);
     return 0; 
};

