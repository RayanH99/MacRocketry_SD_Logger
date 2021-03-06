#include <MacRocketry_SD_Logger.h>  //header file
#include <SPI.h>     //SPI library
#include <SD.h>      //SD card library

MacRocketry_SD_Logger::MacRocketry_SD_Logger(void){ //constructor
  init();
  connectSD = SD.begin(SD_CS_Pin);      //initialize SD
  connectFile = openNextFile();         //open file
}

MacRocketry_SD_Logger::MacRocketry_SD_Logger(String filePath){ //constructor
  init();
  connectSD = SD.begin(SD_CS_Pin);      //initialize SD
  connectFile = openFile(filePath);         //open file

}

void MacRocketry_SD_Logger::init(void){ //initialize variables to null
  connectSD = false;
  connectFile = false;
  
  //variables for more efficient SD write
  bufferSize = 0;
}


//getters and setters --------------------------------------------------
uint16_t MacRocketry_SD_Logger::maxUInt(void){ return 0xffff; } //max 16-bit number
bool MacRocketry_SD_Logger::getConnectSD(void){ return connectSD; }
bool MacRocketry_SD_Logger::getConnectFile(void){ return connectFile; }

//file open function --------------------------------------------------
bool MacRocketry_SD_Logger::openNextFile(void){
  if (getConnectSD()){ //if SD card is connected
    
    uint16_t numNext = 0;
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
    int16_t bufferAllow = min(Write_Buffer - bufferSize, (int16_t)data.length());
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

