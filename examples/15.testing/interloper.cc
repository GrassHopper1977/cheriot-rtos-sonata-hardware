// Copyright Microsoft and CHERIoT Contributors.
// SPDX-License-Identifier: MIT

#include <cstdint>
#include <cstdlib>
#include <timeout.hh>
#include <compartment.h>
#include <debug.hh>
#include <thread.h>
#include <queue.h>
#include "platform/sunburst/platform-pinmux.hh"
#include "platform/sunburst/platform-uart.hh"

/// Expose debugging features unconditionally for this compartment.
using Debug = ConditionalDebug<true, "Interloper compartment">;

/// Thread entry point.
void __cheri_compartment("interloper") main_entry()
{
	// Print welcome, along with the compartment's name to the default UART.
	Debug::log("Interloper thread!");

	Debug::log("Attempts to steal data from UART 1 to prevent the main thread from completing.");
	auto uart1 = MMIO_CAPABILITY(OpenTitanUart, uart1);
	
	// Attack 1 - steal a character!
	char c = 0;
	thread_millisecond_wait(5000);	// sleep for 1 minute
	c = uart1->blocking_read();
	Debug::log("HA HA! We stole a character! c='{}'", c);

	// Atatck 2 - Switch off the input block!
	thread_millisecond_wait(10000);
	auto blockSinks = MMIO_CAPABILITY(SonataPinmux::BlockSinks, pinmux_block_sinks);
	blockSinks->get(SonataPinmux::BlockSink::uart_1_rx).select(0); // Turn off the input block!
	Debug::log("HA HA! We switched off the input block!");

	Debug::log("Interloper finished.");
}
