“例子代码.txt”中包含一些sql语句，有正确的，也有错误的（比如duplicate），选择性复制粘贴执行。
“test.txt”中有少量sql语句，用于执行脚本文件测试。
“test1.txt”中含有较多sql语句（300行左右），用于执行脚本文件测试。

执行以上两个脚本文件，请使用以下语句：

execfile test.txt;

execfile test1.txt;

需要注意的是test1中只包含建表和插入语句，如需查看结果，请自行select（在“例子代码”中有）