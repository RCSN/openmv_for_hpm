# lv_demo_music

## Overview

The music player demo shows what kind of modern, smartphone-like user interfaces can be created on LVGL， and can play music.

Please refer to [official lv_demos github](https://github.com/lvgl/lv_demos)

## Project Configuration

- In the file `CMakeLists.txt`, set a matched audio codec type according to the development board schematic，e.g. "set(CONFIG_CODEC "sgtl5000")"

## Board Settings

Attach LCD panel to the board LCD interface.

Attach MicroSD card that stores songs in WAV format into the card slot;

According to project configuration, connect speaker to [DAO](lab_board_app_dao) interface if using DAO as player, connect headphone to [headphone](lab_board_app_headphone) interface if using audio codec as player.

## Running the Demo

lvgl music will be shown on the LCD panel and can operate play/pause, next, previous.

![lv_demo_music](../../../../../assets/sdk/samples/lv_demo_music.gif "lv_demo_music")

## Remark

As framebuffer occupies a large amount of memory, please specify CMAKE_BUILD_TYPE as flash_sdram_xip. The following commands can be executed:
```
cmake -GNinja -DBOARD=hpm6750evkmini -DCMAKE_BUILD_TYPE=flash_sdram_xip ..
```
