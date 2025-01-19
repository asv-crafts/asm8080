
extern bool isAlpha(char character);
extern bool isDigit(char character);
extern bool isHexDigit(char character);
extern bool endOfLine(char character);
extern const char *overWord(const char *inputLine);
extern const char *overSpaces(const char *inputLine);
extern unsigned int getDigitValue(char digit);
extern bool recognizeLexem(const char **lexems, const char *mPos,
			const char *tPos, unsigned int *lexemCode);
int parseString(const char *data, const char **sePos, bool parseOnly,
	Binarizer *binarizer, unsigned int *stringSize);
const char *getErrorDescription(unsigned int errorCode);
