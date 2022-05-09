这是一个用于c++和脚本语言进行交互的库。
内部独立实现了一个非常完善的c++反射机制
可以很方便的将C++的函数导出到脚本。
可以很方便的将C++的类导出到脚本，并且允许在脚本里面从C++的类派生新的脚本类：
一、脚本类可以直接从C++类派生
二、脚本类可以调用C++基类的函数，访问基类的成员
三、脚本类可以重载C++基类的虚函数
四、支持VSCode的调试协议，可以使用VSCode直接进行调试脚本语言（将vscode_extensions/script-debugger目录复制到vscode的插件目录下即可）。

This is a library for interaction between C + + and scripting languages.
A very perfect C++ reflection mechanism is implemented independently inside.
It is very convenient to export C++ functions to scripts.
It is very convenient to export C++ classes to scripts, and it is allowed to derive new script classes from C++ classes in scripts:
1、 Script classes can be derived directly from C++ classes
2、 The script class can call the functions of the C++ base class and access the members of the base class
3、 Script classes can overload virtual functions of C++ base classes
4、 Support the debugging protocol of vscode. You can use vscode to directly debug the script language (Please copy the vscode_extensions/script-debugger to the extensions directory of vscode).
