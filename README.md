# openmv_for_hpm

#### 介绍
此为openmv4.3.0在先楫官方hpm6750evkmini移植的仓库。

openmv移植版本为4.30，直接可使用最新openmvIDE

micropython版本为V1.8

hpm_sdk目前为v0.13.0 

编译平台使用segger的IDE--> SEGGER Embedded Studio for RISC-V 6.30  最好使用这个版本！！

交流-->QQ:1084534438

#### 使用说明
ses工程里面 有两个版本：
一是debug版本，图像采集处理相关缓冲都在16bit SDRAM
二是realease版本，图像采集处理相关缓冲都在SRAM
默认优化debug版本

#### TODO:
micropython和Openmv整体移植                            OK
openmvIDE对接                                         OK
LED module对接                                        OK
time module对接                                       OK
sensor module对接(支持了OV2640 OV5640 OV7670 OV9650)   OK
image module对接  基本可运行所有Image算法例子            OK

采集图像优化            ing
图像处理优化            ing
OV7725对接             ing
其余外设module对接      ing


#### 参与贡献

1.  Fork 本仓库
2.  新建 Feat_xxx 分支
3.  提交代码
4.  新建 Pull Request


#### 特技

1.  使用 Readme\_XXX.md 来支持不同的语言，例如 Readme\_en.md, Readme\_zh.md
2.  Gitee 官方博客 [blog.gitee.com](https://blog.gitee.com)
3.  你可以 [https://gitee.com/explore](https://gitee.com/explore) 这个地址来了解 Gitee 上的优秀开源项目
4.  [GVP](https://gitee.com/gvp) 全称是 Gitee 最有价值开源项目，是综合评定出的优秀开源项目
5.  Gitee 官方提供的使用手册 [https://gitee.com/help](https://gitee.com/help)
6.  Gitee 封面人物是一档用来展示 Gitee 会员风采的栏目 [https://gitee.com/gitee-stars/](https://gitee.com/gitee-stars/)
