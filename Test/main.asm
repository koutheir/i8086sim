
max_matrix_rows_count		equ 7
max_matrix_columns_count	equ 7
max_matrix_length			equ 49

data segment
	s_welcome	db "Welcome!",13,10,"This program calculates the sum & product of 2 given matrices.",13,10,"Version 1.0.",13,10,"Created by Koutheir Attouchi, 2009.",13,10,13,10,'$'

	int_buffer          db 9,0,'+',5 dup('0'),'$'
	s_invalid_number    db "Error: Invalid number or integer overflow.",13,10,"Try again: $"
	s_temp_str			db 32 dup('$')
	s_pause				db "Press any key to continue...",13,10,'$'
	s_new_line          db 13,10,'$'
	s_escape			db "Press ESC to quit...",13,10,'$'
	
	s_enter_matrix_rows_count		db "M.RowsCount = $"
	s_enter_matrix_columns_count	db "M.ColumnsCount = $"
	s_enter_matrix_item				db "M[0,0] = $" 
	s_matrix_name					db "Matrix M:",13,10,'$'
	s_matrix_addition				db "Performing addition... Please wait.",13,10,'$'
	s_matrix_addition_done			db "Addition done.",13,10,'$'
	s_matrix_multiplication			db "Performing multiplication... Please wait.",13,10,'$'
	s_matrix_multiplication_done	db "Multiplication done.",13,10,'$'
	s_incompatible_dimensions		db "Error: Matrices have incompatible dimensions. Operation cannot proceed.",13,10,'$'
	s_overflow						db "Error: Integer overflow detected. Result is wrong.",13,10,'$'

	rows    db 3 dup(0)         ;maximum is max_matrix_rows_count
	columns db 3 dup(0)         ;maximum is max_matrix_columns_count
	matA    dw max_matrix_length dup(0)        ;[row0],[row1],...
	matB    dw max_matrix_length dup(0)        ;[row0],[row1],...
	matC    dw max_matrix_length dup(0)        ;[row0],[row1],...
data ends

stack segment
    dw 512 dup(0)
stack ends

code segment

main:
	assume ds:data,es:data
	assume cs:code
	
	;set segment registers:
    mov ax,data
    mov ds,ax        ;Data segment
    mov es,ax        ;Extra segment 
    
	;cancel blinking and enable all 16 colors:
	mov ax,1003h
	xor bx,bx
	int 10h
    
    mov dx,offset s_welcome
    mov ah,9
    int 21h

main_2:    
    xor dx,dx
    call enter_matrix
    
    inc dx
    call enter_matrix
   
    xor dx,dx
    call print_matrix

    inc dx
    call print_matrix
    
    call pause
    
   	call add_matrices
    cmp ax,0
    jne main_0
    
    mov dx,2
   	call print_matrix
    
    main_0:
    call multiply_matrices
    cmp ax,0
    jne main_1
    
    mov dx,2
    call print_matrix
    
    main_1:
    mov dx,offset s_escape
    mov ah,9
    int 21h
    
    mov ah,7
    int 21h
    cmp al,01bh
    jne main_2
    
    int 20h
    ret

	include "kio.asm"
	include "kmat.asm"

code ends

end main           ;set entry point and stop the assembler
