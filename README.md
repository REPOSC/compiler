# Compiler

### 1、vs中添加参数调试方法

* 点击菜单栏中 `调试` ->  最下方的当前项目的 `属性`

* 按照下图输入参数（此项目中为文件名）

* 将所对应的文件放入项目的 `.vcxproj` 文件所在的目录里

  ![readme1](.\readme1.png)

### 2、g++编译方法

* 输入命令 g++ -finput-charset=gbk main.cpp -o reposc 可以编译成可执行文件reposc

* 将所对应的文件和编译出的可执行文件放在一起，并带参数运行：
  `./reposc 1.txt`
