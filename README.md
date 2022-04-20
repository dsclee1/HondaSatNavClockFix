# HondaSatNavClockFix
Hardware Fix for Honda Sat Navs with GN-80 Furuno GPS modules displaying the incorrect time.

Based off the great work by @pdaderko (https://github.com/pdaderko/PIC-Projects/tree/main/Navi_Clock_Fix.X), here is an Arduino implementation of the code.

This code takes the serial bytes from the GN-80 module (which incorrectly believes we're 1024 weeks in the past), and corrects the date before it's sent on to the Honda Sat Nav.

An Arduino is a pretty over-powered device for performing this, but I had a few D1 minis lying around, and didn't have any PIC ICs.

To use this code you must strip down your entire Honda Sat Nav unit (which if you have an 8th Gen Euro Civic like I do means stripping out a lot of the interior trim to get to the unit in the first place), desolder the GN-80 GPS module (you'll need a hot air soldering station), and then figure out how you'd like to mount the D1 Mini. I drilled a hole in the bottom of the Sat Nav unit and pulled the cables through to some crib board, which I put in a 3D printed box I mounted above the Sat Nav.

My advice is... don't attempt this unless you're extremely happy using a hot air soldering station and have a really good system for remembering which Sat Nav screws go where! There's also the possibility of a software fix... but I can't see it coming from Honda. My testing showed that when the GN-80 starts sending dates in 2003 and onwards the clock will become adjustable again. Honda have previously stated "the problem will fix itself in August", which is partly true as if we add our 1024 weeks to 2003-01-01 we get to around 2022-08-17. You'll then be able to manually adjust the clock, but any auto-adjustments for daylight savings won't work (the Sat Nav will think it's January in August)... If you're in a country that has the Sat Nav with the calendar page I imagine that will also stay in 2003.

The only other software fix would involve modifying the Sat Nav WinCE files themselves to perform the date calculation. I've had a go using Ghidra to decompile the DLLs (for a SuperH-4 Little-Endian 32Bit processor), and managed to find sections in DiagFunction.dll and DiagAPI.dll which read the GN-80 serial bytes (there are references to the 0x75 byte, and functions to convert the bytes) to force the display of a different date... unfortunately this didn't affect the clock, and I didn't try and insert the extra code needed to do the date calculation properly. If anyone is interested in investigating this further please get in contact.

Pinout basics are:

Duplicate existing connections to the GN-80. Apart from for Pin 3, which we need to intercept and change.

Pin 3 to D1 Mini RX

D1 Mini TX back to Sat Nav where Pin 3 would have been returned

Pin 9 (3.3v) to D1 mini 3.3V

Pin 8 (Ground) to D1 mini Ground


Some images of my set up:

![IMG_20220413_171719](https://user-images.githubusercontent.com/4020424/164274492-0ee87484-ac0d-4a98-bfee-8869be3e96ae.jpg)
![IMG_20220413_171701](https://user-images.githubusercontent.com/4020424/164274634-42b2a513-763b-4270-9303-028acace3de1.jpg)
![IMG_20220413_171648](https://user-images.githubusercontent.com/4020424/164274639-85eb5107-f44d-4dbf-a927-5eb6cf10e667.jpg)
![IMG_20220413_172821](https://user-images.githubusercontent.com/4020424/164274785-56fd9b02-67a6-4cc3-8c67-20b378da5d66.jpg)
![IMG_20220413_173138](https://user-images.githubusercontent.com/4020424/164274794-d25ca7c4-4470-4c2a-a57c-9b72621e9ced.jpg)
![IMG_20220414_094318](https://user-images.githubusercontent.com/4020424/164274993-4b403a73-a4ec-40fc-b059-1c3b8314ee96.jpg)
