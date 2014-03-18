

pause proc
	push ax
	push dx
	
    mov dx,offset s_pause
    mov ah,9
    int 21h
    
    mov ah,7
    int 21h

	pop dx
	pop ax
	ret
pause endp


;output:
;ax: integer
get_int proc
    push bx
    push cx
    push dx
    push di
    
    get_int_5:
    ;Get the value of the number in string form
    mov dx,offset int_buffer    
    mov ah,0ah
    int 21h
    
    ;Write the new line to the console
    mov dx,offset s_new_line
    mov ah,9
    int 21h
    
    mov cl,int_buffer[1]   ;cx=number of digits
    xor ch,ch
    mov bx,offset int_buffer + 2  ;[bx]=the digit in string form
    
    ;Adjust the length of the number and the starting offset
    ;when there is a sign.
    cmp int_buffer[2],'+'
    jne get_int_0
    inc bx
    dec cx
    
    get_int_0:
    cmp int_buffer[2],'-'
    jne get_int_1
    inc bx
    dec cx
    
    get_int_1:
    xor ax,ax  ;default output is zero    
   
    ;ensure the string length is not zero 
    cmp cx,0
    jne get_int_6
    
    get_int_7:
    mov dx,offset s_invalid_number    
    mov ah,9
    int 21h
    jmp get_int_5
        
    get_int_6:
    mov di,cx
    get_int_loop1:
        dec di
        
        ;[bx]=the current digit
        cmp [bx],'0'
        	jl get_int_7
        cmp [bx],'9'
        	jg get_int_7

        sub [bx],'0'
        
        ;ax+=[bx]*10^(di-1)
        mov dl,[bx]       ;dx=[bx]*10^(di-1)
        xor dh,dh
        cmp di,0          ;if (di==0) no work is required
        je get_int_3
        cmp dx,0          ;if (dx==0) no work is required
        je get_int_3
        
        push ax
        push di
        get_int_loop2:
            dec di
            
            ;Multiply dx by 10
            mov ax,dx
            ;dx=ax*8
            shl ax,1
            	jc get_int_9
            shl ax,1
           		jc get_int_9
            shl ax,1
           		jc get_int_9
            mov dx,ax
            shr ax,2     
            add dx,ax    ;dx+=ax*2
            jno get_int_8
            
            get_int_9:
	        pop di    
	        pop ax
	        jmp get_int_7
            
            get_int_8:
            cmp di,0
            jne get_int_loop2
            
        pop di    
        pop ax
            
        get_int_3: ;dx=dx=[bx]*10^(di-1)
        add ax,dx
        inc bx     ;point to the next digit
        
        cmp di,0
        jne get_int_loop1
        
    get_int_2:
    ;If no overflow has occured, the number must be positive in this stage
    cmp ax,0
    jl get_int_7
    
    ;Check if the number is negative
    mov bx,offset int_buffer
    cmp int_buffer[2],'-'
    jne get_int_4
    neg ax
    
    get_int_4:
    pop di
    pop dx
    pop cx
    pop bx
    ret
get_int endp

           
;Get the length of a string holding the signed number in ax
;output:
;ax: length of the word when converted to string
get_int_string_length proc
	push bx
	push cx
	push dx
	   
	mov bx,0      ;bx:length of the word when converted to string
	mov cx,10
	
 	;Check the sign of the number
	cmp ax,0
	jge get_int_string_length_1
	
	;The number is negative
	;Negate it to get its positive part
	neg ax
	inc bx                
	
	get_int_string_length_1:
		inc bx
		
		;Divide the word by 10
		mov dx,0
		div cx

		;Check if there are more digits in the word		
		cmp ax,0
		jne get_int_string_length_1

	mov ax,bx	
	pop dx
	pop cx
	pop bx
	ret
get_int_string_length endp


;input:
;ax: the signed number to print        
print_int proc
	push ax
	push bx
	push cx
	push dx   
	push si

    push ax
    	call get_int_string_length
    	mov si,ax
	pop ax    
	
    mov s_temp_str[si],'$'
	mov bx,offset s_temp_str

 	;Check the sign of the number
	cmp ax,0
	jge print_int_loop1
	
	;The number is negative
	;Negate it to get its positive part
	neg ax
	mov s_temp_str[0],'-'
	inc bx
	dec si

	print_int_loop1:	
		dec si
		
		;Divide the word by 10 
		mov cx,10 
		mov dx,0
		div cx          
		
		add dx,'0'
		mov [bx+si],dl
		
		cmp si,0
		jne print_int_loop1
		
    mov dx,offset s_temp_str
	mov ah,9
	int 21h
	
	pop si	
	pop dx
	pop cx
	pop bx
	pop ax
	ret
print_int endp
