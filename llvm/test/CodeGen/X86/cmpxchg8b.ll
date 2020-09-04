; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -mtriple=i686-unknown- -mcpu=core2 | FileCheck %s --check-prefixes=CHECK,X86
; RUN: llc < %s -mtriple=x86_64-unknown- -mcpu=core2 | FileCheck %s --check-prefixes=CHECK,X64
; RUN: llc < %s -mtriple=i686-unknown- -mcpu=i486 | FileCheck %s --check-prefixes=I486
; RUN: llc < %s -mtriple=i686-unknown- -mcpu=znver1 | FileCheck %s --check-prefixes=CHECK,X86
; RUN: llc < %s -mtriple=i686-unknown- -mcpu=lakemont | FileCheck %s --check-prefixes=CHECK,X86

; Basic 64-bit cmpxchg
define void @t1(i64* nocapture %p) nounwind ssp {
; X86-LABEL: t1:
; X86:       # %bb.0: # %entry
; X86-NEXT:    pushl %ebx
; X86-NEXT:    pushl %esi
; X86-NEXT:    movl {{[0-9]+}}(%esp), %esi
; X86-NEXT:    xorl %eax, %eax
; X86-NEXT:    xorl %edx, %edx
; X86-NEXT:    xorl %ecx, %ecx
; X86-NEXT:    movl $1, %ebx
; X86-NEXT:    lock cmpxchg8b (%esi)
; X86-NEXT:    popl %esi
; X86-NEXT:    popl %ebx
; X86-NEXT:    retl
;
; X64-LABEL: t1:
; X64:       # %bb.0: # %entry
; X64-NEXT:    movl $1, %ecx
; X64-NEXT:    xorl %eax, %eax
; X64-NEXT:    lock cmpxchgq %rcx, (%rdi)
; X64-NEXT:    retq
;
; I486-LABEL: t1:
; I486:       # %bb.0: # %entry
; I486-NEXT:    pushl %ebp
; I486-NEXT:    movl %esp, %ebp
; I486-NEXT:    andl $-8, %esp
; I486-NEXT:    subl $8, %esp
; I486-NEXT:    movl 8(%ebp), %eax
; I486-NEXT:    movl $0, {{[0-9]+}}(%esp)
; I486-NEXT:    movl $0, (%esp)
; I486-NEXT:    movl %esp, %ecx
; I486-NEXT:    pushl $5
; I486-NEXT:    pushl $5
; I486-NEXT:    pushl $0
; I486-NEXT:    pushl $1
; I486-NEXT:    pushl %ecx
; I486-NEXT:    pushl %eax
; I486-NEXT:    calll __atomic_compare_exchange_8
; I486-NEXT:    addl $24, %esp
; I486-NEXT:    movl %ebp, %esp
; I486-NEXT:    popl %ebp
; I486-NEXT:    retl
entry:
  %r = cmpxchg i64* %p, i64 0, i64 1 seq_cst seq_cst
  ret void
}

