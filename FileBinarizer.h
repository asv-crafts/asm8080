
class FileBinarizer : public Binarizer {

public:
	FileBinarizer(std::string target);
	void setOrigin(unsigned int address)
		{ originAddress = address; }
	void setBinarySize(unsigned int size)
		{ binarySize = size; }
	virtual bool putByte(unsigned char data);
	~FileBinarizer();

private:

	bool _putByte(unsigned char data);

	// file name for writing compiled data //
	std::string binaryFilename;

	// file descriptor for writing compiled data //
	std::FILE *binaryFile;

	// look definitions below for possible states //
	unsigned int binarizerState;

	unsigned int originAddress;
	unsigned int binarySize;
};

#define INITIAl_STATE    0
#define PROCESSING_STATE 1
#define FAILED_STATE     2
