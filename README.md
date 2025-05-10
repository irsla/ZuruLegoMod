# ZuruLegoMod
Hardware Mod for the Zuru Max Retro Computer Lego

This project was created for a friend (@CabriDIY) , it hasn't been curated and will most certainly not get any update.

![image](https://github.com/user-attachments/assets/9784b16d-be13-455c-8a1e-974622e521ba)

![20250426_161508](https://github.com/user-attachments/assets/d9971d33-a0ba-4d6f-ac3d-542408541825)

![20250426_164929](https://github.com/user-attachments/assets/e7054f21-e6ce-4481-84d1-72a42ba5a1b4)

![20250509_092854](https://github.com/user-attachments/assets/ebca3af8-e9ff-4966-b554-2924d0e4e740)

![image-5](https://github.com/user-attachments/assets/a29fa141-e2ae-4082-ae0b-96918fb2d1aa)

![image-4](https://github.com/user-attachments/assets/f75d5424-5c11-482f-9703-b33d3728abb0)


## Building the firmware

The source code should build with any arm-none-eabi- GCC compilers.

```console
$ make
[...]
arm-none-eabi-size build/firmware_LegoZuru.elf
   text	   data	    bss	    dec	    hex	filename
 737740	    200	 191496	 929436	  e2e9c	build/firmware_LegoZuru.elf
arm-none-eabi-objcopy -O ihex build/firmware_LegoZuru.elf build/firmware_LegoZuru.hex
arm-none-eabi-objcopy -O binary -S build/firmware_LegoZuru.elf build/firmware_LegoZuru.bin
```

## Hardware

**The hardware was generated on KiCad**
  - schematics (pdf)
  - gerbers
  - BOMs
  - positions
  - 3D stl
  - Tutorial - photo only (PDF)

Project was manufactured and assembled by JLCPCB
 
**Not Included in the documentation:**
 - OLED Display: NHD-1.8-160128B, datasheet in folder Hardware
 - FPV ribbon: 10pin 0.5mm pitch type A (careful **not** to use a type B)
 - Power: any 5V, we used a 5V only usb cable

## Some photos

### 211 lego pieces:

![20250312_195518](https://github.com/user-attachments/assets/14f7dc8d-69ce-4656-a960-9de76e76a244)

### very first try of a GIF (I think the color was not correctly parsed)

![20250409_224945](https://github.com/user-attachments/assets/2acd9c76-202a-4ced-beb1-8f93f648540d)

### Oh No I have a dead pixel

![20250323_222348](https://github.com/user-attachments/assets/80375a22-b16c-4055-8a00-666ea80504af)

### first main board was incorrect (I had to bodge)

![signal-2025-04-14-19-46-56-328](https://github.com/user-attachments/assets/6500ea8a-21be-4862-96d1-45752aac67be)



