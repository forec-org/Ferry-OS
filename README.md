# Ferry OS
2017 操作系统课程设计，由 [@FlyingX](https://github.com/non1996) 和 [@Forec](http://forec.cn) 实现。在现有操作系统上层软件定义一个简单的单进程操作系统。
通过软件模拟计算资源、内存管理、硬盘、外设等。Ferry OS 可以灵活的响应配置变化，支持虚拟内存。

此外，[@FlyingX](https://github.com/non1996) 设计了一种类汇编语言。他同时为 Ferry OS 编写了一个编译器，编译后的字节码可在 Ferry OS 模拟的虚拟机上运行。该语言支持
基本四则运算、函数定义和调用、条件分支、循环、系统调用等。

Ferry OS 提供了基于现有底层文件系统的 Shell 交互界面，其使用方式与传统 Unix 风格控制台类似，支持基本的文件、目录操作以及一些其他常见命令。

![Ferry OS](http://7xktmz.com1.z0.glb.clouddn.com/ferry_os_screen.png)

# 支持
* 硬件可配置化
* FlyingX ASM 编译器
* 信号量
* 内存分页管理
* 虚拟机内存动态分配
* 内存预热和虚拟内存

# 一个空转 FASM 程序示例
```$fasm
Func loop {
LoopStart:
	CallCore incFreeTime
	Jmp LoopStart
}

Func main {
	Call loop
}
```

# Shell 命令
* 常见命令：`ls`、`cd`、`cp`、`mv`、`rm`、`touch`、`cpdir`、`mvdir`、`rmdir`、`mkdir`
* `./FILE.FSE`：Ferry OS 默认的可执行文件后缀名为 `.FSE`，可以通过指定可执行文件路径启动进程
* `kill [PID]`：结束编号为 Pid 的进程
* `top`：查看当前系统硬件、进程使用情况
* `watch`：查看进程输出结果，由于 Ferry OS 为单进程系统，输出与 Shell 将会冲突，因此通过 `watch` 观察进程的结果。`watch` 默认会从上次查看的位置之
后输出新的结果，参数 `-c` 将输出从 Ferry OS 开始执行到当前时刻全部进程输出结果。`watch` 指令参数数量没有限制，可以通过在参数中指定 pid 来查看特定进程
的输出，如 `watch 1 3 18`，只会显示进程编号为 1、3、18 的进程输出结果。
* `vi` 和 `nano`：文本编辑，使用的是上层文件系统编辑器
* `cat`、`head`、`tail`：与传统 Shell 中 `head` 不同的是，参数无需加 `-`，如 `head 10` 即可。
* `shutdown`、`quit`、`exit`：关机 
* `compile [SOURCE_FILE] [OUTPUT]`：将 FASM 源码编译为 FSE 可执行文件。

# 编译及测试
* Ferry OS 依赖 Boost 和 GoogleTest（测试），请保证依赖已安装。
* 进入仓库目录
* `mkdir build && cd build`
* `cmake ..`
* `make`
* `./bin/runUnitTests` 以测试
* `./bin/ferry` 以启动 Ferry OS

# Tips
* Ferry OS 自带了一些可执行文件，位于 `fasm/bin` 目录下，`fasm/src` 中存放了部分 FASM 程序源码。
* 界面的 Loading 都是靠延时做的，认真就输了。
* 做的 Bug 多我们也很无奈，毕竟只有一星期。
* 有任何问题欢迎联系 Forec 或 FlyingX，对 FlyingX ASM 有问题请联系 FlyingX。

# 已知问题
* Shell 中对路径的支持有瑕疵（时间问题来不及修改），Ferry OS 的文件系统暂不支持绝对路径，只能相对当前路径操作。
* FlyingX ASM 编译器的跨平台支持。FlyingX 在 Windows 下开发了这个编译器，将其移植到 Forec 处整合时出现异常，在 Mac OS 无法正确解析，因此 Ferry OS 中预装的可执行文件均为 FlyingX 在 Windows 下编译后的文件。

# 许可证
本仓库中的代码均受仓库中 [LICENSE](https://github.com/forec-org/Ferry-OS/blob/master/LICENSE) 声明的许可证保护。
