;;; basic file table, string consists of '{filename1-sector#,...}'
db '{calculator-04,testfile-06}'

TIMES 512-($-$$) db 0	; pad file to 512 bytes