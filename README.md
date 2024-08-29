# 简易 Huffman 编码文件压缩器

一个简单的，基于 Qt C++ 与 Huffman 编码的文件压缩器。  
支持**可视化界面**，**单文件解压缩**，**分离密钥文件解压缩**。  


## Features  

1. 可视化界面  
基于 Qt6 C++ 开发的简易界面，支持树状文件视图。  

2. 单文件解压缩  
通过统计文件字节流中，每个字节的出现频率，构建 Huffman 编码，并生成压缩后的文件。  
基于字节流，支持各种文件类型。  
压缩文件结构见 `Project HUFC.txt` 。

3. 分离密钥  
压缩操作会将**字节词频数据**写入压缩后的文件，以便解压时能读取并重建 Huffman 编码树。  
但也可以选择将词频数据**不写入**压缩后文件，而是单独存储，作为密钥文件，实现加密效果。  
支持使用密钥文件解压。  


## Build  
使用 Qt Creator 与 Qt6，将 `HUFC.pro` qmake 文件导入 Qt Creator，并配置开发套件。


## Notice
缺少优化，速度较慢。没有并发编程，操作大文件时 UI 会被阻塞 ;)  