/*

Programmer for DS2502 device used in  DELL 90Watt power supply to identify the power adapter as a 90W, 19.5V 4.6A DELL AC adapter

This sketch needs a DS2502 programmer interface board attached to arduino digital pins 6 and 7, fed with a 12V PS capable of 100ma
This sketch is running on a Pro-mini running at 16mhz, 5V from the on board 12 to 5V regulator and has been verified under Arduino 1.6.1 IDE, 

Eagle CAD files for programmer board can be downloaded from Github here: https://github.com/garyStofer/DS2502_DELL_PS
PCB boards can be ordered from OSH PCB here: https://oshpark.com/shared_projects/l4VC80Px

Sketch requires library  PJRC OneWire 2.0 library http://www.pjrc.com/teensy/td_libs_OneWire.html

Sketch is roughly based on the One-Wire example code provided  by Guillermo Lovato <glovato@gmail.com>

 The DS250x is a 512/1024bit add-only PROM(you can add data but cannot change the old one) that's used mainly for device identification purposes
 like serial number, mfgr data, unique identifiers, etc. It uses the Maxim 1-wire bus.

Use: Upon power-up of the Arduino the code is looking for an attached DS2502 device in a 3 second loop. if the device is found it will be programmed with the 
text in progStr and after wards the programmed block will be read back, bot in hex and as a string. The program then stops executing until reset is pressed.
Programming a device multiple times will not hurt or corrupt the device unless a different programming string or address is chosen.

Progress can be monitored via the Serial terminal in the IDE

 
 Sept. 2015 , Gary Stofer, GaryStofer@gmail.com
 This file archived at https://github.com/garyStofer/DS2502_DELL_PS

*/

#include <OneWire.h>

#define LEDRed_PIN 13
#define LEDGreen_PIN 8
#define ONE_WIRE_IO_PIN_read 6
#define PROG_PIN 3
#define ONE_WIRE_IO_PIN 6
#define DS2502DevID 0x09
#define DS2502DevID2 0x89
#define DELLchipID 0x11
 
// DS250x ROM commands
#define READ_ROM   0x33
#define MATCH_ROM  0x55
#define SKIP_ROM   0xCC
#define SEARCH_ROM 0xF0
#define ROM_SZ 8  // 8 bytes of laser etched ROM data , LSB = 0x09 == DS2502

// DS250x Memory commands -- can only be executed after one of the above ROM commands has been executed properly
#define READ_MEMORY  0xF0
#define READ_STATUS  0xAA
#define WRITE_STATUS 0x55
#define WRITE_MEMORY 0x0F

#define ProgLocation 0x00

#define ProgPulseUs 480    // length of time 12V prog pulse needs to be applied for 
OneWire ds(ONE_WIRE_IO_PIN);             // OneWire bus on digital pin 6
OneWire ds1(ONE_WIRE_IO_PIN_read);             // OneWire bus on digital pin 6

    char const *progStr ;
   
    byte data[64];                  // container for the data from device
    byte leemem[12];                // array with the commands to initiate a read, DS250x devices expect 3 bytes to start a read: command,LSB&MSB adresses

    byte crc;                      // Variable to store the command CRC
    byte crc_calc;
    
    byte  listingoption = 0;
    char prg_char , option_character;
    byte loopinput = 0;
    byte loopinput_b = 0;
    byte loopinput_c = 0;
    byte input_option = 0;
    byte entered = 0;
    byte index = 0;
    
char progStr240[] ="DELL00AC240195123CN0DF261478906750576A00"; 
char progStr180[] ="DELL00AC180195092CN0DF261478906750576A00";
char progStr130[] ="DELL00AC130195066CN0DF261478906750576A00"; 
char progStr90[] ="DELL00AC090195046CN0DF261478906750576A00"; 
char progStr65[] ="DELL00AC065195033CN0DF261478906750576A00"; 
char progStr45[] ="DELL00AC045195023CN0CDF577243865Q27F2A05"; 
char progStr_key[] ="DELL00AC240195123CN0DF261478906750576A00"; 

  
void setup()
{
  Serial.begin (115200);
  // Pin 13 has an LED connected on most Arduino boards:
  pinMode(LEDGreen_PIN, OUTPUT); 
  pinMode(LEDRed_PIN, OUTPUT); 
  digitalWrite(LEDGreen_PIN, HIGH); 
     
  // Pin 7 is the PRGM/ output, sending a low enables the programming voltage +12V onto the wire
  digitalWrite(PROG_PIN, HIGH);   // set Non Programming mode
  pinMode(PROG_PIN,OUTPUT);

  Serial.println F(("DELL Adapter Identification Chip Programmer (PSID)")); 
  Serial.println F(("Version 1.0"));
  Serial.println F(("DS2502 chip, possibly DS2501 (not tested) and similar are supported for PSID writing"));
  Serial.println F(("*****************************************************"));
}

