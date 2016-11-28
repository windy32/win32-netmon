#ifndef __LEAKDETECTOR_H
#define __LEAKDETECTOR_H
#define _CRTDBG_MAP_ALLOC 
#include <crtdbg.h>
//把分配内存的信息保存下来，可以定位到那一行发生了内存泄露。用于检测 new 分配的内存
#ifdef _DEBUG
#define new   new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif
void EnableMemLeakCheck();
#endif
//使用方法：
/*
将EnableMemLeakCheck();语句在主函数下运行，这个时候，crtdbg就会在VS的输出窗口中将内存泄漏的信息显示出来，如下所示：
Detected memory leaks!
Dumping objects ->
XXX\demo.cpp(262) : {132} normal block at 0x006149D8, 4 bytes long.
Data: <xV4 > 78 56 34 12 
Object dump complete.
注意：重点在这里“demo.cpp(262) : {132}”，其中小括号是源码的所在行，而大括号代表的是第几次分配内存，也就是malloc或者new了多少次
通过上述信息，就可以定位到对应的源码，假设源码比较难找的话，可以使用“_CrtSetBreakAlloc(132);”来设置断点，这个断点的意义就是在
第132次分配栈空间的时候，断点。这样VS就可以快速定位到此次内存泄漏的内存分配的地点，然后找到此变量的最终闲置地点，手动释放即可。
忘记还有一个更加方便的方法了，可以直接在输出窗口里的内存泄漏的报告位置，找到对应的…….cpp(262) : {132} normal……行，双击即可
定位到内存泄漏前，此次内存分配的代码位置。

//还有另外一种方法来检测内存泄漏情况，使用vld，也就是visual leak detector工具，这个东西比较方便，到官网下载vld安装包，安装即可。
然后在需要vld进行内存监测的项目的主函数main或WinMain的上方添加上其头文件“#include "vld.h"”即可，如果项目是控制台程序，其会将
内存泄漏报告输出到控制台，假设您在VS中调试程序时，此信息也会输出到输出窗口，当然，你也可以让检测报告输出到本地磁盘上，那么需要
将vld安装目录里的vld.ini文件的某些参数设置好，如：“ReportFile =.\memory_leak_report.txt”，再将其拷贝到项目源码的所在目录上。
*/