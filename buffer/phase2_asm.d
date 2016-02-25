
phase2_asm.o:     file format elf32-i386


Disassembly of section .text:

00000000 <.text>:
   0:	bb 92 8b 04 08       	mov    $0x8048b92,%ebx
   5:	53                   	push   %ebx
   6:	bb 0c c2 04 08       	mov    $0x804c20c,%ebx
   b:	b9 85 f8 d5 4e       	mov    $0x4ed5f885,%ecx
  10:	89 0b                	mov    %ecx,(%ebx)
  12:	c3                   	ret    
