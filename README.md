# f/term F256

## Overview

f/term is a very basic terminal emulator for the [Foenix F256jr and F256K retro computers](http://wiki.f256foenix.com/index.php?title=Main_Page). If you do not own a Foenix computer, or if you don't know what one is, this software should have limited interest for you. Go back now before you are sucked into the world of programming and using 8-bit and 16-bit computers as if it was the 1980s. You have been warned. 

If you are a brand new F256 owner, congratulations! It's a great piece of hardware, and ton of fun to program. If you haven't already, jump on the Discord and share what you are working on.

### Why you would want this

You have:
1. An F256K or Jr with the "classic" memory layout.
2. EITHER a Wifi232 modem and a desire to go BBS'ing OR another retro, classic, or modern computer with an RS-232 port and a null cable modem. 

## Features

- text-only terminal communications via serial port. 
- understands many ANSI codes. Obviously, it does not process codes for alternative character sets, more than 16 colors, etc. 
- scrolls terminal area
- wow, right? hehe.

### Possible Future Features

- XMODEM or ZMODEM download capability
- Built-in AT command support for changing baud rate of Wifi232 modem, setting and using phone book, etc. 
- Built-in phone book for BBSes?
- Help page? 

## Usage

### Installation

1. Copy "fterm.pgZ" onto your F256 SD card. You can do with from your modern computer, or you can use FOENIXMGR with, e.g., "python3 $FOENIXMGR/FoenixMgr/fnxmgr.py --copy fterm.pgZ"
2. Start up f/manager if it is not already running. Select "fterm.pgZ" from the file list and hit Enter to launch it. If you do not have f/manager installed in flash memory, you can also load f/term from DOS or from SuperBASIC. 

### Starting a Session

1. Start up the app
2. Match the baud rate it communicates at, to the baud rate your Wifi232 modem is set to. For example, if the Wifi232 is expecting 1200 baud communication, set the baud rate of f/term to 1200. ALT-1 is the slowest speed (300), ALT-0 is the highest speed (115200). 4800-9600 is probably the sweet spot in terms of reliability. 
3. Select the desired font/character set. ALT-I selects an IBM-style font in ANSI layout, ALT-A selects a Foenix style font in ANSI layout, and ALT-F selects the Foenix font, in Foenix layout. If you are connecting to a text BBS or another Foenix, ALT-F is a good option. If connecting to an ANSI BBS use either ALT-I or ALT-F. 
4. (Optional) Type "at" and hit Enter and make sure your Wifi232 modem responds with "OK". If it doesn't, you probably aren't ready to dial anywhere. If you are connecting to another computer via NULL modem cable, you can just do type away, no AT commands necessary. 
5. Enter the AT command for dialing the BBS you want to log into. For example, "atdt  cqbbs.ddns.net:6800" (plus Enter) will dial the Captain's Quarters II BBS. Your wifi modem may also have a phonebook that you can program with shortcuts (e.g, "atds=0" to dial the first number in the phone book).


### Commands

All commands are initiated with ALT key combinations. There is no separate "menu" or "command" mode. 

#### Change baud
- **ALT-1**: 300 baud
- **ALT-2**: 1200 baud
- **ALT-3**: 2400 baud
- **ALT-4**: 3600 baud
- **ALT-5**: 4800 baud
- **ALT-6**: 9600 baud
- **ALT-7**: 19200 baud
- **ALT-8**: 38400 baud
- **ALT-9**: 57600 baud
- **ALT-0**: 115200 baud
- **ALT-R**: Reset serial connection. If you change the Wifi modem's speed, you might get a communication error. After matching the new speed, if it appears stuck, ALT-R may fix it. 

#### Change font / character set

- **ALT-I**: IBM font, ANSI encoding
- **ALT-A**: Foenix font, ANSI encoding
- **ALT-F**: Foenix font, Foenix encoding

#### Change colors

- **ALT-C**: cycle to the next foreground color. This is mainly useful for text-only BBSes, as ANSI BBSes tend to be pretty trigger-happy when it comes to colors. 

#### Change clock

- **ALT-T**: lets you set the built-in real-time clock. Enter date/time in "YYYY-MM-DD HH:MM" format. 


