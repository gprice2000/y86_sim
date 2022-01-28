/**
 * Names:Gabe Price, Kevin Kamto
 * Team: Pricekamto
*/
#include <iostream>
#include <fstream>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <queue>
#include "Loader.h"
#include "Memory.h"
//first column in file is assumed to be 0
#define ADDRBEGIN 2   //starting column of 3 digit hex address 
#define ADDREND 4     //ending column of 3 digit hex address
#define DATABEGIN 7   //starting column of data bytes
#define COMMENT 28    //location of the '|' character 

     Memory * memInstance = NULL; 
     std::queue<int32_t> addresses; 
     int32_t last_address_written;
     std::string error_statement = "";

/**
 * Loader constructor
 * Opens the .yo file named in the command line arguments, reads the contents of the file
 * line by line and loads the program into memory.  If no file is given or the file doesn't
 * exist or the file doesn't end with a .yo extension or the .yo file contains errors then
 * loaded is set to false.  Otherwise loaded is set to true. And Dr. Wilkes was here.
 *
 * @param argc is the number of command line arguments passed to the main; should
 *        be 2
 * @param argv[0] is the name of the executable
 *        argv[1] is the name of the .yo file
 */
Loader::Loader(int argc, char * argv[])
{
   loaded = false;
   //Start by writing a method that opens the file (checks whether it ends 
   //with a .yo and whether the file successfully opens; if not, return without 
   //loading)
   addresses.push(0);

   if (argv[1] == NULL){

      printf("no file provided\n");
      return;

   }   

   bool isValid = checkFile(argv[1]);
   
   if(isValid){

      memInstance= Memory::getInstance();
      readFile(argv[1]);

   }

   else{
      return;
   }
   //The file handle is declared in Loader.h.  You should use that and
   //not declare another one in this file.
      // Use std::ifstream inf;
   //Next write a simple loop that reads the file line by line and prints it out
   
   //Next, add a method that will write the data in the line to memory 
   //(call that from within your loop)

   //Finally, add code to check for errors in the input line.
   //When your code finds an error, you need to print an error message and return.
   //Since your output has to be identical to your instructor's, use this cout to print the
   //error message.  Change the variable names if you use different ones.
   //  std::cout << "Error on line " << std::dec << lineNumber
   //       << ": " << line << std::endl;


   //If control reaches here then no error was found and the program
   //was loaded into memory.
}

/**
 * isLoaded
 * returns the value of the loaded data member; loaded is set by the constructor
 *
 * @return value of loaded (true or false)
 */
bool Loader::isLoaded()
{
   return loaded;
}

/* checkFile
*  accepts a name of a file and returns true if file is valid (
*  exists, ends with .yo, and can be opend)
*
* @param fileName is equal to the name of file, argv[0]
*
* @return true if file is valid and can be opened
* false otherwise
*/
bool Loader::checkFile(char*& fileName)
{

   inf.open(fileName); 

   if(inf.is_open() != true){

       printf("file doesn't exist\n");
   
   }
   if(inf.is_open() && checkFileEndLine(fileName))
   {
      inf.close();
      return true;
   }
   
   return false; 
}

/*
* readFile
* accepts the name of the file and iterates through line by line
* checking if line has errors, in which it returns
* if no errors found, load data into memory
*
* @param fileName is equal to the name of the file to be opened
*
* @return (break from file reading) if errors are found on current line
*/
void Loader::readFile(char*& fileName)
{
   inf.open(fileName);
   std::string currentLine;
   int line_number = 1;
   while (!inf.eof())
   {
      // store current line into a string 
      std::getline(inf, currentLine);

    if(hasErrors(currentLine)){
         std::cout << "Error on line " << std::dec << line_number
             << ": " << currentLine  << std::endl;
             
            //  std:: cout<< "Load error." << std::endl;
            // std:: cout << "Usage: yess <file.yo>" << std::endl ;

            // printf("\n");
      return;
      }
      else{
         //stores current line data in memory
         if(currentLine[0] == '0' && currentLine[DATABEGIN] != ' '){

             loadLine(currentLine);
         }
      }
   
      line_number++;

   }
loaded = true;   
}

/*
* hasErrors
* accepts the current line as a string and calls multiple 
* error-checking methods
* returns false if error free, true otherwise
*
* @param currentLine is the currentline to be read 
*
* @return false if no errors are found or if line is 
* in a given format. True if errors are found
*/
bool Loader::hasErrors(std::string & currentLine)
{
   //check if line only contains a comment 
   if(onlyComment(currentLine)){
      return false;
   }
   //checks if line is in form of an address and a comment
   if(onlyAddressComment(currentLine)){
      return false;
   }

   //checks if current line is blank
   if(isBlankLine(currentLine)){
      return false;
   }
   //call various methods to check for specific errors
   if(addressError(currentLine) || dataError(currentLine) || 
     colonError(currentLine) || pipeError(currentLine)){
        return true;
    }

   return false;
}

/*
* isBlankLine
* accepts the current line as a string
* returns true if line is blank, false otherwise
* 
* @param linetoParse is the current line
*
* @return false if line is not blank, true otherwise
*/
bool Loader::isBlankLine(std::string & linetoParse){
   std::string val ="";

   for(int i = 0; i < linetoParse.length(); i++){

      if(linetoParse[i] != ' '){
         return false;
      }

   }

   return true;
}

/*
* onlyAddressComment
* accepts the current line as a string
* returns true if line contains only an address and comment 
* 
* @param linetoParse is the current line
*
* @return false if line is not in address-comment format, true otherwise
*/
bool Loader::onlyAddressComment(std::string &linetoParse){

   for(int i = ADDREND + 2; i < COMMENT; i++){

      if(linetoParse[i] != ' '){
         return false;
      }

   }

   if((addressError(linetoParse) || pipeError(linetoParse) || colonError(linetoParse))){
      return false;
   }

   return true; 
}

