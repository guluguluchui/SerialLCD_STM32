//
// This file is part of the GNU ARM Eclipse distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stm32f10x.h>
#include "diag/Trace.h"
#include "uart_object.h"
#include "lcd_object.h"
#include "ATcmd_object.h"
#include "delay.h"


// ----------------------------------------------------------------------------
//
// STM32F1 empty sample (trace via ITM).
//
// Trace support is enabled by adding the TRACE macro definition.
// By default the trace messages are forwarded to the ITM output,
// but can be rerouted to any device or completely suppressed, by
// changing the definitions required in system/src/diag/trace_impl.c
// (currently OS_USE_TRACE_ITM, OS_USE_TRACE_SEMIHOSTING_DEBUG/_STDOUT).
//

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"

int
main(int argc, char* argv[])
{
  // At this stage the system clock should have already been configured
  // at high speed.
	Systick_Init();
  object_uart_init(&usart1,1);
  object_lcd_init(&global_lcd);
  object_ATcmd_init(&at_command);

  // Infinite loop
  while (1)
    {
      // Add your code here.
      at_command.get_cmd(&at_command);
      at_command.execute(&at_command);
    }
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
