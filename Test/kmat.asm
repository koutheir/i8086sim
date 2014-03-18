

;input:
;dx: matrix number (0=>A or 1=>B)
enter_matrix_dimensions proc
    push ax
    push si
    
    enter_matrix_dimensions_0:
    ;Write the prompt
    push ax
    push dx
        add dl,'A' 
        mov s_enter_matrix_rows_count,dl

        mov dx,offset s_enter_matrix_rows_count
        mov ah,9
        int 21h
    pop dx
    pop ax
    
    call get_int
    cmp ax,0
    jle enter_matrix_dimensions_0
    cmp ax,max_matrix_rows_count
    jg enter_matrix_dimensions_0
    
    mov si,dx
    mov rows[si],al
    
    enter_matrix_dimensions_1:
    ;Write the prompt
    push ax
    push dx
        add dl,'A' 
        mov s_enter_matrix_columns_count,dl

        mov dx,offset s_enter_matrix_columns_count
        mov ah,9
        int 21h
    pop dx
    pop ax
    
    call get_int
    cmp ax,0
    jle enter_matrix_dimensions_1
    cmp ax,max_matrix_columns_count
    jg enter_matrix_dimensions_1
    
    mov columns[si],al
    
    pop si
    pop ax
    ret
enter_matrix_dimensions endp


;Compute the offset of the item in the matrix
;input:
;al: row <= 7
;ah: column <= 7
;dx: matrix number (0=>A or 1=>B or 2=>C)
;output:
;cx: offset = number_of_columns * row_index + column_index
get_matrix_item_offset proc
    push ax
    push bx
    push dx
    push di
        mov di,dx
        mov bl,columns[di]  ;bx=number of columns
        xor bh,bh
        xor ah,ah           ;ax=row index
        mul bx           
        mov cx,ax           ;cx=number_of_columns * row_index
    pop di
    pop dx
    pop bx
    pop ax
    
    push ax
        mov al,ah
        xor ah,ah  ;ax=column index
        add cx,ax  ;cx=number_of_columns * row_index + column_index
        shl cx,1   ;multiply cx by 2 because integers are words
    pop ax
    ret
get_matrix_item_offset endp


;input:
;al: row <= 7
;ah: column <= 7
;dx: matrix number (0=>A or 1=>B)
enter_matrix_item proc
    ;Write the prompt
    push ax
    push dx
        add dl,'A' 
        add al,'1'
        add ah,'1'
        
        mov s_enter_matrix_item,dl
        mov s_enter_matrix_item[2],al
        mov s_enter_matrix_item[4],ah 
        
        mov dx,offset s_enter_matrix_item
        mov ah,9
        int 21h
    pop dx
    pop ax
    
    push ax
    push cx
    push si
        ;Get the offset of item in the matrix in cx
        call get_matrix_item_offset
             
        ;Get the value in ax
        call get_int

        mov si,cx
        cmp dl,1
        je enter_matrix_item_0
        
        mov matA[si],ax         ;modify matrix A
        jmp enter_matrix_item_1
        
        enter_matrix_item_0:
        mov matB[si],ax         ;modify matrix A
        
        enter_matrix_item_1:
    pop si
    pop cx
    pop ax
    
    ret
enter_matrix_item endp


;input:
;dx: matrix number (0=>A or 1=>B)
enter_matrix proc  
    call enter_matrix_dimensions
    
    push ax
    push bx
    push si
    
    mov si,dx
    mov bl,rows[si]
    mov bh,columns[si]

    xor al,al
    enter_matrix_loop1:
        xor ah,ah
        enter_matrix_loop2:
            call enter_matrix_item
            
            inc ah
            cmp ah,bh
            jne enter_matrix_loop2
            
        inc al
        cmp al,bl
        jne enter_matrix_loop1

    pop si
    pop bx
    pop ax    
    ret
enter_matrix endp


;input:
;dx: matrix number (0=>A or 1=>B or 2=>C) 
print_matrix proc
    push ax
    push bx
    push cx
    push dx
	push si
	push di

	push dx
	    mov si,dx
	    
		add dl,'A'
		mov s_matrix_name[7],dl
		mov dx,offset s_matrix_name
	    mov ah,9
	    int 21h
		
		;Calculate the number of '-' char to write
	    mov al,columns[si]
	    xor ah,ah
	    mov cx,7
	    xor dx,dx
	    mul cx
	    dec ax
	    
	    ;Draw horizontal line
	    mov cx,ax
	    mov al,'-'
	    mov bh,0
		mov ah,0ah	
		int 10h
		
		mov dx,offset s_new_line
	    mov ah,9
	    int 21h
    pop dx
    
    push cx
    mov bl,rows[si]
    mov bh,columns[si]
    xor al,al
    print_matrix_loop1:
    	xor ah,ah
    	print_matrix_loop2:
    		call get_matrix_item_offset
    		mov di,cx
    		
    		push ax
    		push bx
    		push dx
	    		cmp dx,2
	    		je print_matrix_2
	    		
	    		cmp dx,1
	    		je print_matrix_1
	    		
	    		mov cx,matA[di]
	    		jmp print_matrix_3
	    		
	    		print_matrix_1:
	    		mov cx,matB[di]
	    		jmp print_matrix_3
	    		
	    		print_matrix_2:
	    		mov cx,matC[di]
	    		
	    		print_matrix_3:
	    		push cx
				    xor bh,bh
				    mov ah,3
				    int 10h          ;Get cursor position into dx
				pop ax               ;ax=old cx
			    
	    		call print_int
	    		
	    		;Advance cursor position by 7 characters
			    add dl,7
			    xor bh,bh
			    mov ah,2
			    int 10h
			pop dx 
    		pop bx
    		pop ax

			inc ah
            cmp ah,bh
            jne print_matrix_loop2
        
        push ax
        push dx    
			mov dx,offset s_new_line
	    	mov ah,9
	    	int 21h
	    pop dx
	    pop ax
	    
        inc al
        cmp al,bl
        jne print_matrix_loop1
        
    ;Draw horizontal line
    pop cx
    mov al, '-'
    mov bh, 0
	mov ah, 0ah	
	int 10h
	
	mov dx,offset s_new_line
    mov ah,9
    int 21h

	pop di
	pop si
    pop dx
    pop cx
    pop bx
    pop ax
	
	ret
