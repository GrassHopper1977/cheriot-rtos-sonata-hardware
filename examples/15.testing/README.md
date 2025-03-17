# A Basic CHERI Tests

This uses serial at 9600. Connect pins pmod0_2 and pmod0_3 together to create a physical loopback.

## What Does This Show?
1. It shows that hardware interfaces aren't protected. A second compartment can easily interfere with the UART by capturing bytes or using the `pinmux` to disconnect the UART input from a physical pin.
2. Arrays in structs don't seem to be protected. In the second half we attempt to print strings to Debug. Writing out of bound to a string causes an error that is captured. However, if you have a struct consisting of three strings, you can give a negative index to the string in the middel and it will start reading the previous string, likewise you can exceed the length of the middle string and read the third string too - all without raising an error. I imagine that the memory is defined for the struct in one piece and access to the internal memebrs are an offset from the start of the struct. 
