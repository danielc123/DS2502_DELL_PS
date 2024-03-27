A fork of https://github.com/garyStofer/DS2502_DELL_PS from Gary Stofer

I found this code on Internet a while ago, I tried to find the source, but I really couldn't find it. Thank you to Gary and to the Guy who added the more functions.

It works fine. I've just modified a very few things including more power codes (45W and 65W)


You will need:

* An Arduino board, I have used a Mini Pro that I had laying around.
* The DS2502 Interface originally proposed by the author at /CAD_files, I've just added the Schematic in PDF 
* A 5V and a 12V power supply, needed to program the DS2502
* A blank DS2502 chip

Instructions:

Connect the interface to the Arduino and insert the DS2502.
Modify the Arduino code if you need to change the assigned pins, I am using the pin 6 to read/write and the pin 3 for the program pulse.
Just upload the code into the Arduino, and connect it to the serial terminal at 115200bps, you will get a menu, select the desired power code and burn it.

Remmember, the DS2502 cannot be erased and reprogrammed.
