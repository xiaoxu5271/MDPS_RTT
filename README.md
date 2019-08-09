# MDPS 基于 RTT 

FLASH 分区情况：
```
[I/FAL] ==================== FAL partition table ====================
[I/FAL] | name       | flash_dev         |   offset   |    length  |
[I/FAL] -------------------------------------------------------------
[I/FAL] | bootloader | onchip_flash_128k | 0x00000000 | 0x00020000 |
[I/FAL] | app        | onchip_flash_128k | 0x00020000 | 0x000e0000 |
[I/FAL] | download   | norflash0         | 0x00000000 | 0x00100000 |
[I/FAL] | factory    | norflash0         | 0x00100000 | 0x00100000 |
[I/FAL] | filesystem | norflash0         | 0x00200000 | 0x00a00000 |
[I/FAL] =============================================================
```

#LOG :
1. [2019-08-05 15:11:17]支持OTA,外部SPI flash 内部flash 已经分好区

2. [2019-08-07 17:47:17]增加RW007 ，目前RW007与以太网共存的话会导致 RW007无法获取DNS； 增加DW1000等相关文件

3. [2019-08-08 14:52:04] 修改DW1000 SPI驱动相关代码 暂不好用。

4. [2019-08-09 19:02:46]解决DW1000 send err 问题，原因是SPI1使用ABP2-84M ,SPI2/SPI3使用ABP2-42M, 应该分出2.63M时钟。  既SPI2 使用16分频。
