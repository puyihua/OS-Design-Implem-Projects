.code16
.globl start
start:
_start:



# print string
	movb $0x13,%ah
	movb $0x01,%al

	movb $0,%bh
	movb $7,%bl
	
	movw $45,%cx   # length of string
	
	movb $8,%dh
	movb $0,%dl
	
	mov $thestring,%bp

	int $0x10
	

# press any key to continue	
	movb $0x00,%ah
	int $0x16

# video mode
	movb $0,%ah
	movb $0x13,%al
	
	movl $0xA0000,%ebx

	int $0x10
	

# little test, write something in (0xA0000)
	movl $0x12345678,(%ebx)

# show picture
	mov $packet,%si
	movb $0x80,%dl
	movb $0x42,%ah

	int $0x13

# wait for a key pressing
	movb $0x0,%ah
	int $0x16

# change color
	
	movw $0x1010,%ax
	movw $0x1,%bx
	movb $60,%dh
	movb $0x0,%ch
	movb $30,%cl
	int $0x10

	movw $0x1015,%ax
	movw $0x0,%bx
	int $0x10

stop:
	jmp stop

thestring:
	.string "Welcome to xv6@UIC. Press any key to continue"

packet:
	.byte 0x10
	.byte 0x00
	.word 125
	.word 0x0000, 0xa000
	.word 0x1,0x0,0x0,0x0

.org 0x01fe
 .word 0xAA55
