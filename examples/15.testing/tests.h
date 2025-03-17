// Copyright Microsoft and CHERIoT Contributors.
// SPDX-License-Identifier: MIT

#include <compartment.h>

/**
 * Write `msg` to the default uart.
 */
int __cheri_compartment("tests") write(const char *msg);
