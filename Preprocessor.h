
class Preprocessor {

public:
	Preprocessor(std::string primarySource, i8080asm *sourceCompiler,
			FileBinarizer *dataBinarizer);
	bool compile();
	std::string getErrorMessage() { return errorMessage; }
	~Preprocessor();

private:
	unsigned int passNumber;
	std::FILE *currentFile;
	std::string _primarySource;
	std::string errorMessage;
	i8080asm *compiler;
	FileBinarizer *binarizer;
	Source *currentSource;

	std::list<std::string> filesInProcessing;
	bool startPass();
	bool preprocessFile(std::string sourceFilename);
	bool preprocessLine(const char *line);
	void reportError(std::string message);
};

