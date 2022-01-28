
class Loader
{
   private:
      bool loaded;        //set to true if a file is successfully loaded into memory
      std::ifstream inf;  //input file handle
   public:
      Loader(int argc, char * argv[]);
      bool isLoaded();
      bool checkFile(char*& fileName);
      void readFile(char*& fileName);
      int32_t convert(std::string &line, int begin, int end);
      bool checkFileEndLine(char*& fileName);
      void loadLine(std::string &currentLine);
      bool hasErrors(std::string & currentLine);
      bool onlyComment(std::string &linetoParse);
      bool onlyAddressComment(std::string & linetoParse);
      bool addressError(std::string & linetoParse);
      bool dataError(std::string &linetoParse);
      bool isBlankLine(std::string &linetoParse);
      bool colonError(std::string & linetoParse);
      bool pipeError(std::string & linetoParse); 
};