// This is the magic string Dell laptops need to see from the device in order to not go into limp home mode 
// The interesting bit is the wattage 090 followed by the voltage 1950 and the amperage 4.6 
//char const *progStr ="DELL00AC090195046CN09T";


void loop()
{
  while (loopinput == 0) {
    if ( listingoption == 0){
      Serial.println F(("Will we program or read the  chip? P / R :"));
      listingoption = 1;
    }
    
    if (Serial.available()) {
       option_character = Serial.read();
      if ( option_character == 'P' ||  option_character == 'p' ) {
        listingoption = 0;
        loopinput_b = 0;
  
        while (loopinput_b == 0) {
          if ( listingoption == 0){
            Serial.println F(("Enter an option:"));
            Serial.println F(("1. for 45W"));
            Serial.println F(("2. for 65W"));
            Serial.println F(("3. for 90W"));
            Serial.println F(("4. for 130W"));
            Serial.println F(("5. for 180W"));
            Serial.println F(("6. for 240W"));
            Serial.println F(("7. for your own assignment"));
            listingoption = 1;
          }
  
          if (Serial.available()) {
            prg_char = Serial.read();
            if ( prg_char == '1') {loopinput_b = 1; writerom (45);  }
            if ( prg_char == '2') {loopinput_b = 1; writerom (65);  }
            if ( prg_char == '3') {loopinput_b = 1; writerom (90);  }
            if ( prg_char == '4') {loopinput_b = 1; writerom (130);  }
            if ( prg_char == '5') {loopinput_b = 1; writerom (180);  }
            if ( prg_char == '6') {loopinput_b = 1; writerom (240);  }
            if ( prg_char == '7') {loopinput_b = 1; uzivatelske_zadani(); writerom (1);  }     
            if ( prg_char != '7' && prg_char != '6' && prg_char != '5' && prg_char != '4' && prg_char != '3' && prg_char != '2' && prg_char != '1' &&  prg_char != 10 && prg_char != 13 ) {
              Serial.println ("Incorrect entry, redial:");  listingoption = 0; }
           }
        }
      }
      else if (  option_character == 'r' ||  option_character == 'R') {
        loopinput_c = 0;
        listingoption = 0;
  
        while (loopinput_c == 0) {
          if ( listingoption == 0){
          Serial.println F(("From read operation you want to do?:"));
          Serial.println F(("1. Data from Eprom"));
          Serial.println F(("2. Status from IC"));
          listingoption = 1;
          }
       
          if (Serial.available()) {
            prg_char = Serial.read();
            if ( prg_char == '1') {loopinput_c = 1; input_option = 0;  }
            if ( prg_char == '2') {loopinput_c = 1; input_option = 1;  }
            if ( prg_char != '2' && prg_char != '1' &&  prg_char != 10 && prg_char != 13 ) { 
              Serial.println ("Incorrect entry, redial:");  listingoption = 0; }
          }
        }   
       loopinput = 1;
       readrom(input_option); //precte ID adapteru
      }
      else if ( option_character == 10 ||  option_character == 13) {Serial.println(" ");}
   
      else {  listingoption = 0; Serial.println F(("Incorrect entry, redial:"));}
  
    }
  }
}


////////////////////////////////////////////////////////////////////////////
//Podprogramy
////////////////////////////////////////////////////////////////////////////

// Zacykleni na konci programu


