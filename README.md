# zxc-interpreter

C语言（子集）解释器：

第一个命令行参数是被解释C代码的文件名，余下参数作为被解释程序的命令行参数；返回值是被解释程序的返回值。

*此解释器可以解释自身，如：“.\zxcc.exe .\zxcc\zxcc.c test-fac.txt”*

仅限32位（因为使用int存储指针，而MSVC使用LLP64）。

支持int、 char、if、else、return；

printf(arg1, arg2)、fopen、fread、malloc、free、memmove、memchr、exit。

没有循环语句，但是尾递归没有额外开销，可以声明函数原型。

#值得注意的与ISO-C的区别：

不支持 for, while, do while, continue, break, switch, case 等；

if else 必须使用复合语句（花括号）；

&&, || 具有相同优先级；<, >, <=, >=, ==, != 具有相同优先级。