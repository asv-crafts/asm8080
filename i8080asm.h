
#define ARG_NONE     0
#define ARG_REG      1
#define ARG_REGPAIR  2
#define ARG_BYTE     3
#define ARG_WORD     4
#define ARG_REGPAIR2 5

struct CompiledData
{
	unsigned int dataLength;
	unsigned char dataBuffer[256];
};

struct MnemonicDescription
{
	// number of arguments, required for mnemonic //
	unsigned int argNumber;

	// zero means length may vary for this mnemonic //
	unsigned int instructionLength;

	unsigned int exactCodeLength;
	unsigned char exactCode;
	unsigned char argTypes[2];
};

class i8080asm
{
public:
	i8080asm(FileBinarizer *binarizer);
	void setPassNumber(unsigned int pass);
	void setAddress(unsigned int newAddress);
	bool compileLine(const char *sourceLine);
	bool dumpDebugInfo(const char *hintsFilename);
	bool isCompleted() { return compileCompleted; }
	unsigned int getOrigin() { return originAddress; }
	unsigned int getBinarySize() { return binarySize; }
	std::string getErrorMessage() const
		{ return errorMessage; }
	~i8080asm();

private:
	bool dispatchMnemonic(unsigned int mnemoCode, const char **args);
	bool compileExactMnemonic(unsigned int mnemoCode, const char *args);
	bool compileExact3Mnemonic(const MnemonicDescription *mnemonicDescription,
			unsigned int firstArg);
	bool compileExact2Mnemonic(const MnemonicDescription *mnemonicDescription,
			unsigned int firstArg);
	bool compileExactRMnemonic(const MnemonicDescription *mnemonicDescription,
			unsigned int firstArg);

	void formErrorMessage(const char *sourceLine);

	// check, that word between cPos and wePos is a label declaration //
	// cPos is a pointer to a first character of the word //
	// wePos is a pointer to a character that comes after word last character //
	bool recognizeLabel(const char *cPos, const char *wePos);

	// if label is recognized, store label name and its value to //
	// storage of labels                                         //
	bool defineLabel(const char *cPos, const char **wePos,
		unsigned int currentAddress);

	// mPos is a mnemonic first char //
	// tPos is a character, that comes after last mnemonic character //
	bool recognizeMnemonic(const char *mPos, const char *tPos,
			unsigned int *mnemoCode);

	bool recognizeregisterPairId(const char *rpPos,
		const char *rpePos, const char *regSamples[],
		unsigned int *regIdx);

	bool recognizeRegister(const char *rPos, const char *rePos,
			unsigned int *regIdx);
	bool recognizeRegistersPair(const char *rpPos, const char *rpePos,
			unsigned int *regIdx);
	bool recognizeRegistersPair2(const char *rpPos, const char *rpePos,
			unsigned int *regIdx);

	// return value of a number or a label address //
	bool getOperandValue(const char *cPos, const char **sePos,
		unsigned int *result);

	// calculate high-prioiry operators sequence //
	bool getHiExpressionValue(const char *expPos, const char **expEnd,
			unsigned int *result);

	// calculate low-priority operators equence //
	// set pts to true, if expression is in parentheses //
	bool getLowExpressionValue(const char *expPos, const char **expEnd,
			unsigned int *expResult, bool pts);

	bool getExpressionValue(const char *expPos, const char **expEnd,
			unsigned int *expResult);

	bool getLabelAddress(const char *labelPos, const char **labelEnd,
		unsigned int *address);

	bool getNumber(const char *numPos, const char **numEnd,
			unsigned int *numValue);

	bool getMnemonicArgument(char argType, const char *argPos,
					const char **cPos, unsigned int *argVal);
					
	bool compileLxi(unsigned int regPair, unsigned int imData);

	bool compileStax(unsigned int regIdx);

	bool compileInx(unsigned int regIdx);

	bool compileInr(unsigned int regIdx);

	bool compileDcr(unsigned int regIdx);

	bool compileMvi(unsigned int regIdx, unsigned int imData);

	bool compileDad(unsigned int regIdx);

	bool compileLdax(unsigned int regIdx);

	bool compileDcx(unsigned int regIdx);

	bool compileMov(unsigned int regIdx, unsigned int regIdx2);

	bool compilePop(unsigned int regIdx);

	bool compilePush(unsigned int regIdx);

	bool compileRst(unsigned int intIdx);
	
	bool processOrg(unsigned int orgAddr);

	bool compileDb(const char *data, const char **newPos);

	bool compileDw(const char *data, const char **newPos);

	bool compileDs(const char *data, const char **newPos);

	bool compileDbString(const char *data, const char **sePos);
	
	// first pass is defining labels and overall binary size //
	// second pass is compiling source to binary             //
	unsigned int passNumber;

	// the following two variables are valid on second pass only //
	unsigned int originAddress;
	unsigned int binarySize;
	unsigned int currentAddress;
	std::string errorMessage;
	const char *errorPosition;
	DebugInfo debugInfo;
	FileBinarizer *_binarizer;
	bool originDefined;
	bool compileCompleted;

	// whether new line that contains only comment   //
	// can be appended to comment from previous line //
	bool ableAppendComment;
	unsigned int lastCommentAddr;
	bool appendCommentLeft;
};

#define NOP   0
#define LXI   1
#define STAX  2
#define INX   3
#define INR   4
#define DCR   5
#define MVI   6
#define DAD   8
#define LDAX  9
#define DCX   10
#define MOV   22
#define POP   33
#define PUSH  37
#define RST   39
#define ORG   78
#define DB    79
#define DS    80
#define DW    81
#define EQU   82
#define END   83
