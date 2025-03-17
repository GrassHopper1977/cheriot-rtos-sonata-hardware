// Copyright Microsoft and CHERIoT Contributors.
// SPDX-License-Identifier: MIT

#include "compartment-macros.h"
#include "platform/sunburst/platform-pinmux.hh"
#include <array>
#include <compartment.h>
#include <cstdint>
#include <cstdlib>
#include <debug.hh>
#include <futex.h>
#include <interrupt.h>
#include <tick_macros.h>
#include <timeout.hh>
#include <platform/sunburst/platform-uart.hh>
#include <multiwaiter.h>
#include <queue.h>
#include <thread.h>
#include "tests.h"

/// Expose debugging features unconditionally for this compartment.
using Debug = ConditionalDebug<true, "uart">;

uint32_t get_current_ms()
{
    static const uint32_t CyclesPerMillisecond = CPU_TIMER_HZ / 1'000;
    uint64_t cycles = rdcycle64();  // Hidden in riscvreg.h and included through thread.h
    uint32_t msCount = static_cast<uint32_t>(cycles / CyclesPerMillisecond);    // Driver is not bothered by it wrapping (apprently).
    return msCount;
}

#define MAX_WAIT_MS (5000)	// 30s


/// Thread entry point.
void __cheri_compartment("uart") uart_entry()
{
	Debug::log("Configuring pinmux");
	auto pinSinks = MMIO_CAPABILITY(SonataPinmux::PinSinks, pinmux_pins_sinks);
	pinSinks->get(SonataPinmux::PinSink::pmod0_2).select(4); // uart1 tx -> pmod0_2
	auto blockSinks = MMIO_CAPABILITY(SonataPinmux::BlockSinks, pinmux_block_sinks);
	blockSinks->get(SonataPinmux::BlockSink::uart_1_rx).select(5); // pmod0_3 -> uart1 rx

	Debug::log("Configure UART");
	auto uart1 = MMIO_CAPABILITY(OpenTitanUart, uart1);
	uart1->init(9600);

	Debug::log("Start test loop");
	char outChar = '0';
	char inChar = ' ';
	uint32_t retry = 0;
	uart1->blocking_write(outChar);
	printf("Write: %c (try: %u)\r\n", outChar, retry);
	do
    {
		thread_millisecond_wait(200);	// sleep for a bit
		uint32_t waitUntil = get_current_ms() + MAX_WAIT_MS;
		while(uart1->status & OpenTitanUart::StatusReceiveEmpty) {
			if(get_current_ms() >= waitUntil) {
				Debug::log("'{}' was stolen by another compartment or input switched off! Try again.", outChar);
				retry++;
				blockSinks->get(SonataPinmux::BlockSink::uart_1_rx).select(5); // pmod0_3 -> uart1 rx
				uart1->blocking_write(outChar);
				printf("Write: %c (try: %u)\r\n", outChar, retry);
				waitUntil = get_current_ms() + MAX_WAIT_MS;
			}
		}
		inChar = uart1->readData;
		printf(" Read: %c\r\n", inChar);
		// Don't send more until the first one comes back.
		if(inChar == outChar) {
			if(outChar == '9') {
				outChar = 'A';
			} else if(outChar == 'Z') {
				outChar = 'a';
			} else if(outChar == 'z') {
				break;
			} else {
				outChar++;
			}
			retry = 0;
			uart1->blocking_write(outChar);
			printf("Write: %c (try: %u)\r\n", outChar, retry);
		}

	} while(inChar != 'z');

	Debug::log("This will print OK.");	
	write("Test 1");

	Debug::log("Will fail after a few goes");
	char msg1[] = "Hello world!";
	for(int i = 0; i < 14; i++) {
		write(&msg1[i]);
	}

	Debug::log("What about a struct?");
	struct MsgStruct {
		char one[4];
		char two[4];
		char three[4];
	};

	MsgStruct msg2 = {
		.one = "123",
		.two = "456",
		.three = "789"
	};
	MsgStruct* msg3 = &msg2;

	Debug::log("one");
	write(msg2.one);
	Debug::log("two");
	write(msg2.two);
	Debug::log("three");
	write(msg2.three);


	for(int i = -2; i < 6; i++) {
		Debug::log("&(msg2.two[{}]):", i);
		write(&(msg2.two[i]));
	}

	Debug::log("What about a pointer to a struct?");
	Debug::log("one");
	write(msg3->one);
	Debug::log("two");
	write(msg3->two);
	Debug::log("three");
	write(msg3->three);

	for(int i = -2; i < 6; i++) {
		Debug::log("&(msg3->two[{}]):", i);
		write(&(msg3->two[i]));
	}


	Debug::log("Tests finished!");
}
