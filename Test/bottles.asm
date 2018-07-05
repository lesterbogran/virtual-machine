// 99 bottles of beer in pure NINJA Assembler
// Nov 2017, Sebastian Walter

pushc 99 //init count
popg 0

//loop header
loop:
pushg 0
wrint
call first //" bottles of beer on the wall, "
pushg 0
wrint
call second //" bottles of beer."
pushc 10
wrchr //new line
call third //"Take one down and pass it around, "
pushg 0
pushc 1
sub
dup
popg 0
wrint
call fourth //" bottles of beer on the wall."
pushc 10
wrchr //new line
pushc 10
wrchr //new line

//loop footer
pushg 0
pushc 1
eq
brf loop

//last lines
pushc 1
wrint
call first
pushc 1
wrint
call second
pushc 10
wrchr
call third
pushc 110
wrchr
pushc 111
wrchr
pushc 32
wrchr
pushc 109
wrchr
pushc 111
wrchr
pushc 114
wrchr
pushc 101
wrchr
call fourth
pushc 10
wrchr
pushc 10
wrchr
//end this shit
halt

first:
    pushc 32
    wrchr
    pushc 98
    wrchr
    pushc 111
    wrchr
    pushc 116
    wrchr
    pushc 116
    wrchr
    pushc 108
    wrchr
    pushc 101
    wrchr
    pushc 115
    wrchr
    pushc 32
    wrchr
    pushc 111
    wrchr
    pushc 102
    wrchr
    pushc 32
    wrchr
    pushc 98
    wrchr
    pushc 101
    wrchr
    pushc 101
    wrchr
    pushc 114
    wrchr
    pushc 32
    wrchr
    pushc 111
    wrchr
    pushc 110
    wrchr
    pushc 32
    wrchr
    pushc 116
    wrchr
    pushc 104
    wrchr
    pushc 101
    wrchr
    pushc 32
    wrchr
    pushc 119
    wrchr
    pushc 97
    wrchr
    pushc 108
    wrchr
    pushc 108
    wrchr
    pushc 44
    wrchr
    pushc 32
    wrchr
    ret

second:
    pushc 32
    wrchr
    pushc 98
    wrchr
    pushc 111
    wrchr
    pushc 116
    wrchr
    pushc 116
    wrchr
    pushc 108
    wrchr
    pushc 101
    wrchr
    pushc 115
    wrchr
    pushc 32
    wrchr
    pushc 111
    wrchr
    pushc 102
    wrchr
    pushc 32
    wrchr
    pushc 98
    wrchr
    pushc 101
    wrchr
    pushc 101
    wrchr
    pushc 114
    wrchr
    pushc 46
    wrchr
    ret

third: 
    pushc 84
    wrchr
    pushc 97
    wrchr
    pushc 107
    wrchr
    pushc 101
    wrchr
    pushc 32
    wrchr
    pushc 111
    wrchr
    pushc 110
    wrchr
    pushc 101
    wrchr
    pushc 32
    wrchr
    pushc 100
    wrchr
    pushc 111
    wrchr
    pushc 119
    wrchr
    pushc 110
    wrchr
    pushc 32
    wrchr
    pushc 97
    wrchr
    pushc 110
    wrchr
    pushc 100
    wrchr
    pushc 32
    wrchr
    pushc 112
    wrchr
    pushc 97
    wrchr
    pushc 115
    wrchr
    pushc 115
    wrchr
    pushc 32
    wrchr
    pushc 105
    wrchr
    pushc 116
    wrchr
    pushc 32
    wrchr
    pushc 97
    wrchr
    pushc 114
    wrchr
    pushc 111
    wrchr
    pushc 117
    wrchr
    pushc 110
    wrchr
    pushc 100
    wrchr
    pushc 44
    wrchr
    pushc 32
    wrchr
    ret

fourth:
    pushc 32
    wrchr
    pushc 98
    wrchr
    pushc 111
    wrchr
    pushc 116
    wrchr
    pushc 116
    wrchr
    pushc 108
    wrchr
    pushc 101
    wrchr
    pushc 115
    wrchr
    pushc 32
    wrchr
    pushc 111
    wrchr
    pushc 102
    wrchr
    pushc 32
    wrchr
    pushc 98
    wrchr
    pushc 101
    wrchr
    pushc 101
    wrchr
    pushc 114
    wrchr
    pushc 32
    wrchr
    pushc 111
    wrchr
    pushc 110
    wrchr
    pushc 32
    wrchr
    pushc 116
    wrchr
    pushc 104
    wrchr
    pushc 101
    wrchr
    pushc 32
    wrchr
    pushc 119
    wrchr
    pushc 97
    wrchr
    pushc 108
    wrchr
    pushc 108
    wrchr
    pushc 46
    wrchr
    ret
    