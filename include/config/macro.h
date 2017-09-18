//
// Created by 王耀 on 2017/9/18.
//

#ifndef SDOS_MACRO_H
#define SDOS_MACRO_H

#define _CPU_CORE_NUM 2
#define _CPU_CPU_RATE 1.0

#define _DISK_BLOCK_SIZE 512               // Byte
#define _DISK_BUFFER_DEFAULT_BLOCK 2
#define _DISK_BUFFER_MAX_BLOCK 16
#define _DISK_BUFFER_MIN_BLOCK 0
#define _DISK_DEFAULT_BLOCKS_PER_FILE 4

#define _MEM_DEFAULT_CAPACITY 1024         // Page
#define _MEM_DEFAULT_PROCESS_PAGE 16       // Page
#define _MEM_DEFAULT_STACK_PAGE 1          // Page
#define _MEM_DEFAULT_PAGE_BIT 10
#define _MEM_DEFAULT_PAGE_SIZE (1 << _MEM_DEFAULT_PAGE_BIT)   // Byte

#define _OS_MEM_DEFAULT_OS_USED_PAGE 128   // Page
#define _OS_BOOT_MEMORY_KB 512             // KByte

#endif //SDOS_MACRO_H
