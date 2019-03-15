//adding for git tests

/*

    Part of the Raspberry-Pi Bare Metal Tutorials
    Copyright (c) 2015, Brian Sidebotham
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice,
        this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "./peripheral/rpi-aux.h"
#include "./peripheral/rpi-gpio.h"
//#include "./peripheral/rpi-interrupts.h"
#include "./peripheral/rpi-systimer.h"
#include "./peripheral/rpi-smi.h"

extern uint32_t global_counter;

/** Main function - we'll never return from here */
void kernel_main( unsigned int r0, unsigned int r1, unsigned int atags )
{
    /* Enable interrupts! */
   // _enable_interrupts();

    /* Initialise the UART */
    RPI_AuxMiniUartInit( 115200, 8 );
    RPI_WaitMicroSeconds(2000000);
    rpi_sys_timer_t *timer = RPI_GetSystemTimer();

    /* Print to the UART using the standard libc functions */
    printf( "Initialise UART console with standard libc\r\n\n" );

    struct smi_settings _smi;

    //smi_dump_context_labelled();

    smi_setup_clock(0, 0);

    smi_set_default_settings(&_smi);
    _smi.write_hold_time = 1;
    _smi.write_strobe_time = 1;
    _smi.write_setup_time = 1;
    _smi.write_pace_time = 1;
    smi_setup(&_smi);


    //RPI_WaitMicroSeconds(2000000);


    smi_set_address(0x0 );


    volatile uint32_t i,k;
    for (i=0; i<=43; i++)
    {
      if (!((i==26)|(i==27)|(i==14)|(i==15)|(i==4)))
      {
        RPI_SetGpioPinFunction(i,FS_ALT1);
      }
    }
    RPI_SetGpioPinFunction(4,FS_ALT0);
    gpioclk_setup_clock(0x5,0);

    uint8_t d[4096];
    k=0;
    for (i=0; i<4096; i++)
    {
        d[i]=k++;
        if (k>=0xFF) k=0;

    }
   // int counter;
    printf( "Setup complete :\r\n\n" );
    smi_dump_context_labelled();

    // smi_write_fifo((uint32_t)d, 4096);
     uint32_t count0 = 1;
     uint32_t t1, t2;

    while( 1 )
    {

       // smi_write_single_word(1);
       //     smi_write_n_words(d,1024);
   // RPI_WaitMicroSeconds(1000);
   // t1 = timer->counter_lo;
     smi_write_fifo((uint32_t)d, 4096);
    // t2 = timer->counter_lo;
    // RPI_WaitMicroSeconds(1000);
    // printf ("Global count: %i \r\n", global_counter);
    // printf ("Timer count : %i cycles\r\n", t2-t1);
    // count0++;


   }
}
