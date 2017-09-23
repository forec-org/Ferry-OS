//
// Created by 王耀 on 2017/9/18.
//

#ifndef SDOS_MACRO_H
#define SDOS_MACRO_H

#define _CPU_CORE_NUM 2
#define _CPU_CPU_RATE 5                    // Hz

#define _DISK_ROOT_PATH "./fs"             // 上层文件系统根目录
#define _DISK_BLOCK_SIZE 512               // Byte  硬盘块大小
#define _DISK_BLOCK_COUNT 8388608          // Block 硬盘块数
#define _DISK_BUFFER_DEFAULT_BLOCK 2       // Block 硬盘缓冲区大小
#define _DISK_BUFFER_MAX_BLOCK 16          // Block 硬盘最大缓冲区大小
#define _DISK_BUFFER_MIN_BLOCK 0           // Block 硬盘最小缓冲区大小
#define _DISK_DEFAULT_BLOCKS_PER_FILE 4    // Block 每个文件句柄默认的缓冲区大小

#define _MEM_DEFAULT_CAPACITY 1024         // Page  机器物理内存大小
#define _MEM_DEFAULT_PROCESS_PAGE 16       // Page  已作废
#define _MEM_DEFAULT_STACK_PAGE 1          // Page  每个进程默认的栈大小
#define _MEM_DEFAULT_PAGE_BIT 20           // Bit   每个页的大小(2^)
#define _MEM_DEFAULT_PAGE_SIZE (1 << _MEM_DEFAULT_PAGE_BIT)   // Byte  每个页的字节数

#define _OS_MEM_DEFAULT_OS_USED_PAGE (_MEM_DEFAULT_CAPACITY >> 3)   // Page  操作系统独占内存大小
#define _OS_MEM_SWAP_PAGE _MEM_DEFAULT_CAPACITY                     // Page  虚拟内存交换空间大小
#define _OS_MEM_SWAP_BLOCK (_OS_MEM_SWAP_PAGE * (_MEM_DEFAULT_PAGE_SIZE / _DISK_BLOCK_SIZE))   // 虚拟内存交换空间的硬盘块数
#define _OS_MEM_SWAP_SIZE ( _OS_MEM_SWAP_PAGE << _MEM_DEFAULT_PAGE_BIT)  // Byte  虚拟内存交换空间字节数
#define _OS_BOOT_MEMORY_KB 512             // KByte  操作系统引导区内存大小
#define _OS_MAXIMUM_TASKS 64               // 操作系统能够支持的最大进程数
#define _OS_MAXIMUM_TASK_PAGE (_MEM_DEFAULT_CAPACITY << 1)          // Page  每个进程能使用的最大虚拟内存大小
#define _OS_EXEC_FILE_EXT ".FSE"           // 可执行文件后缀
#define _OS_TICKS_PER_SEC 2                // 每秒时钟中断数
#define _OS_TICKS_PER_SCHED 5              // 调度频率
#define _OS_MAX_SEM 64                     // 最多可支持信号量数量

#endif //SDOS_MACRO_H
