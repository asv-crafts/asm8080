
class StringBinarizer : public Binarizer {

public:
	StringBinarizer();
	virtual bool putByte(unsigned char data);
	std::string getContent()
		{ return content; }
	std::string getErrorMessage()
		{ return errorMessage; }
	virtual ~StringBinarizer();

protected:
	std::string errorMessage;
	std::string content;
};