print_matrix endp


;output:
;ax: zero if success, non-zero on failure
add_matrices proc
	push dx
	push si
	
    mov dx,offset s_matrix_addition
    mov ah,9
    int 21h
	
	mov al,rows[0]
	xor ah,ah
	cmp al,rows[1]
	jne add_matrices_0
 	mov rows[2],al       ;Set the dimensions of the resulting matrix

	mov al,columns[0]
	xor ah,ah
	cmp al,columns[1]
 	jne add_matrices_0
 	mov columns[2],al    ;Set the dimensions of the resulting matrix       
 	
    mul rows[0]
    shl ax,1         	 ;integers are words => ax = ax*2
    mov si,ax
    add_matrices_loop1:
    	sub si,2
    	
    	mov ax,matA[si]
    	add ax,matB[si]
    	
    	jno add_matrices_2
    	push ax
    	push dx
			mov dx,offset s_overflow
			mov ah,9
			int 21h
		pop dx
		pop ax
    	
    	add_matrices_2:
    	mov matC[si],ax
    	
    	cmp si,0
    	jne add_matrices_loop1
 	
    mov dx,offset s_matrix_addition_done
    mov ah,9
    int 21h
    
 	xor ax,ax		;success
 	jmp add_matrices_1

	add_matrices_0:
    mov dx,offset s_incompatible_dimensions
    mov ah,9
    int 21h
    mov ax,1		;error
	
	add_matrices_1:
	pop si
	pop dx
	ret
add_matrices endp


;output:
;ax: zero if success, non-zero on failure
multiply_matrices proc
	push bx
	push cx
	push dx
	push si
	push di
	
    mov dx,offset s_matrix_multiplication
    mov ah,9
    int 21h
    
    mov al,columns[0]
    mov ah,rows[1]
    cmp al,ah
    jne multiply_matrices_0

  	;Set the dimensions of the resulting matrix
 	mov al,rows[0]
 	mov rows[2],al
 	mov al,columns[1]
 	mov columns[2],al

	mov al,rows[0]         			;al=number of rows of A
	multiply_matrices_loop1:
		dec al
	
		mov ah,columns[1]           ;ah=number of columns of B
		multiply_matrices_loop2:
			dec ah
			
			push dx
		    	mov dx,2            ;Matrix C
				call get_matrix_item_offset
				mov si,cx
				mov matC[si],0
			pop dx

			mov bl,columns[0]       ;bl=number of columns of A
			multiply_matrices_loop3:
				dec bl
				
				push ax
					mov ah,bl
			    	mov dx,0        ;Matrix A
					call get_matrix_item_offset
					mov dl,cl       ;dl=offsetA[al,bl]
				pop ax
			
				push ax
					mov al,bl
				    push dx
				    	mov dx,1        ;Matrix B
						call get_matrix_item_offset
					pop dx
					mov dh,cl       ;dh=offsetB[bl,ah]
				pop ax
				
				push dx
					mov dx,2
					call get_matrix_item_offset
					mov si,cx
				pop dx
				
				push ax
				push dx
					mov di,dx
					and di,0ffh
					mov ax,matA[di]
					
					mov di,dx
					shr di,8
					mov cx,matB[di]
					
					mul cx
					add matC[si],ax
					
					cmp dx,0
					je multiply_matrices_2
					
					mov dx,offset s_overflow
					mov ah,9
					int 21h

					multiply_matrices_2:
					
				pop dx
				pop ax
					
				cmp bl,0
				jne multiply_matrices_loop3
				
			cmp ah,0
			jne multiply_matrices_loop2
			
		cmp al,0
		jne multiply_matrices_loop1
	
    mov dx,offset s_matrix_multiplication_done
    mov ah,9
    int 21h
    
	xor ax,ax
	jmp multiply_matrices_1
	
	multiply_matrices_0:	
    mov dx,offset s_incompatible_dimensions
    mov ah,9
    int 21h
    mov ax,1		;error
	
	multiply_matrices_1:
	pop di
	pop si
	pop dx
	pop cx
	pop bx
	ret
multiply_matrices endp 
