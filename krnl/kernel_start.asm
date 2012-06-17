MULTIBOOT_PAGE_ALIGN   equ 1<<0
MULTIBOOT_MEMORY_INFO  equ 1<<1

MULTIBOOT_HEADER_MAGIC equ 0x1BADB002
MULTIBOOT_HEADER_FLAGS equ MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO
CHECKSUM equ -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
; The Multiboot header (in NASM syntax)
        align 4
        dd MULTIBOOT_HEADER_MAGIC
        dd MULTIBOOT_HEADER_FLAGS
        dd CHECKSUM
 
[BITS 32]
[global _start]
[extern k_main] ; this is in the c file

_start:
  ; stop using bootloader GDT, and load new GDT
  lgdt [gdt_ptr]

  mov ax,LINEAR_DATA_SEL
  mov ds,ax
  mov es,ax
  mov ss,ax
  mov fs,ax
  mov gs,ax
  jmp LINEAR_CODE_SEL:sbat
sbat:

; set up interrupt handlers, then load IDT register
	mov ecx,(idt_end - idt) >> 3 ; number of exception handlers
	mov edi,idt
	mov esi,isr0
do_idt:
	mov eax,esi			; EAX=offset of entry point
	mov [edi],ax			; set low 16 bits of gate offset
	shr eax,16
	mov [edi + 6],ax		; set high 16 bits of gate offset
	add edi,8			; 8 bytes/interrupt gate
	add esi,(isr1 - isr0)		; bytes/stub
	loop do_idt

	lidt [idt_ptr]
    
  push ebx	; Multiboot Informationen an k_main übergeben
  call k_main	; k_main Funktion aufrufen

  jmp $

;-------------------------------------------------
;	Interrupt Service Routinen (teils ausgeliehen)
;-------------------------------------------------

fault:

; I shouldn't have to do this!
%macro PUSHB 1
	db 6Ah
	db %1
%endmacro

%macro INTR 1				; (byte offset from start of stub)
isr%1:
	push byte 0			; ( 0) fake error code
	PUSHB %1			; ( 2) exception number
	push gs				; ( 4) push segment registers
	push fs				; ( 6)
	push es				; ( 8)
	push ds				; ( 9)
	pusha				; (10) push GP registers
		mov ax,LINEAR_DATA_SEL	; (11) put known-good values...
		mov ds,eax		; (15) ...in segment registers
		mov es,eax		; (17)
		mov fs,eax		; (19)
		mov gs,eax		; (21)
		mov eax,esp		; (23)
		push eax		; (25) push pointer to regs_t
.1:
; setvect() changes the operand of the CALL instruction at run-time,
; so we need its location = 27 bytes from start of stub. We also want
; the CALL to use absolute addressing instead of EIP-relative, so:
			mov eax,fault	; (26)
			call eax	; (31)
			jmp all_ints	; (33)
%endmacro				; (38)

%macro INTR_EC 1
isr%1:
	nop				; error code already pushed
	nop				; nop+nop=same length as push byte
	PUSHB %1			; ( 2) exception number
	push gs				; ( 4) push segment registers
	push fs				; ( 6)
	push es				; ( 8)
	push ds				; ( 9)
	pusha				; (10) push GP registers
		mov ax,LINEAR_DATA_SEL	; (11) put known-good values...
		mov ds,eax		; (15) ...in segment registers
		mov es,eax		; (17)
		mov fs,eax		; (19)
		mov gs,eax		; (21)
		mov eax,esp		; (23)
		push eax		; (25) push pointer to regs_t
.1:
; setvect() changes the operand of the CALL instruction at run-time,
; so we need its location = 27 bytes from start of stub. We also want
; the CALL to use absolute addressing instead of EIP-relative, so:
			mov eax,fault	; (26)
			call eax	; (31)
			jmp all_ints	; (33)
%endmacro				; (38)

; the vector within the stub (operand of the CALL instruction)
; is at (isr0.1 - isr0 + 1)

all_ints:
	pop eax
	popa				; pop GP registers
	pop ds				; pop segment registers
	pop es
	pop fs
	pop gs
	add esp,8			; drop exception number and error code
	iret

	
; $$$ CONTEXT SWITCHING + SOFTWARE MULTITASKING $$$
[global contextswitch]
contextswitch:
	pusha	; 32 byte push
	mov	eax, [esp+36]
	mov	[eax], esp
	mov	esp, [esp+40] ; $$$
	popa
	ret
	
[global testfunc]
testfunc:
ret

[global panic]
	int 13

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; name:			getvect
; action:		reads interrupt vector
; in:			[EBP + 12] = vector number
; out:			vector stored at address given by [EBP + 8]
; modifies:		EAX, EDX
; minimum CPU:		'386+
; notes:		C prototype:
;			typedef struct
;			{	unsigned access_byte, eip; } vector_t;
;			getvect(vector_t *v, unsigned vect_num);
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

[global getvect]
getvect:
	push ebp
		mov ebp,esp
		push esi
		push ebx
			mov esi,[ebp + 8]

; get access byte from IDT[i]
			xor ebx,ebx
			mov bl,[ebp + 12]
			shl ebx,3
			mov al,[idt + ebx + 5]
			mov [esi + 0],eax

