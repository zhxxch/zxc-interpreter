# zxc-interpreter

（未完工，正在调试）

C语言（子集）解释器：

仅限32位。

支持int、 char、if、else、return；

printf(arg1, arg2)、fopen、fread、malloc、free、memmove、memchr、exit

没有循环语句，但是尾递归没有额外开销，可以声明函数原型。

值得注意的与ISO-C的区别：

if else 必须使用复合语句（花括号）

&&、|| 具有相同优先级；lt, gt, le, ge, eq ne 具有相同优先级