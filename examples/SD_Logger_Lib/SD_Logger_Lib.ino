//Start of Header ====================================================================================================
#ifndef MacRocketry_SD_Logger_h //include guard
#define MacRocketry_SD_Logger_h

#include <Arduino.h>  //Arduino library
#include <SPI.h>      //SPI library
#include <SD.h>       //SD card library

#define SD_CS_Pin 10
#define fileNamePrefix "file_"
#define SD_Buffer 512
#define Write_Buffer (SD_Buffer-16)

class MacRocketry_SD_Logger {
  public:
    MacRocketry_SD_Logger(void);              //default constructor
    MacRocketry_SD_Logger(String filePath);   //filePath
    
    bool openFile(String filePath);    //open file
    bool writeFile(String data);      //write to file
    bool writeBuffer(String data);    //faster write
    uint16_t maxUInt(); //largest num representable by uint

    bool getConnectSD(void);
    bool getConnectFile(void);

  private:
    uint8_t connectState;
    File sdFile; //File object for SD

    //for incremental file name
    uint16_t numNext;
    
    int16_t bufferSize, bufferAllow;     //buffer up to 512 char

    void init();
    bool openNextFile();

};

#endif
//End of Header ======================================================================================================
//Start of Source ====================================================================================================
#include <Arduino.h>                //Arduino library
//#include <MacRocketry_SD_Logger.h> //header file
#include <SPI.h>     //SPI library
#include <SD.h>      //SD card library

MacRocketry_SD_Logger::MacRocketry_SD_Logger(void){ //constructor
  init();
  if (SD.begin(SD_CS_Pin))    //initialize SD
    connectState |= 0b01;
  if (openNextFile())         //open file
    connectState |= 0b10;
}

MacRocketry_SD_Logger::MacRocketry_SD_Logger(String filePath){ //constructor
  init();
  if (SD.begin(SD_CS_Pin))    //initialize SD
    connectState |= 0b01;
  if (openNextFile())         //open file
    connectState |= 0b10;

  //variables for incremental file
  numNext = 0;

}

void MacRocketry_SD_Logger::init(void){ //initialize variables to null
  connectState = 0b00;
  
  //variables for more efficient SD write
  bufferSize = 0;
  bufferAllow = 0;
}


//getters and setters --------------------------------------------------
uint16_t MacRocketry_SD_Logger::maxUInt(void){ return 0xffff; } //max 16-bit number
bool MacRocketry_SD_Logger::getConnectSD(void){ return (connectState & 0b01); }
bool MacRocketry_SD_Logger::getConnectFile(void){ return (connectState & 0b10); }

//file open function --------------------------------------------------
bool MacRocketry_SD_Logger::openNextFile(void){
  if (getConnectSD()){ //if SD card is connected
    
    while ( //if file already exist and numNext have not reach max value
      (SD.exists(String(fileNamePrefix) + String(numNext))) && 
      (numNext < maxUInt()))
    {
      numNext++;
    }
    //if file num does not exceed maxUInt()
    if (numNext < maxUInt()) return openFile(String(fileNamePrefix) + String(numNext));
  }
  return false; //no SD card
}


bool MacRocketry_SD_Logger::openFile(String filePath){
  if (getConnectSD()){
    if (sdFile){        //if current file is open
      sdFile.close();   //close current file
      delay(5);         //delay 5ms
    }
    sdFile = SD.open(filePath, FILE_WRITE);   //open new file for write
    if (sdFile){                              //if successful
      writeFile("start logging...");
      return true;                            //open file successfully
    }
  }
  return false; //cannot open file
}


//file close function --------------------------------------------------
bool MacRocketry_SD_Logger::writeFile(String data){
  if (getConnectFile()){    //if file is open
    sdFile.println(data);
    sdFile.flush();         //flush all bytes to SD card
    return true;
  }
  return false; //cannot write to file
}


bool MacRocketry_SD_Logger::writeBuffer(String data){
  if (getConnectFile()){  //if file is open

    //calculate available space in buffer
    //String.length() returns length in unsigned int
    bufferAllow = min(Write_Buffer - bufferSize, (int16_t)data.length());
    bufferSize += bufferAllow; //update current buffer size

    //write string with allowable space
    sdFile.print(data.substring(0, bufferAllow));
    if (bufferSize >= Write_Buffer){ //if buffer is full
      sdFile.print("\nbuffered\n");
      sdFile.flush(); //actually record to SD
      bufferSize = 0; //reset buffer

      //write the rest of the data
      sdFile.print(data.substring(bufferAllow));
    }
    return true;
  }
  return false;
}


//End of Source ======================================================================================================
//Start of Main ======================================================================================================
MacRocketry_SD_Logger sd;

void setup() {
}

void loop() {
}