; get handler address from stub
			mov eax,isr1
			sub eax,isr0	; assume stub size < 256 bytes
			mul byte [ebp + 12]
			mov ebx,eax
			add ebx,isr0
			mov eax,[ebx + (isr0.1 - isr0 + 1)]
			mov [esi + 4],eax
		pop ebx
		pop esi
	pop ebp
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; name:			setvect
; action:		writes interrupt vector
; in:			[EBP + 12] = vector number,
;			vector stored at address given by [EBP + 8]
; out:			(nothing)
; modifies:		EAX, EDX
; minimum CPU:		'386+
; notes:		C prototype:
;			typedef struct
;			{	unsigned access_byte, eip; } vector_t;
;			getvect(vector_t *v, unsigned vect_num);
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

[global setvect]
setvect:
	push ebp
		mov ebp,esp
		push esi
		push ebx
			mov esi,[ebp + 8]

; store access byte in IDT[i]
			mov eax,[esi + 0]
			xor ebx,ebx
			mov bl,[ebp + 12]
			shl ebx,3
			mov [idt + ebx + 5],al

; store handler address in stub
			mov eax,isr1
			sub eax,isr0	; assume stub size < 256 bytes
			mul byte [ebp + 12]
			mov ebx,eax
			add ebx,isr0
			mov eax,[esi + 4]
			mov [ebx + (isr0.1 - isr0 + 1)],eax
		pop ebx
		pop esi
	pop ebp
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; interrupt/exception stubs
; *** CAUTION: these must be consecutive, and must all be the same size.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	INTR 0		; zero divide (fault)
	INTR 1		; debug/single step
	INTR 2		; non-maskable interrupt (trap)
	INTR 3		; INT3 (trap)
	INTR 4		; INTO (trap)
	INTR 5		; BOUND (fault)
	INTR 6		; invalid opcode (fault)
	INTR 7		; coprocessor not available (fault)
	INTR_EC 8	; double fault (abort w/ error code)
	INTR 9		; coproc segment overrun (abort; 386/486SX only)
	INTR_EC 0Ah	; bad TSS (fault w/ error code)
	INTR_EC 0Bh	; segment not present (fault w/ error code)
	INTR_EC 0Ch	; stack fault (fault w/ error code)
	INTR_EC 0Dh	; GPF (fault w/ error code)
	INTR_EC 0Eh	; page fault
	INTR 0Fh	; reserved
	INTR 10h	; FP exception/coprocessor error (trap)
	INTR 11h	; alignment check (trap; 486+ only)
	INTR 12h	; machine check (Pentium+ only)
	INTR 13h
	INTR 14h
	INTR 15h
	INTR 16h
	INTR 17h
	INTR 18h
	INTR 19h
	INTR 1Ah
	INTR 1Bh
	INTR 1Ch
	INTR 1Dh
	INTR 1Eh
	INTR 1Fh

; isr20 through isr2F are hardware interrupts. The 8259 programmable
; interrupt controller (PIC) chips must be reprogrammed to make these work.
	INTR 20h	; IRQ 0/timer interrupt
	INTR 21h	; IRQ 1/keyboard interrupt
	INTR 22h
	INTR 23h
	INTR 24h
	INTR 25h
	INTR 26h	; IRQ 6/floppy interrupt
	INTR 27h
	INTR 28h	; IRQ 8/real-time clock interrupt
	INTR 29h
	INTR 2Ah
	INTR 2Bh
	INTR 2Ch
	INTR 2Dh	; IRQ 13/math coprocessor interrupt
	INTR 2Eh	; IRQ 14/primary ATA ("IDE") drive interrupt
	INTR 2Fh	; IRQ 15/secondary ATA drive interrupt

; syscall software interrupt
	INTR 30h

; the other 207 vectors are undefined

%assign i 31h
%rep (0FFh - 30h)

	INTR i

%assign i (i + 1)
%endrep

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

SECTION .data
gdt:
; NULL descriptor
	dw 0		; limit 15:0
	dw 0		; base 15:0
	db 0		; base 23:16
	db 0		; type
	db 0		; limit 19:16, flags
	db 0		; base 31:24

; unused descriptor
	dw 0
	dw 0
	db 0
	db 0
	db 0
	db 0

LINEAR_DATA_SEL	equ	$-gdt
	dw 0FFFFh
	dw 0
	db 0
	db 92h		; present, ring 0, data, expand-up, writable
	db 0CFh		; page-granular (4 gig limit), 32-bit
	db 0

LINEAR_CODE_SEL	equ	$-gdt
	dw 0FFFFh
	dw 0
	db 0
	db 9Ah		; present,ring 0,code,non-conforming,readable
	db 0CFh		; page-granular (4 gig limit), 32-bit
	db 0
gdt_end:

gdt_ptr:
	dw gdt_end - gdt - 1
	dd gdt

; 256 ring 0 interrupt gates

idt:
;interrupts 3-14 now, since we are making the descriptors
;identical, we are going to loop to get them all(12 total)
%rep 256
  dw 0x0000
  dw LINEAR_CODE_SEL
  dw 0x8E00
  dw 0x20
%endrep
idt_end:

idt_ptr:
	dw idt_end - idt - 1		; IDT limit
	dd idt				; linear adr of IDT
