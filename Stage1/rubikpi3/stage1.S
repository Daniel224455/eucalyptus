  .section .text
  .global _Start

_Start:
  //ldr   w0, =0x02000121
  //ldr   w1, =0x00000023
  //mov   w2, wzr
  //mov   w3, wzr
  //ldr   w4, =0x1
  //smc   #0
  ret

_Dead:
  b _Dead