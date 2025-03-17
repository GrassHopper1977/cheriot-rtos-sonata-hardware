-- Copyright Microsoft and CHERIoT Contributors.
-- SPDX-License-Identifier: MIT

set_project("Sonata UART Example")
sdkdir = "../../sdk"
includes(sdkdir)
set_toolchains("cheriot-clang")

option("board")
    set_default("sonata")

compartment("interloper")
    add_files("interloper.cc")

compartment("uart")
    add_files("uart.cc")

compartment("tests")
    add_files("tests.cc")

-- Firmware image for the example.
firmware("basic_tests")
    add_deps("freestanding", "debug", "stdio", "message_queue")
    add_deps("interloper", "uart", "tests")
    on_load(function(target)
        target:values_set("board", "$(board)")
        target:values_set("threads", {
            {
                compartment = "interloper",
                priority = 1,
                entry_point = "main_entry",
                stack_size = 0x800,
                trusted_stack_frames = 2
            },
            {
                compartment = "uart",
                priority = 1,
                entry_point = "uart_entry",
                stack_size = 0x800,
                trusted_stack_frames = 4
            }
        }, {expand = false})
    end)
