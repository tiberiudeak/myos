;;; basic file table, string consists of '{filename1-sector#,...}'
db '{calculator-06,testfile-07}'

TIMES 512-($-$$) db 0	; pad file to 512 bytes