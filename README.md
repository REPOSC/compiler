# Compiler

### 1、先初始化表，然后再分析程序

* 根据不同的情况，对main函数传递不同的参数，参数有`--init` 和 `1.txt` （即存放源代码的文件名称）两种

* 如果第一次运行程序，项目目录夹下还没有生成 `saved_table` 文件时，先给main函数传递一个 `--init` 	参数运行一次程序（在vs中运行时，修改参数方法如下所示），根据文法产生表文件`saved_table`（Action表和Goto表）

* 当生成 `saved_table` 文件后，再将要分析的程序文件名作为参数运行一次程序，即可获得语法树，以及四元组文件
output_compile.txt，前者输出到屏幕上，后者可以直接打开查看。

* 以后如果不更改文法，分析其他程序时，跳过`--init`步骤直接编译，可以获得性能提升

### 2、添加参数调试方法

- vs中添加参数调试方法

    * 点击菜单栏中 `调试` ->  最下方的当前项目的 `属性`
    
    * 按照下图输入参数（此项目中为文件名）
    
    * 将所对应的文件放入项目的 `.vcxproj` 文件所在的目录里

    ![readme1](./readme1.png)

- g++编译方法

    * 输入命令 `g++ --std=c++11 main.cpp -o reposc.exe` 可以编译成可执行文件reposc.exe
    
    * 将所对应的文件和编译出的可执行文件放在一起，并带参数运行：
      `./reposc 1.txt`

### 3、安装masm32汇编器

[在这里下载masm32汇编器](http://www.masm32.com/download/masm32v11r.zip)

[如果是win10系统，请在这里下载masm32汇编器](http://www.masm32.com/w10download/m32v11w10.zip)

* 下载并安装masm32汇编器后，运行编译器，在最后一步指定masm32的安装目录，编译器自动调用masm32汇编器生成可执行文件main.exe

### 其他声明和辅助使用：

由于`--init`过程较慢（和计算机配置有关，一般需要1-2分钟运行完毕，Visual Studio的Debug模式可能需要更多时间）
我们已经上传了正确的saved_table，可以直接使用，不再需要`--init`步骤。

到此为止，我们实现了：

- 整数和浮点数的加减乘除运算，整数的取余运算

- 整数和浮点数的逻辑运算，包含 && || 和 ！

- 整数和浮点数的大小比较运算，包含 > < >= 和 <=

- 整数和浮点数的输入输出，请使用scanf和printf函数，printf每输出一个数自动换行

- if分支，do while, while和for循环

- break和continue，用于跳出循环

- 代码优化和类型检查

以下是一段示例代码，可以复制到文件然后直接把文件名作为参数编译：

```
void main(){
	int a = 10;
	scanf(a);
	for (double i = 0; i < a; i = i + 1){
		if (i == 2){
			continue;
		}
		printf(i);
	}
	if (3 && 2.5){
		printf(2.5);
	}
}
```