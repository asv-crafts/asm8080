
class Source {

public:
	Source(std::string filename);
	bool getLine(char *lineHolder, unsigned int holderSize);
	bool endOfFile() { return eof; }
	bool close();
	std::string getErrorMessage() { return errorMessage; }
	std::string getFilename() { return sourceFilename; }
	unsigned int getLineNumber() { return lineNumber; }
	~Source();

private:
	std::FILE *sourceFile;
	bool eof;
	unsigned int lineNumber;
	std::string sourceFilename;
	std::string errorMessage;
};

