;-----------------------------------------------------------------------------
; Copyright (c) 2012 GarageGames, LLC
;
; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"), to
; deal in the Software without restriction, including without limitation the
; rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
; sell copies of the Software, and to permit persons to whom the Software is
; furnished to do so, subject to the following conditions:
;
; The above copyright notice and this permission notice shall be included in
; all copies or substantial portions of the Software.
;
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
; FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
; IN THE SOFTWARE.
;-----------------------------------------------------------------------------

        
segment .text

; syntax: export_fn <function name>
%macro export_fn 1
   %ifidn __OUTPUT_FORMAT__, elf
   ; No underscore needed for ELF object files
   global %1
   %1:
   %else
   global _%1
   _%1:
   %endif
%endmacro

; push registers 
%macro pushreg 0
;    pushad
    push ebx
    push ebp
    push esi
    push edi
%endmacro

; pop registers
%macro popreg 0
    pop edi
    pop esi
    pop ebp
    pop ebx
;    popad
%endmacro
      
; void detectX86CPUInfo(char *vendor, U32 *processor, U32 *properties);
export_fn detectX86CPUInfo
   push         ebp
   mov          ebp, esp

   pushreg

   push         edx
   push         ecx
   pushfd
   pushfd                        ; save EFLAGS to stack
   pop          eax              ; move EFLAGS into EAX
   mov          ebx, eax
   xor          eax, 0x200000    ; flip bit 21
   push         eax
   popfd                         ; restore EFLAGS
   pushfd
   pop          eax
   cmp          eax, ebx
   jz           EXIT             ; doesn't support CPUID instruction

   ;
   ; get vendor information using CPUID eax == 0
   xor          eax, eax
   cpuid

   ; store the vendor tag (12 bytes in ebx, edx, ecx) in the first parameter,
   ; which should be a char[13]
   push         eax             ; save eax
   mov          eax, [ebp+8]    ; store the char* address in eax
   mov          [eax], ebx      ; move ebx into the first 4 bytes
   add          eax, 4          ; advance the char* 4 bytes
   mov          [eax], edx      ; move edx into the next 4 bytes
   add          eax, 4          ; advance the char* 4 bytes
   mov          [eax], ecx      ; move ecx into the last 4 bytes
   pop          eax             ; restore eax
        
   ; get generic extended CPUID info
   mov          eax, 1
   cpuid                         ; eax=1, so cpuid queries feature information

   and          eax, 0x0fff3fff
   push         ecx
   mov          ecx, [ebp+12]
   mov          [ecx], eax      ; just store the model bits in processor param
   mov          ecx, [ebp+16]
   mov          [ecx], edx      ; set properties param
   pop          ecx

   ; want to check for 3DNow(tm).  
   ; need to see if extended cpuid functions present. 
   mov          eax, 0x80000000
   cpuid
   cmp          eax, 0x80000000
   jbe          MAYBE_3DLATER
   mov          eax, 0x80000001
   cpuid
   ; 3DNow if bit 31 set -> put bit in our properties        
   and          edx, 0x80000000  
   push         eax
   mov          eax, [ebp+16]
   or           [eax], edx
   pop          eax
MAYBE_3DLATER:
EXIT:
   popfd
   pop          ecx
   pop          edx

   popreg

   pop          ebp
   ret
