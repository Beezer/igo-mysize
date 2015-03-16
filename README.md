# Introduction

iGO-MySize allows you to resize iGO 2008 to any size you wish, primarily for embedding in car PC applications like FreeICE or RoadRunner. **It does this with little or no impact on CPU usage**

# Quick Start

You just need to copy iGO-MySize.exe and iGO-MySize.dll from the distribution root directory to your iGO8 folder. Make sure you run with arguments as follows, either from a shortcut or from your front end

iGO-MySize.exe  -width=640 -height=384

You **must** have both width and height set or it will not resize. Make sure you keep the same format as above, a single "-" and an "=" sign.

You can call iGO-MySize from anywhere and it will run iGO8 properly without complaining that it can't find ctype.dat.

**Please provide feedback to Beezer on Digital Car UK forums http://www.digital-car.co.uk/forum/showthread.php?t=9335**

![https://lh4.googleusercontent.com/-J4Ahmk0Yu7g/Tek568zLqbI/AAAAAAAAACQ/IhvTieaXhuM/s400/igo-in-freeice.jpg](https://lh4.googleusercontent.com/-J4Ahmk0Yu7g/Tek568zLqbI/AAAAAAAAACQ/IhvTieaXhuM/s400/igo-in-freeice.jpg)


# Optional Install Steps

The default release contains a DLL that's compiled for any CPU & performs fast resizing. However, you also have a directory called "Optional DLL's". This contains alternative DLL's that you can use if you want to try them. They are split into QUALITY and SPEED. The QUALITY DLL's use something similar to bilinear filtering when they resize whilst the SPEED DLL's just drop lines. QUALITY DLL's take more CPU. I've found that as a general rule, when you're resizing upwards, use SPEED DLL's and when you're resizing downwards, use QUALITY DLL's.

Each of QUALITY or SPEED also contain 3 folders. ANY, SSE and SSE2. These contain the **same** dll functionality wise as in the root directory, except they are compiled for ANY, SSE and SSE2 respectively.


# Geeky Stuff

This solution consists of 2 programs. A loader and a dll. The loader causes the dll to be injected into iGO hooking the BitBlt function of GDI & overwrites it with a custom function that uses StretchBlt to resize the window. The loader also hooks the WM\_LBUTTONDOWN,  WM\_LBUTTONUP and WM\_MOUSEMOVE messages & resizes the coordinates approprately. Lastly, the loader populates the private data area of the iGO window with the scale factor. The loader then quits & iGO continues on its own, using the overloaded dll.

# Changelog

## Beta 1
Added optional DLL's for QUALITY and SPEED, each of which comes in a ANY, SSE and SSE2 flavour. I hope to get feedback on if these different CPU compiled versions even make any difference to speed. If not, I'll just distribute ANY in future.

## Alpha 4
Discovered that I compiled the DLL with SSE2 optimizations, which meant that it wouldn't work on Pentium 3. I've recompiled with neither SSE nor SSE2 optimizations as standard.

Also, caught more mouse events which mean that the button presses work much better now.

## Alpha 3
Improved BitBlt hook by changing the stretch mode from HALFTONE to COLORONCOLOR. Seems to halve CPU when the window size is large \o/

## Alpha 2
Improved hook code for Mouse and BitBlt. Alpha1 found the iGO window and retrieved our private data for every single call, now it only does it once & stores it locally in a static. Should be much more efficient.

## Alpha 1
First release.