/*
* onlyComment
* accepts the current line as a string
* returns true if line contains only a comment 
* 
* @param linetoParse is the current line
*
* @return true if line only contains a comment, false otherwise 
*/
bool Loader::onlyComment(std::string &linetoParse){

  for(int i = 0; i < COMMENT; i++){

     if(linetoParse[i] != ' '){
        return false;
     }

  } 

  if(pipeError(linetoParse)){
     return false;
  }

  return true;
}

/*
* addressError 
* accepts the current line as a string
* returns true if address contains an error 
* 
* @param linetoParse is the current line
*
* @return true if address gives an error, false otherwise 
*/
bool Loader::addressError(std::string & linetoParse)
{
   
   std::string addr_str = "";

   for(int i = 0; i<= ADDREND; i++ ){
      addr_str.push_back(linetoParse[i]);
   }

   if (addr_str[0] != '0' || addr_str[1] != 'x' || addr_str.length() < 5){
      return true;
   }

   int32_t address = convert(linetoParse, ADDRBEGIN, ADDREND);

   if (address < last_address_written){
      return true;
   }

   if(addresses.back() > address){
      return true;
   } 

   addresses.push(address); 

   if (address < 0 || address >= 0x1000){
      return true;
   } 
   
 
  return false;
}

/*
* dataError
* accepts the current line as a string
* returns true if data contains an error 
* 
* @param linetoParse is the current line
*
* @return true if data gives an error, false otherwise 
*/
bool Loader::dataError(std::string & linetoParse)
{
   std::string data_str = "";
   std::string cur_char;
   int i;
   int num_bytes = 0;

   for(i = DATABEGIN; linetoParse[i] != ' '; i++){

      if(!isxdigit(linetoParse[i])){
         return true;
      }
      cur_char = "";
      cur_char.push_back(linetoParse[i]);
     
      num_bytes ++;
      data_str.push_back(linetoParse[i]);
   }

   for(int j = i + 1; j < COMMENT; j++){

      if(linetoParse[j] != ' '){
         return true;
      }
   }

   if(num_bytes % 2 != 0){
      return true;
   }

   num_bytes/=2;
   
   last_address_written = addresses.back() + num_bytes;

   if (data_str.length() < 2 || data_str.length() > 20 || (last_address_written - 1) > 0xfff){
      return true;
   }

   return false;
   
}


/*
* pipeError 
* accepts the current line as a string
* returns true if | is not at correct location 
* 
* @param linetoParse is the current line
*
* @return true if | is not at correct location, false otherwise 
*/
bool Loader::pipeError(std::string & linetoParse)
{
  
  if (linetoParse[COMMENT]!= '|'){
     return true;
  }
  
  return false;
}

/*
* colonError 
* accepts the current line as a string
* returns true if : is not at correct location 
* 
* @param linetoParse is the current line
*
* @return true if : is not at correct location, false otherwise 
*/
bool Loader::colonError(std::string & linetoParse)
{
   
   if(linetoParse[ADDREND + 1] != ':'){
      return true;
   }

   if(linetoParse[ADDREND + 2] != ' '){
      return true;
   } 

   return false;
}

/*
* convert 
* accepts the current line as a string and a start and end position
* returns a int32_t value converted from string created from begin to end 
* 
* @param line is the current line
* @param begin is the starting position of string to convert
* @param end is the ending position of string to convert
*
* @return addr, the address or any value to convert into a int32_t type 
*/
int32_t Loader::convert(std::string &line, int begin, int end){
   std::string address = "";

      for(int i = begin;i <= end;i++ )
      {
         address.push_back(line[i]);
      }     
 
 int32_t addr = std::stoul(address, NULL, 16);
 return addr;
}

/*
* checkFileEndLine 
* accepts the name of file to read
* returns true if file ends with .yo extension, false otherwise 
* 
* @param fileName is the name of the file 
*
* @return returnValue set to true if .yo is found in fileName
* false otherwise 
*/
bool Loader::checkFileEndLine(char*& fileName)
{
   const char *lookingFor = ".yo"; 
   char * resolve = strstr(fileName, lookingFor);
   bool returnValue = false;

   if (resolve != NULL)
   {
      returnValue = true;
   }else{
      printf("File doesn't end with a .yo extension\n");
   }
   
   return returnValue;
}


/*
* loadLine 
* accepts current line of file 
* loads values into specified address at beginning of line
* 
* @param currentLine is the current line of file to get data from 
* 
*/
void Loader::loadLine(std::string &currentLine)
{
  
    bool error;
     //convert starting address to int32_t 
      int32_t addr = convert(currentLine, ADDRBEGIN, ADDREND);
      
      int byte_counter = 0;

      //declare a queue used to store the values of each byte in data
      int num_bytes = 0;
      std::queue<int32_t> byte_q;
      std::string value; 
      
      while(currentLine[DATABEGIN + byte_counter] != ' '){
         value = "";
         //adds each byte into a new string of length 2
        for(int i = 0 ; i < 2 ; i++){
           value.push_back(currentLine[DATABEGIN + i + byte_counter]);
         } 

           int32_t val1 = stoul(value,NULL,16);
         
            //store converted byte into queue
           byte_q.push(val1);
       
           byte_counter += 2; 
          
          num_bytes++;
           

      }
      //add each byte into memory
      for(int i = 0; i <= num_bytes-1; i++){

           memInstance->putByte((uint8_t)byte_q.front(),addr+i,error); 
          
           byte_q.pop(); 
      }
      



}