void  uzivatelske_zadani(void){
  Serial.println F(("Enter your own programming data"));
  Serial.println F((" For example: DELL00AC240195123CN0DF261478906750576A00"));
  Serial.println F(("Follow this format and text length (40 prg_charu,  ack will be added automatically)"));
  Serial.println F((""));
  entered = 0;
  index = 0;
 
  while ( entered == 0){
    if (Serial.available() > 39 ) {
      while (Serial.available() > 0){
      progStr_key[index] = Serial.read();
      index++;
      }
      progStr_key[index] = 6;
      Serial.println F(("Your choice is:-)"));
      Serial.println (progStr_key);
      entered = 1;
    }
  }
}
//progStr_key

  
void vybernakonec( void)
{
  Serial.println F(("End of programm"));
  Serial.println F((""));
  Serial.println F(("Return to main menu:"));
  //while (1) 
  //{
  delay (1000);
  loopinput = 0;
  listingoption = 0;
    
}

// 12V programming pulse triggered
void 
ProgPulse( void)
{    
  digitalWrite(PROG_PIN, LOW);  
  delayMicroseconds(ProgPulseUs);
  digitalWrite(PROG_PIN, HIGH); 
  delayMicroseconds(100); 
}



//Adapter chip readout
void readrom(int volba){
  byte i,j;  
  digitalWrite(LEDGreen_PIN, LOW);   // set the LED off
  if (volba == 0)
  {
    if (ds1.reset())            // We only try to read the data if there's a device present
    {
      digitalWrite(LEDGreen_PIN, HIGH);   // LED on
      Serial.println F(("communication established"));
      // Read and display ROM data of device attached and check CRC 
      ds1.write( READ_ROM,1);
      Serial.println F(("ROM  Data: "));  
      for ( i = 0; i < 8; i++)     
      {
        data[i] = ds1.read();        
        Serial.print(data[i],HEX); 
        Serial.print(" ");          
      }
          
      Serial.println(".");
      crc_calc = OneWire::crc8(data, 7);
      if (crc_calc == data[7] )
      {
        Serial.println F(("ROM CRC agree"));
        if (data[0] == DS2502DevID || data[0]== DS2502DevID2)
        {
          Serial.println("Connected chip is DS2502 "); 
          Serial.print("Id chip is: ");
          Serial.print(data[0],HEX);
          Serial.println("h");
        }
        if (data[0] == DELLchipID)
        {
          Serial.println F(("DELL adapter is connected, not specified chip")); 
          Serial.print F(("Id chip is: "));
          Serial.print(data[0],HEX);
          Serial.println F(("h"));
        }
        if (data[0] != DELLchipID && data[0] != DS2502DevID && data[0]!= DS2502DevID2)
        {
          Serial.println F(("The connected chip is not fully identifiable")); 
          Serial.print F(("Id chip is: "));
          Serial.print(data[0],HEX);
          Serial.println F(("h"));
        }
      }
      else
      {
        Serial.println F(("ROM CRC disagree")); 
        vybernakonec();
        return; 
      } 
      Serial.println F(("Reading memory"));        
      ds1.reset(); 
      ds1.skip();                      // Skip ROM search, address the device no matter what
      leemem[0] = READ_MEMORY;        // command 
      leemem[1] = ProgLocation;       // low address byte
      leemem[2] = 0;                  // High address byte, always 0 for 2502
      ds1.write(leemem[0],1);        // Read data command, leave ghost power on
      ds1.write(leemem[1],1);        // LSB starting address, leave ghost power on
      ds1.write(leemem[2],1);        // MSB starting address, leave ghost power on
      crc = ds1.read();             // DS250x generates a CRC for the command we sent, we assign a read slot and store it's value
      crc_calc = OneWire::crc8(leemem, 3);  // We calculate the CRC of the commands we sent using the library function and store it
  
      if ( crc_calc != crc)        // Then we compare it to the value the ds250x calculated, if it fails, we print debug messages and abort
      {
        Serial.println F(("Disagree CRC"));
        Serial.print F(("Calculated CRC:"));
        Serial.println(crc_calc,HEX);    // HEX makes it easier to observe and compare
        Serial.print F(("DS250x calculated CRC:"));
        Serial.println(crc,HEX);
        vybernakonec();
        return;                     // Since CRC failed, we abort 
      }
      Serial.println("Data: "); 
      for ( i = 0; i < 41; i++)     
      {
        data[i] = ds1.read();        
        Serial.print(data[i],HEX);       
        Serial.print(" ");           
      }
      Serial.println();
      Serial.println("Translation:");
      data[41]=0;
      Serial.println((char *) data);  // string
      vybernakonec();
      return;  
    }
    else                             // Nothing is connected in the bus
    {
      Serial.println F(("Connect the communication"));
      delay(1500);
      readrom(volba);
    }
  }
  else {
    if (ds.reset())            // We only try to read the data if there's a device present
    {
      digitalWrite(LEDGreen_PIN, HIGH);   // LED on
      Serial.println F(("communication established"));
      // Read and display STATUS data of device attached and check CRC 
      // Read and display ROM data of device attached and check CRC 
      ds.write( READ_ROM,1);
      Serial.println F(("ROM  Data: "));  
      for ( i = 0; i < 8; i++)     
      {
        data[i] = ds.read();        
        Serial.print(data[i],HEX); 
        Serial.print(" ");          
      }
          
      Serial.println(".");
      crc_calc = OneWire::crc8(data, 7);
      if (crc_calc == data[7] )
      {
        Serial.println F(("ROM CRC agree"));
        if (data[0] == DS2502DevID || data[0]== DS2502DevID2)
        {
          Serial.println("Connected chip is DS2502 "); 
          Serial.print("Id chip is: ");
          Serial.print(data[0],HEX);
          Serial.println("h");
        }
        if (data[0] == DELLchipID)
        {
          Serial.println F(("DELL adapter is connected, not specified chip")); 
          Serial.print F(("Id chip is: "));
          Serial.print(data[0],HEX);
          Serial.println F(("h"));
        }
        if (data[0] != DELLchipID && data[0] != DS2502DevID && data[0]!= DS2502DevID2)
        {
          Serial.println F(("The connected chip is not fully identifiable")); 
          Serial.print F(("Id chip is: "));
          Serial.print(data[0],HEX);
          Serial.println F(("h"));
        }
      }
      else
      {
        Serial.println F(("ROM CRC disagree")); 
        vybernakonec();
        return; 
      } 
      Serial.println F(("Reading Status"));        
      ds.reset(); 
      ds.skip();                      // Skip ROM search, address the device no matter what
      leemem[0] = READ_STATUS;        // command 
      leemem[1] = ProgLocation;       // low address byte
      leemem[2] = 0;                  // High address byte, always 0 for 2502
      ds.write(leemem[0],1);        // Read data command, leave ghost power on
      ds.write(leemem[1],1);        // LSB starting address, leave ghost power on
      ds.write(leemem[2],1);        // MSB starting address, leave ghost power on
      crc = ds.read();             // DS250x generates a CRC for the command we sent, we assign a read slot and store it's value
      crc_calc = OneWire::crc8(leemem, 3);  // We calculate the CRC of the commands we sent using the library function and store it
      if ( crc_calc != crc)        // Then we compare it to the value the ds250x calculated, if it fails, we print debug messages and abort
      {
        Serial.println F(("Disagree CRC"));
        Serial.print F(("Calculated CRC:"));
        Serial.println(crc_calc,HEX);    // HEX makes it easier to observe and compare
        Serial.print F(("DS250x calculated CRC:"));
        Serial.println(crc,HEX);
        vybernakonec();
        return;                     // Since CRC failed, we abort 
      }
      else
      {
        Serial.println F(("Status Command CRC agree")); 
      }
      Serial.println("Status Data: "); 
      for ( i = 0; i < 8; i++)     
      {
        data[i] = ds.read();        
        Serial.print(data[i],HEX);       
        Serial.print(" ");           
      }
      Serial.println(".");
      j= 1;
      for (i=0; i<4 ;i++ )
      {
        Serial.print("Page ");
        Serial.print(i);
        Serial.print(" protected status: ");
        Serial.println(data[0] && j); 
        j = j << 1;
      }
      vybernakonec();
      return;  
    }
    else                             // Nothing is connected in the bus
    {
      Serial.println F(("Connect the communication"));
      delay(1500);
      readrom(volba);
    }
  }
   //Serial.print F(("volba vstupu:"));
   //Serial.println (volba);
}


