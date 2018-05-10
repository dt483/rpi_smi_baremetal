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

#include "./peripheral/rpi-aux.h"
#include "./peripheral/rpi-gpio.h"
#include "./peripheral/rpi-interrupts.h"
#include "./peripheral/rpi-systimer.h"
#include "./peripheral/rpi-smi.h"



/** Main function - we'll never return from here */
void kernel_main( unsigned int r0, unsigned int r1, unsigned int atags )
{
    /* Enable interrupts! */
   // _enable_interrupts();

    /* Initialise the UART */
    RPI_AuxMiniUartInit( 115200, 8 );

    /* Print to the UART using the standard libc functions */
    printf( "Initialise UART console with standard libc\r\n\n" );

    RPI_WaitMicroSeconds(2000000);

    struct smi_instance smi_iface;
    smi_init(&smi_iface);
    smi_dump_context_labelled(&smi_iface);




    RPI_WaitMicroSeconds(2000000);

    smi_setup_clock(&smi_iface, 50, 0);
    smi_setup_regs(&smi_iface);
    smi_set_address(&smi_iface, 0x0 );


    /*int i;
    for (i=0; i<44; i++)
    {
      if (~((i==25)|(i==26)))
      {
        RPI_SetGpioPinFunction(i,FS_ALT1);
      }
    }*/

    printf( "Setup complete :\r\n\n" );
    smi_dump_context_labelled(&smi_iface);
    while( 1 )
    {
            smi_write_single_word(&smi_iface, 0x5555AAAA);
    }
}
