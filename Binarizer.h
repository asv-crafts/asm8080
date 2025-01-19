

class Binarizer {

public:
	Binarizer();
	virtual bool putByte(unsigned char data);
	std::string getErrorMessage()
		{ return errorMessage; }
	virtual ~Binarizer();

protected:
	std::string errorMessage;
};