//Zapis do cipu ds2502
void writerom(uint8_t wattage){
  byte i;  
  const char  *progStrA = "";
  byte pwrctrlbit = 0;
  if (wattage == 240) { pwrctrlbit = 1; progStrA = progStr240;  }
  if (wattage == 180) { pwrctrlbit = 1; progStrA = progStr180;  }
  if (wattage == 130) { pwrctrlbit = 1; progStrA = progStr130; } 
  if (wattage == 90)  { pwrctrlbit = 1; progStrA = progStr90; } 
  if (wattage == 65)  { pwrctrlbit = 1; progStrA = progStr65; }
  if (wattage == 45)  { pwrctrlbit = 1; progStrA = progStr45; }
  if (wattage == 1)  { pwrctrlbit = 1; progStrA = progStr_key; }
  if (pwrctrlbit == 0) 
  { 
    Serial.println F(("Adapter power input incorrect"));    vybernakonec();
    return;  
  }
  Serial.println F(("Programming the option:"));   
  Serial.println (progStrA);     
  ds.reset(); 
  ds.skip();  
  leemem[0] = WRITE_MEMORY;       // command to initiate a write seq using array for later CRC calc
  leemem[1] = ProgLocation;       // low address byte
  leemem[2] = 0;                  // high address is always 0
  leemem[3] = progStrA[0];         // first byte of data
  ds.write_bytes(leemem,4,1);        // write command,address and data, leave ghost power on
  crc = ds.read();            // DS250x responds with crc over command and address and data
  crc_calc = OneWire::crc8(leemem, 4);
  if ( crc_calc != crc)   
  {
    Serial.println F(("CRC does not match the write command"));
    Serial.print F(("Calculated CRC:"));
    Serial.println(crc_calc,HEX);  
    Serial.print F(("DS250x calculated CRC:"));
    Serial.println(crc,HEX);
    vybernakonec();
    return; 
  }
  else
  {
    ProgPulse();
  }
  data[0] = ds.read();
  Serial.print F(("read back programmed data "));
  Serial.println(data[0],HEX);   
  // Serial.println (progStrA);
  //  Serial.println(strlen(progStrA)); 

  // loop for subsequent bytes -- CRC calculation is based on incremented LSB of address and data only now
  for (i = 1; i < strlen(progStrA); i++)
  // for (i = 1; i < 23; i++)
  {
    leemem[1]++;    // increment address for parallel crc calculation 
    leemem[2]=progStrA[i];  // next data byte
    ds.write(leemem[2],1);  // write data byte only
    crc = ds.read();            // DS250x responds with crc over command and address and data
    {
      ProgPulse();
    }
    data[0] = ds.read();
    Serial.print F(("read back programmed data "));
    Serial.println(data[0],HEX);       // printout in ASCII
  }
  Serial.println("Reading Memory");        
  ds.reset(); 
  ds.skip();                      // Skip ROM search, address the device no matter what
  leemem[0] = READ_MEMORY;        // command 
  leemem[1] = ProgLocation;       // low address byte
  leemem[2] = 0;                  // High address byte, always 0 for 2502
  ds.write(leemem[0],1);        // Read data command, leave ghost power on
  ds.write(leemem[1],1);        // LSB starting address, leave ghost power on
  ds.write(leemem[2],1);        // MSB starting address, leave ghost power on
  crc = ds.read();             // DS250x generates a CRC for the command we sent, we assign a read slot and store it's value
  crc_calc = OneWire::crc8(leemem, 3);  // We calculate the CRC of the commands we sent using the library function and store it
  if ( crc_calc != crc)        // Then we compare it to the value the ds250x calculated, if it fails, we print debug messages and abort
  {
    Serial.println("Invalid command CRC!");
    Serial.print("Calculated CRC:");
    Serial.println(crc_calc,HEX);    // HEX makes it easier to observe and compare
    Serial.print("DS250x readback CRC:");
    Serial.println(crc,HEX);
    vybernakonec();
    return;                     // Since CRC failed, we abort 
  }
        
  Serial.println("Data is: "); 
  for ( i = 0; i < 41; i++)     
  {
    data[i] = ds.read();        
    Serial.print(data[i],HEX);       
    Serial.print(" ");           
  }
  Serial.println();
  data[strlen(progStrA)]=0;
  Serial.println((char *) data);  // string
  vybernakonec();
  return; 
}
