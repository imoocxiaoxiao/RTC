## 获取实时时间的通用模块,内含各RTC驱动程序，如：DS1302

### 此模块可实现
   *  实时更新，并读写实时时间(ZipTime格式)
   * 可选实现：提供读写备份数据功能

### 主要文件说明：
   #### RTC.h 通用接口
   #### RTC_Bak 保存实时RTC功能模块, 如：关机时间
   #### RTC_cbZipTime.c 对接ZipTime组件时的实现(自动更新ZipTime组件的实时时间)

### RTC硬件驱动：
        * RTC_DS1302.c  RTC.h 通用接口在DS1302中的实现
       * DS1302驱动  (DS1302目录里)：
          + DS1302Drv:  DS1302驱动程序(直接IO口驱动，依赖于IoCtrl.h)
          + DS1302.h  DS1302寄存器定义

#### STM32F4x驱动：
        * RTC_STM32F4x/ RTC_STM32F4xx  RTC.h 通用接口在STM32F4x的RTC寄存器中的实现(时间走的不对，未找出问题)


