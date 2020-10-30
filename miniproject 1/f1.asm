;�������� ������ ���197
;������� 12
;����������� ���������, ����������� �  ������� ���������� ���� � ���������
;��  ���� 0,1% �������� ������� 1/e^�   ���  ��������� ��������� x (������������ FPU)


format PE console
entry start

include 'win32a.inc'


section '.code' code readable executable

start:
    invoke printf, yValue ;y=1/(e^x)
    invoke printf, xValue ;x=
    invoke scanf,  istr, x ;input x

  FINIT
;teylor series
taylor:

  fld qword[a]  ;sign a=(-1)^n
  fld qword[lastnum]  ;lastnum=(n-1)!
  fld qword[n]  ;n
  fmulp  ;n*(n-1)!
  fst qword[lastnum]  ;lastnum=n!
  fdivp  ;(-1)^n/n!
  fld qword[lastx]  ;lastx=x^(n-1)
  fld qword[x]  ;x
  fmulp   ;x*x^(n-1)
  fst qword[lastx]    ;lastx=x^n
  fmulp   ;x^n*(-1)^n/n!
  fld qword[sum] ;put sum
  fcom qword[sumPrevious] ;real comparison
  fstsw ax ;saves the current value of the SR register
  sahf ;copy the contents of the AH register into the lower 8 bits of the flags register
  fstp qword[sum]  ;unload sum
  jz .end      ;if ZF=1, so sum==sumPrevious
  fld qword[sumPrevious]
  fstp qword[sumPreviousPrevious]
  fld qword[sum]  ; teylor sum from n-1
  fst qword[sumPrevious] ;save previous sum
  faddp      ;plus step from n
  fstp qword[sum]     ;sum=sum+x^n*(-1)^n/n!
  fld qword[a]  ;a=(-1)^n
  fld1  ;1
  fchs  ;-1 (sign change)
  fmulp ;(-1)*(-1)^n
  fstp qword[a] ;a=(-1)^(n+1)
  fld qword[n]  ;n
  fld1 ;1
  faddp ;n+1
  fstp qword[n] ; n=n+1
  jmp taylor    ;cycle

.end:

  fld qword[sum] ;previous sum
  fld qword[sumPreviousPrevious];difference
  fsubp;
  fabs ; module
  fstp qword[error] ;error
  invoke printf, ostr, dword[sum], dword[sum+4]
  invoke printf, errorStr, dword[error], dword[error+4]
  invoke printf, previousStr, dword[sumPreviousPrevious], dword[sumPreviousPrevious+4]
    ret


section '.data' data readable writable
  a dq -1.0    ;sign
  x dq 0.0   ;value
  lastx dq 1.0  ; previous x power
  lastnum dq 1.0;previous factorial value
  n dq 1.0  ;iterator
  yValue db 'y=1/(e^x)',10,0
  xValue db 'x=',0
  istr db '%lf',0
  ostr db 'y=%.30lf',10,0 ;answer
  sum dq 1.0 ;teylor series sum
  sumPrevious dq 0.0 ;teylor series sum
  sumPreviousPrevious dq 0.0 ;teylor series sum
  error dq 0.0 ;teylor series sum
  errorStr db 'error=%.30lf',10,0 ;error
  previousStr db 'previous-previus sum=%.30lf',10,0 ;previous-previus sum
  exceptionr db 'exception',10,0 ;previous-previus sum

section '.idata' import data readable
    library kernel, 'kernel32.dll',\
            msvcrt, 'msvcrt.dll'

    import kernel,\
           ExitProcess, 'ExitProcess'
    import msvcrt,\
           printf, 'printf',\
           sprintf, 'sprintf',\
           scanf, 'scanf'