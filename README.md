# 你的日记导入导出

#### 介绍
你的日记导入导出工具。
支持功能：
1. 导出日记为json文件
2. 导出日记为pdf文件
3. 从json文件导入日记


#### 使用说明

若不想自己编译，只想使用。<br>
则打开 [release页面 ](https://gitee.com/xx2211/nideriji_export_import/releases/r0.92) <br>
下载exe文件，运行后按提示操作即可。

#### 构建说明
构建工具，qt5.15.2 mingw<br>
步骤: file -> new projcet -> import project -> git clone -> choose -> 填入本项目git clone地址 -> next -> finish -> 选择 qt5.xx.x mingw xxbit -> configure project -> 点击左下角绿色三角即可run/debug <br>
 **注意**  <br>
若自己构建，需要提供opesll库的libcrypto-1_1-x64.dll和libssl-1_1-x64.dll两个文件。<br>
否则无法使用https协议(提示tls初始化失败)<br>
调试时把这两个文件放在编译器的bin目录即可。<br>
打包时把这两个文件和可执行文件放在一起即可。<br>

#### 其他说明
目前只能使用mingw编译，用msvc编译器有奇怪的bug。
