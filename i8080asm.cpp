
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <string>
#include <map>
#include "Binarizer.h"
#include "StringBinarizer.h"
#include "FileBinarizer.h"
#include "DebugInfo.h"
#include "common.h"
#include "i8080asm.h"

using std::string;
using std::map;

const static char *mnemonics[] = { "NOP", "LXI", "STAX", "INX", "INR",
		"DCR", "MVI", "RLC", "DAD", "LDAX", "DCX", "RRC", "RAL", "RAR",
		"SHLD", "DAA", "LHLD", "CMA", "STA", "STC", "LDA", "CMC", "MOV",
		"HLT", "ADD", "ADC", "SUB", "SBB", "ANA", "XRA", "ORA", "CMP",
		"RNZ", "POP", "JNZ", "JMP", "CNZ", "PUSH", "ADI", "RST", "RZ",
		"RET", "JZ", "CZ", "CALL", "ACI", "RNC", "JNC", "OUT", "CNC",
		"SUI", "RC", "JC", "IN", "CC", "SBI", "RPO", "JPO", "XTHL", "CPO",
		"ANI", "RPE", "PCHL", "JPE", "XCHG", "CPE", "XRI", "RP",
		"JP", "DI", "CP", "ORI", "RM", "SPHL", "JM", "EI", "CM", "CPI",
		"ORG", "DB", "DS", "DW", "EQU", "END", "" };

const static MnemonicDescription codeMap[] = {
		// args, len, exlen, excode,        argtypes
		{     0,   1,     1, 0x00, { ARG_NONE, ARG_NONE }},     // 0. NOP //
		{     2,   3,     0, 0x00, { ARG_REGPAIR, ARG_WORD }},  // 01. LXI //
		{     1,   1,     0, 0x00, { ARG_REGPAIR, ARG_NONE }},  // 02. STAX //
		{     1,   1,     0, 0x00, { ARG_REGPAIR, ARG_NONE }},  // 03. INX  //
		{     1,   1,     0, 0x00, { ARG_REG, ARG_NONE }},      // 04. INR  //
		{     1,   1,     0, 0x00, { ARG_REG, ARG_NONE }},      // 05. DCR  //
		{     2,   2,     0, 0x00, { ARG_REG, ARG_BYTE }},      // 06. MVI  //
		{     0,   1,     1, 0x07, { ARG_NONE, ARG_NONE }},     // 07. RLC  //
		{     1,   1,     0, 0x00, { ARG_REGPAIR, ARG_NONE }},  // 08. DAD  //
		{     1,   1,     0, 0x00, { ARG_REGPAIR, ARG_NONE }},  // 09. LDAX //
		{     1,   1,     0, 0x00, { ARG_REGPAIR, ARG_NONE }},  // 10. DCX  //
		{     0,   1,     1, 0x0f, { ARG_NONE, ARG_NONE }},     // 11. RRC  //
		{     0,   1,     1, 0x17, { ARG_NONE, ARG_NONE }},     // 12. RAL  //
		{     0,   1,     1, 0x1F, { ARG_NONE, ARG_NONE }},     // 13. RAR  //
		{     1,   3,     1, 0x22, { ARG_WORD, ARG_NONE }},     // 14. SHLD //
		{     0,   1,     1, 0x27, { ARG_NONE, ARG_NONE }},     // 15. DAA  //
		{     1,   3,     1, 0x2a, { ARG_WORD, ARG_NONE }},     // 16. LHLD //
		{     0,   1,     1, 0x2f, { ARG_NONE, ARG_NONE }},     // 17. CMA  //
		{     1,   3,     1, 0x32, { ARG_WORD, ARG_NONE }},     // 18. STA  //
		{     0,   1,     1, 0x37, { ARG_NONE, ARG_NONE }},     // 19. STC  //
		{     1,   3,     1, 0x3a, { ARG_WORD, ARG_NONE }},     // 20. LDA  //
		{     0,   1,     1, 0x3f, { ARG_NONE, ARG_NONE }},     // 21. CMC  //
		{     2,   1,     0, 0x00, { ARG_REG, ARG_REG }},       // 22. MOV  //
		{     0,   1,     1, 0x76, { ARG_NONE, ARG_NONE }},     // 23. HLT  //
		{     1,   1,     1, 0x80, { ARG_REG, ARG_NONE }},      // 24. ADD  //
		{     1,   1,     1, 0x88, { ARG_REG, ARG_NONE }},      // 25. ADC  //
		{     1,   1,     1, 0x90, { ARG_REG, ARG_NONE }},      // 26. SUB  //
		{     1,   1,     1, 0x98, { ARG_REG, ARG_NONE }},      // 27. SBB  //
		{     1,   1,     1, 0xa0, { ARG_REG, ARG_NONE }},      // 28. ANA  //
		{     1,   1,     1, 0xa8, { ARG_REG, ARG_NONE }},      // 29. XRA  //
		{     1,   1,     1, 0xb0, { ARG_REG, ARG_NONE }},      // 30. ORA  //
		{     1,   1,     1, 0xb8, { ARG_REG, ARG_NONE }},      // 31. CMP  //
		{     0,   1,     1, 0xc0, { ARG_NONE, ARG_NONE }},     // 32. RNZ  //
		{     1,   1,     0, 0x00, { ARG_REGPAIR2, ARG_NONE }}, // 33. POP  //
		{     1,   3,     1, 0xc2, { ARG_WORD, ARG_NONE }},     // 34. JNZ  //
		{     1,   3,     1, 0xc3, { ARG_WORD, ARG_NONE }},     // 35. JMP  //
		{     1,   3,     1, 0xc4, { ARG_WORD, ARG_NONE }},     // 36. CNZ  //
		{     1,   1,     0, 0x00, { ARG_REGPAIR2, ARG_NONE }}, // 37. PUSH //
		{     1,   2,     1, 0xc6, { ARG_BYTE, ARG_NONE }},     // 38. ADI  //
		{     1,   1,     0, 0x00, { ARG_BYTE, ARG_NONE }},     // 39. RST  //
		{     0,   1,     1, 0xc8, { ARG_NONE, ARG_NONE }},     // 40. RZ   //
		{     0,   1,     1, 0xc9, { ARG_NONE, ARG_NONE }},     // 41. RET  //
		{     1,   3,     1, 0xca, { ARG_WORD, ARG_NONE }},     // 42. JZ   //
		{     1,   3,     1, 0xcc, { ARG_WORD, ARG_NONE }},     // 43. CZ   //
		{     1,   3,     1, 0xcd, { ARG_WORD, ARG_NONE }},     // 44. CALL //
		{     1,   2,     1, 0xce, { ARG_BYTE, ARG_NONE }},     // 45. ACI  //
		{     0,   1,     1, 0xd0, { ARG_NONE, ARG_NONE }},     // 46. RNC  //
		{     1,   3,     1, 0xd2, { ARG_WORD, ARG_NONE }},     // 47. JNC  //
		{     1,   2,     1, 0xd3, { ARG_BYTE, ARG_NONE }},     // 48. OUT  //
		{     1,   3,     1, 0xd4, { ARG_WORD, ARG_NONE }},     // 49. CNC  //
		{     1,   2,     1, 0xd6, { ARG_BYTE, ARG_NONE }},     // 50. SUI  //
		{     0,   1,     1, 0xd8, { ARG_NONE, ARG_NONE }},     // 51. RC   //
		{     1,   3,     1, 0xda, { ARG_WORD, ARG_NONE }},     // 52. JC   //
		{     1,   2,     1, 0xdb, { ARG_BYTE, ARG_NONE }},     // 53. IN   //
		{     1,   3,     1, 0xdc, { ARG_WORD, ARG_NONE }},     // 54. CC   //
		{     1,   2,     1, 0xde, { ARG_BYTE, ARG_NONE }},     // 55. SBI  //
		{     0,   1,     1, 0xe0, { ARG_NONE, ARG_NONE }},     // 56. RPO  //
		{     1,   3,     1, 0xe2, { ARG_WORD, ARG_NONE }},     // 57. JPO  //
		{     0,   1,     1, 0xe3, { ARG_NONE, ARG_NONE }},     // 58. XTHL //
		{     1,   3,     1, 0xe4, { ARG_WORD, ARG_NONE }},     // 59. CPO  //
		{     1,   2,     1, 0xe6, { ARG_BYTE, ARG_NONE }},     // 60. ANI  //
		{     0,   1,     1, 0xe8, { ARG_NONE, ARG_NONE }},     // 61. RPE  //
		{     0,   1,     1, 0xe9, { ARG_NONE, ARG_NONE }},     // 62. PCHL //
		{     1,   3,     1, 0xea, { ARG_WORD, ARG_NONE }},     // 63. JPE  //
		{     0,   1,     1, 0xeb, { ARG_NONE, ARG_NONE }},     // 64. XCHG //
		{     1,   3,     1, 0xec, { ARG_WORD, ARG_NONE }},     // 65. CPE  //
		{     1,   2,     1, 0xee, { ARG_BYTE, ARG_NONE }},     // 66. XRI  //
		{     0,   1,     1, 0xf0, { ARG_NONE, ARG_NONE }},     // 67. RP   //
		{     1,   3,     1, 0xf2, { ARG_WORD, ARG_NONE }},     // 68. JP   //
		{     0,   1,     1, 0xf3, { ARG_NONE, ARG_NONE }},     // 69. DI   //
		{     1,   3,     1, 0xf4, { ARG_WORD, ARG_NONE }},     // 70. CP   //
		{     1,   2,     1, 0xf6, { ARG_BYTE, ARG_NONE }},     // 71. ORI  //
		{     0,   1,     1, 0xf8, { ARG_NONE, ARG_NONE }},     // 72. RM   //
		{     0,   1,     1, 0xf9, { ARG_NONE, ARG_NONE }},     // 73. SPHL //
		{     1,   3,     1, 0xfa, { ARG_WORD, ARG_NONE }},     // 74. JM   //
		{     0,   1,     1, 0xfb, { ARG_NONE, ARG_NONE }},     // 75. EI   //
		{     1,   3,     1, 0xfc, { ARG_WORD, ARG_NONE }},     // 76. CM   //
		{     1,   2,     1, 0xfe, { ARG_BYTE, ARG_NONE }},     // 77. CPI  //
		{     1,   0,     0, 0x00, { ARG_WORD, ARG_NONE }},     // 78. ORG  //
		{     0,   0,     0, 0x00, { ARG_NONE, ARG_NONE }},     // 79. DB   //
		{     0,   0,     0, 0x00, { ARG_NONE, ARG_NONE }},     // 80. DS   //
		{     0,   0,     0, 0x00, { ARG_NONE, ARG_NONE }},     // 81. DW   //
		{     0,   0,     0, 0x00, { ARG_WORD, ARG_NONE }},     // 82. EQU  //
		{     0,   0,     0, 0x00, { ARG_WORD, ARG_NONE }}      // 83. END  //
};

const static char *regPairs[] = { "BC", "DE", "HL", "SP" , "B", "D", "H", ""};
const static char *regPairs2[] = { "BC", "DE", "HL", "PSW" , "B", "D", "H", ""};
const static char regs[] = { 'B', 'C', 'D', 'E', 'H', 'L', 'M', 'A'};

i8080asm::i8080asm(FileBinarizer *binarizer)
{
	originDefined = false;
	originAddress = 0;
	currentAddress = 0;
	binarySize = 0;
	_binarizer = binarizer;
	compileCompleted = false;
}

void i8080asm::setPassNumber(unsigned int pass)
{
	passNumber = pass;
	currentAddress = 0;
	originDefined = false;
	compileCompleted = false;
}

bool i8080asm::compileLine(const char *sourceLine)
{
	const char *cPos;

	// word end position //
	const char *wePos;

	unsigned int lineAddress = currentAddress;
	unsigned int mnemoCode;

	cPos = overSpaces(sourceLine);

	if ('\0' == *cPos)
		return true;

	if (';' == *cPos) {
		if (1 == passNumber)
			// at first pass we cannot determine address //
			// to which assign this comment in debugInfo //
			return true;

		if (! originDefined)
			// we cannot assign comment to undefined address //
			return true;

		// remember position where comment begins //
		const char *commentPos = cPos;

		string commentText;
		while (('\0' != *cPos) && (13 != *cPos) && (10 != *cPos)) {
			commentText += *cPos;
			cPos++;
		}

		// all comments that not begin from first column //
		// can be appended to previous comment           //
		if ((commentPos != sourceLine) && (ableAppendComment)) {
			// this comment can be appended to comment from previous line //
			if (appendCommentLeft) {
				debugInfo.addLeftComment(commentText, lastCommentAddr);
			} else {
				debugInfo.addRightComment(commentText, lastCommentAddr);
			}
		} else {
			debugInfo.addLeftComment(commentText, lineAddress);
			lastCommentAddr = lineAddress;
			ableAppendComment = true;
			appendCommentLeft = true;
		}
		return true;
	}

	ableAppendComment = false;

	// line cannot start with a digit //
	if (isDigit(*cPos)) {
		errorPosition = cPos;
		errorMessage = "Syntax error";
		formErrorMessage(sourceLine);
		return false;
	}

	wePos = overWord(cPos);

	// check, if line starts with a label //
	if (recognizeLabel(cPos, wePos)) {


		if (false == defineLabel(cPos, &wePos, currentAddress)) {
			formErrorMessage(sourceLine);
			return false;
		}
		// stay to character after mnemonic declaration //
		cPos = wePos;

		// after declaration of label via EQU directive //
		// there is always end-of-line                  //
		if (endOfLine(*cPos))
			return true;

		// walk over colon //
		cPos++;

		cPos = overSpaces(cPos);

		if (endOfLine(*cPos))
			return true;

		wePos = overWord(cPos);
	}


	// check, if there is a mnemonic at current character position //
	if (recognizeMnemonic(cPos, wePos, &mnemoCode)) {

		bool status;

		if (EQU == mnemoCode) {
			// bad EQU usage - no label name specified //
			errorMessage = "No label name specified for EQU directive";
			errorPosition = cPos;
			formErrorMessage(sourceLine);
			return false;
		}


		// walk to arguments of the mnemonic //
		cPos = overSpaces(wePos);
		status = dispatchMnemonic(mnemoCode, &cPos);
		if (! status) {
			formErrorMessage(sourceLine);
			return false;
		}
	}

	cPos = overSpaces(cPos);

	if (! endOfLine(*cPos)) {
		errorMessage = "Syntax error";
		errorPosition = cPos;
		formErrorMessage(sourceLine);
		return false;
	}

	if (';' != *cPos)
		return true;

	if (1 == passNumber)
		return true;

	string commentText;
	while (('\0' != *cPos) && (13 != *cPos) && (10 != *cPos)) {
		commentText += *cPos;
		cPos++;
	}
	debugInfo.addRightComment(commentText, lineAddress);
	lastCommentAddr = lineAddress;
	ableAppendComment = true;
	appendCommentLeft = false;
	return true;
}

bool i8080asm::dumpDebugInfo(const char *hintsFilename)
{
	return debugInfo.storeInfo(hintsFilename);
}

void i8080asm::formErrorMessage(const char *sourceLine)
{
	// calculate number of charachers to error position //
	unsigned int positionNumber = 0;
	const char *cPos = sourceLine;

	// check that errorPosition points to character in sourceLine //
	while (!endOfLine(*cPos) && (cPos != errorPosition)) {
		positionNumber++;
		cPos++;
	}

	if (cPos == errorPosition) {
	
		// check for newline character at the end of sourceLine //
		while (('\n' != *cPos) && ('\0' != *cPos))
			cPos++;

		// add source line with highlighting to error message //
		string highLight;

		// if no newline character, add it ho hightLight string //
		if ('\0' == *cPos)
			highLight += '\n';

		for (unsigned int i = 0; i < positionNumber; i++) {
			// place tabs to highLight in same places //
			// as in source line                      //
			if ('\t' == sourceLine[i]) {
				highLight += '\t';
			} else {
				highLight += ' ';
			};
		}
		highLight += '^';
		highLight += '\n';

		errorMessage = string(sourceLine) + highLight +
				errorMessage;
	} else {
		errorMessage = string(sourceLine) + string("\n") + 
				errorMessage;
	}
}

bool i8080asm::recognizeLabel(const char *cPos, const char *wePos)
{
	const char *dPos;
	const char *dePos;
	unsigned int mnemoCode;

	if (cPos == wePos)
		return false;

	if (':' == *wePos)
		return true;

	if (endOfLine(*wePos))
		return false;

	// there is also may be an EQU directive that defines label //
	dPos = wePos;	

	dPos++;

	dPos = overSpaces(dPos);

	if (endOfLine(*dPos))
		return false;

	dePos = overWord(dPos);

	if (false == recognizeMnemonic(dPos, dePos, &mnemoCode))
		return false;

	if (EQU == mnemoCode)
		return true;

	return false;
}

bool i8080asm::defineLabel(const char *cPos, const char **wePos,
		unsigned int currentAddress)
{
	// lets make stupid limit on label identifier length //
	char labelName[32];
	unsigned int lnIdx = 0;
	const char *curChar = cPos;

	// which address assign to label //
	unsigned int labelValue;

	while ((curChar != *wePos) && (lnIdx < 32)) {

		labelName[lnIdx] = *curChar;
		lnIdx++;
		curChar++;
	}

	if (lnIdx > 31) {
		errorPosition = cPos;
		errorMessage = "Label identifier is too long";
		return false;
	}

	labelName[lnIdx] = 0;

	if (':' == *curChar) {
		// assign current address as value for semi-colon terminated labels //
		labelValue = currentAddress;
	} else {
		bool status;

		// end-of-argument position //
		const char *aePos;

		// this label defined via EQU directive (check the recongnizeLabel function) //
		// its value should be taken from argument of EQU directive                  //
		curChar = overSpaces(curChar);

		// walk over EQU directive //
		curChar = overWord(curChar);

		// walk to argument of EQU directive
		curChar = overSpaces(curChar);

		if (true == endOfLine(*curChar)) {
			errorMessage = "Missing argument for EQU directive";
			errorPosition = curChar;
			return false;
		}

		// get value for the label //
		status = getExpressionValue(curChar, &aePos, &labelValue);
		if (! status)
			return false;

		labelValue &= 0xffff;

		aePos = overSpaces(aePos);

		if (false == endOfLine(*aePos)) {
			errorMessage = "Exrta argument for EQU directive";
			errorPosition = aePos;
			return false;
		}

		*wePos = aePos;
	}

	// labels map filling going on first pass only //
	if (1 == passNumber) {

		string labelId(labelName);

		// check for duplicate definition //
		if (debugInfo.getLabelAddress(labelName, NULL)) {
			errorPosition = cPos;
			errorMessage = "Duplicate label identifier";
			return false;
		}

		debugInfo.addLabel(labelName, labelValue);
	}
	return true;
}

bool i8080asm::recognizeMnemonic(const char *mPos,
		const char *tPos, unsigned int *mnemoCode)
{
	return recognizeLexem(mnemonics, mPos, tPos, mnemoCode);
}

bool i8080asm::recognizeRegister(const char *rPos, const char *rePos,
			unsigned int *regIdx)
{
	const char *cPos = rPos;

	// sigle register always identified by single character //
	cPos++;

	if (cPos != rePos)
		return false;

	unsigned int idx = 0;
	while ((idx < 8) && (toupper(*rPos) != regs[idx]))
		idx++;

	if (idx < 8) {

		if (NULL != regIdx)
			*regIdx = idx;

		if (NULL != rePos) {
			rePos = cPos;
			return true;
		}
	}

	return false;
}

bool i8080asm::recognizeregisterPairId(const char *rpPos,
		const char *rpePos, const char *regSamples[],
		unsigned int *regIdx) {

	if (true == recognizeLexem(regSamples, rpPos, rpePos, regIdx)) {

		if (NULL != regIdx)
			*regIdx = *regIdx & 3;

		return true;
	}
	
	return false;
}

// this function treats SP as last registers pair //
bool i8080asm::recognizeRegistersPair(const char *rpPos,
		const char *rpePos, unsigned int *regIdx)
{
	return recognizeregisterPairId(rpPos, rpePos, regPairs , regIdx);
}

// this function treats PSW as last registers pair //
bool i8080asm::recognizeRegistersPair2(const char *rpPos,
		const char *rpePos, unsigned int *regIdx)
{
	return recognizeregisterPairId(rpPos, rpePos, regPairs2 , regIdx);
}

bool i8080asm::getOperandValue(const char *cPos, const char **sePos,
		unsigned int *result)
{
	bool status;

	// a sequence of quoted characters //
	if ('"' == *cPos) {

		// result of parsing will be stored here //
		StringBinarizer includeName;

		// result code of parsing //
		unsigned int parseCode;

		unsigned int currentValue = 0;
		string parsedSequence;

		parseCode = parseString(cPos, sePos, false, &includeName, NULL);
		parsedSequence = includeName.getContent();

		if (parsedSequence.length() > 4) {
			errorPosition = cPos;
			errorMessage = "Value too big";
			return false;
		}

		for (unsigned int i = 0; i < parsedSequence.length(); i++) {
			currentValue = currentValue << 8;
			currentValue += parsedSequence[i];
		}

		if (NULL != result)
			*result = currentValue;

		return true;
	}

	if (true == isHexDigit(*cPos)) {

		status = getNumber(cPos, sePos, result);

		if (false == status) {

			// label may begin with HEX-specific character //
			if (! isDigit(*cPos))
				status =  getLabelAddress(cPos, sePos, result);
		}
	} else {

		status =  getLabelAddress(cPos, sePos, result);
	}

	return status;
}

// calculate high-prioiry operators sequence //
bool i8080asm::getHiExpressionValue(const char *expPos, const char **expEnd,
			unsigned int *result)
{
	bool status;
	unsigned int currentValue = 0;
	unsigned int newValue;
	char whatToDo;
	const char *cPos = overSpaces(expPos);
	const char *sePos;

	if ((! endOfLine(*cPos)) && (',' != *cPos) &&
			('+' != *cPos) && ('-' != *cPos) && ('(' != *cPos)) {
		// get first operand of expression //
		status = getOperandValue(cPos, &sePos, &currentValue);
		if (false == status)
			return false;

		cPos = overSpaces(sePos);
	}

	if ('(' == *cPos) {
		cPos++;
		// get value of expression in parentheses //
		status = getLowExpressionValue(cPos, &sePos, &currentValue, true);
		if (false == status)
			return false;
		cPos = overSpaces(sePos);
	}

	while ((! endOfLine(*cPos)) && (',' != *cPos) &&
			('+' != *cPos) && ('-' != *cPos) && (')' != *cPos)) {

		if (('*' != *cPos) && ('/' != *cPos)) {
			errorPosition = cPos;
			errorMessage = "Error in expression";
			return false;
		}

		whatToDo = *cPos;
		cPos++;
		cPos = overSpaces(cPos);

		if ('(' == *cPos) {
			cPos++;
			// get value of expression in parentheses //
			status = getLowExpressionValue(cPos, &sePos, &newValue, true);
			if (false == status)
				return false;
		} else {
			// get next operand of expression //
			status = getOperandValue(cPos, &sePos, &newValue);
			if (false == status)
				return false;
		}

		switch (whatToDo) {

			case '*':
				// multiply has greater priority than + and - //
				currentValue *= newValue;
				cPos = sePos;
				break;

			case '/':
				// dividing has greater priority than + and - //
				if (0 == newValue) {
					errorPosition = cPos;
					errorMessage = "Division by zero";
					return false;
				}
				currentValue /= newValue;
				cPos = sePos;
				break;
			case '(':
				// get value of expression in parentheses //
				status = getLowExpressionValue(cPos, &sePos, &newValue, true);
				if (false == status)
					return false;
				break;
		}

		cPos = overSpaces(sePos);
	}

	if (NULL != expEnd)
		*expEnd = cPos;

	if (NULL != result)
		*result = currentValue;

	return true;
}

bool i8080asm::getLowExpressionValue(const char *expPos, const char **expEnd,
			unsigned int *expResult, bool pts)
{
	bool status;
	const char *cPos = overSpaces(expPos);
	const char *sePos;
	char whatToDo = '+';
	unsigned int currentValue = 0;
	unsigned int newValue;

	// get first operand of expression //
	if ((! endOfLine(*cPos)) && (',' != *cPos)) {
		status = getHiExpressionValue(cPos, &sePos, &currentValue);
		if (false == status)
			return false;
		cPos = overSpaces(sePos);
	}

	while ((! endOfLine(*cPos)) && (',' != *cPos) && (')' != *cPos)) {

		// check operator //
		if (('-' != *cPos) && ('+' != *cPos)) {
			errorPosition = cPos;
			errorMessage = "Error in expression";
			return false;
		}

		whatToDo = *cPos;
		cPos++;
		cPos = overSpaces(cPos);

		// get next operand of expression //
		status = getHiExpressionValue(cPos, &sePos, &newValue);
		if (false == status)
			return false;
		
		switch (whatToDo) {

			case '+':
					currentValue += newValue;
				break;

			case '-':
					currentValue -= newValue;
				break;
		}


		cPos = overSpaces(sePos);
	}

	if ((!pts) && (')' == *cPos)) {
		errorPosition = cPos;
		errorMessage = "Unexpected parenthesis in expression";
		return false;
	}

	if ((pts) && (')' != *cPos)) {
		errorPosition = cPos;
		errorMessage = "Missing closing parenthesis";
		return false;
	}

	// walk over closing parenthesis //
	if (pts)
		cPos++;

	if (NULL != expEnd)
		*expEnd = cPos;

	if (NULL != expResult)
		*expResult = currentValue;

	return true;
}

bool i8080asm::getExpressionValue(const char *expPos, const char **expEnd,
			unsigned int *expResult)
{
	return getLowExpressionValue(expPos, expEnd, expResult, false);
}

bool i8080asm::getLabelAddress(const char *labelPos, const char **labelEnd,
		unsigned int *address)
{
	char labelId[32];
	const char *curLabelChar = labelPos;
	const char *endLabelChar = overWord(curLabelChar);
	unsigned int labelIdx = 0;

	if (curLabelChar == endLabelChar) {
		errorPosition = curLabelChar;
		errorMessage = "Malformed identifier";
		return false;
	}

	while ((curLabelChar != endLabelChar) &&
			(labelIdx < 32)) {

		labelId[labelIdx] = *curLabelChar;

		curLabelChar++;
		labelIdx++;
	}

	if (labelIdx <= 31) {
		labelId[labelIdx] = '\0';
	} else {
		errorPosition = labelPos;
		errorMessage = "Identifier is too long";
		return false;
	}

	string labelSample(labelId);

	if (! debugInfo.getLabelAddress(labelId, address)) {
		if (1 == passNumber) {

			// on first pass all undefined identifiers has zero value //
			if (NULL != address)
				*address = 0;
			
		} else {

			// on second pass all identifiers must be defined //
			errorPosition = labelPos;
			errorMessage = "Unknown identifier";
			return false;
		}
	}

	if (NULL != labelEnd)
		*labelEnd = endLabelChar;
	
	return true;
}

bool i8080asm::getNumber(const char *numPos, const char **numEnd,
			unsigned int *numValue)
{
	const char *cPos = numPos;
	const char *ePos = cPos;
	unsigned int byHexValue = 0;
	unsigned int byDecValue = 0;
	bool byDec = true;
	bool status = true;
	bool hexConfirmed = false;

	ePos = overWord(ePos);
	if (ePos == cPos) {
		errorPosition = cPos;
		errorMessage = "Bad numeric format";
		status = false;
	}

	while ((cPos != ePos) && (true == status)) {

		if (isHexDigit(*cPos)) {

			if ((true == byDec) && (isDigit(*cPos))) {
				byDecValue = (byDecValue * 10) + getDigitValue(*cPos);			
			} else {
				byDec = false;
			}

			byHexValue = (byHexValue * 16) + getDigitValue(*cPos);			

			cPos++;

		} else {
			// at the end of number a "h" character may present //
			// it means that number is in hex format //
			if ('H' == toupper(*cPos)) {
				byDec = false;
				hexConfirmed = true;
				cPos++;

				// "h" should always be at the end of number //
				if (cPos != ePos) {
					errorPosition = cPos;
					errorMessage = "Bad numeric format";
					status = false;
				}
			} else {
				// it is not digit character at all //
				errorPosition = cPos;
				errorMessage = "Bad numeric format";
				status = false;
			}
		}
	}

	if (!byDec && !hexConfirmed)
	    return false;

	if (NULL != numEnd)
		*numEnd = cPos;

	if (NULL != numValue) {
		if (byDec) {
			*numValue = byDecValue;
		} else {
			*numValue = byHexValue;
		}
	}

	return status;
}

void i8080asm::setAddress(unsigned int newAddress)
{
	currentAddress = newAddress;
}

bool i8080asm::dispatchMnemonic(unsigned int mnemoCode,
		const char **line)
{
	bool status = false;
	char codeBuf[50];
	const char *args = *line;
	const char *cPos;
	const MnemonicDescription *mnemonicDescription;
	unsigned int firstArg;
	unsigned int secondArg;

	// check if this mnemonic produces exact code //
	mnemonicDescription = &codeMap[mnemoCode];

	// origin of binary cannot be changed as some code was produced //
	if (0 != mnemonicDescription->instructionLength)
		originDefined = true;

	if (0 < mnemonicDescription->argNumber) {

		if (endOfLine(*args)) {
			errorPosition = args;
			errorMessage = "Missing argument";
			return false;
		}

		if (false == getMnemonicArgument(mnemonicDescription->argTypes[0],
						args,  &cPos, &firstArg))
			return false;

		cPos = overSpaces(cPos);

		if (! endOfLine(*cPos) && (1 == mnemonicDescription->argNumber)) {
			errorPosition = cPos;
			errorMessage = "Extra argument";
			return false;
		}
		
		// move position behind first argument //
		*line = cPos;
	}

	if (1 < mnemonicDescription->argNumber) {

		if (',' != *cPos) {
			errorMessage = "Missing second argument";
			errorPosition = cPos;
			return false;
		}
		cPos++;

		cPos = overSpaces(cPos);
		if (endOfLine(*cPos)) {
			errorMessage = "Missing second argument";
			errorPosition = cPos;
			return false;
		}

		if (false == getMnemonicArgument(mnemonicDescription->argTypes[1],
						cPos,  &cPos, &secondArg))
			return false;

		cPos = overSpaces(cPos);
		if (! endOfLine(*cPos)) {
			errorPosition = cPos;
			errorMessage = "Extra argument";
			return false;
		}

		// move position behind second argument //
		*line = cPos;
	}

	switch(mnemoCode) {

		case LXI:
			status = compileLxi(firstArg, secondArg);
		break;

		case STAX:
			status = compileStax(firstArg);
		break;

		case INX:
			status = compileInx(firstArg);
		break;

		case INR:
			status = compileInr(firstArg);
		break;

		case DCR:
			status = compileDcr(firstArg);
		break;

		case MVI:
			status = compileMvi(firstArg, secondArg);
		break;

		case DAD:
			status = compileDad(firstArg);
		break;

		case LDAX:
			status = compileLdax(firstArg);
		break;

		case DCX:
			status = compileDcx(firstArg);
		break;

		case MOV:
			status = compileMov(firstArg, secondArg);
		break;

		case POP:
			status = compilePop(firstArg);
		break;

		case PUSH:
			status = compilePush(firstArg);
		break;

		case RST:
			status = compileRst(firstArg);
		break;

		case ORG:
			status = processOrg(firstArg);
		break;

		case DB:
			status = compileDb(args, line);
		break;

		case DS:
			status = compileDs(args, line);
		break;

		case DW:
			status = compileDw(args, line);
		break;

		case END:
			compileCompleted = true;
			status = true;
		break;

		default:
			// instructions without arguments always exact //
			if (0 == mnemonicDescription->argNumber)
				return compileExactMnemonic(mnemoCode, args);

			// try to match exact multibyte instructions //
			if ((3 == mnemonicDescription->instructionLength) &&
					(1 == mnemonicDescription->argNumber) &&
					(ARG_WORD == mnemonicDescription->argTypes[0])) {
				status =  compileExact3Mnemonic(mnemonicDescription,
						firstArg);
				break;
			}

			// try to match exact multibyte instructions //
			if ((2 == mnemonicDescription->instructionLength) &&
					(1 == mnemonicDescription->argNumber) &&
					(ARG_BYTE == mnemonicDescription->argTypes[0])) {
				status =  compileExact2Mnemonic(mnemonicDescription,
						firstArg);
				break;
			}

			// try to match register-to-accumulator instruction //
			if ((1 == mnemonicDescription->instructionLength) &&
					(1 == mnemonicDescription->argNumber) &&
					(ARG_REG == mnemonicDescription->argTypes[0])) {
				status = compileExactRMnemonic(mnemonicDescription,
					firstArg);
				break;
			}

			snprintf(codeBuf, sizeof(codeBuf), "%d", mnemoCode);
			errorMessage = string("Unknown mnemonic code: ") +
					string(codeBuf);
			return false;
		break;
	}

	// increase current compilation address to compiled data length //
	if (true != status) {
		// complete error message //
		if (0 != errorMessage.length()) {
			errorMessage = errorMessage + string(".");
		} else {
			errorMessage = "Undefined error.";
		}
	}

	return status;
}

bool i8080asm::compileExactMnemonic(unsigned int mnemoCode, const char *args)
{
	const MnemonicDescription *mnemonicDescription;
	
	// no arguments required for this mnemonic //
	if (! endOfLine(*args)) {
		errorPosition = args;
		errorMessage = "Extra argument for mnemonic";
		return false;
	}

	mnemonicDescription = &codeMap[mnemoCode];

	currentAddress++;
	if (passNumber < 2)
		binarySize++;
	if (passNumber > 1)
		_binarizer->putByte(mnemonicDescription->exactCode);

	return true;
}

bool i8080asm::compileExact2Mnemonic(const MnemonicDescription *mnemonicDescription,
			unsigned int firstArg)
{
	currentAddress += 2;
	if (passNumber < 2) {
		binarySize += 2;
		return true;
	}
	_binarizer->putByte(mnemonicDescription->exactCode);
	_binarizer->putByte(firstArg & 0xff);
	return true;
}

bool i8080asm::compileExact3Mnemonic(const MnemonicDescription *mnemonicDescription,
			unsigned int firstArg)
{
	currentAddress += 3;
	if (passNumber < 2) {
		binarySize += 3;
		return true;
	}
	_binarizer->putByte(mnemonicDescription->exactCode);
	_binarizer->putByte(firstArg & 0xff);
	_binarizer->putByte(firstArg >> 8);
	return true;
}

bool i8080asm::compileExactRMnemonic(const MnemonicDescription *mnemonicDescription,
			unsigned int firstArg)
{
	currentAddress++;
	if (passNumber < 2) {
		binarySize++;
		return true;
	}
	_binarizer->putByte(mnemonicDescription->exactCode | firstArg);
	return true;
}

bool i8080asm::getMnemonicArgument(char argType, const char *argPos,
					const char **cPos, unsigned int *argVal)
{
	const char *aePos = argPos;
	bool status;
	unsigned int result;

	switch(argType) {
		case ARG_REG:
				aePos = overWord(argPos);
				status = recognizeRegister(argPos, aePos, &result);
				if (! status) {
					errorMessage = "Bad register identifier";
					errorPosition = argPos;
				}
			break;

		case ARG_REGPAIR:
				aePos = overWord(argPos);

				status = recognizeRegistersPair(argPos, aePos, &result);
				if (! status) {
					errorMessage = "Bad register pair identifier";
					errorPosition = argPos;
				}

			break;

		case ARG_REGPAIR2:
				aePos = overWord(argPos);

				status = recognizeRegistersPair2(argPos, aePos, &result);
				if (! status) {
					errorMessage = "Bad register pair identifier";
					errorPosition = argPos;
				}

			break;

		case ARG_BYTE:
			status = getExpressionValue(argPos, &aePos, &result);
			if (! status && (0 == errorMessage.length())) {
				errorMessage = "Error in expression";
				errorPosition = aePos;
			}

			result &= 0xff;
			break;

		case ARG_WORD:
			status = getExpressionValue(argPos, &aePos, &result);
			if (! status && (0 == errorMessage.length())) {
				errorMessage = "Error in expression";
				errorPosition = aePos;
			}

			result &= 0xffff;
			break;
	}

	if ((true == status) && (NULL != argVal))
		*argVal = result;

	if ((true == status) && (NULL != cPos))
		*cPos = aePos;

	return status;
}

bool i8080asm::compileLxi(unsigned int regIdx, unsigned int data)
{
	currentAddress += 3;
	if (passNumber < 2) {
		binarySize += 3;
		return true;
	}
	_binarizer->putByte(0x01 | (regIdx << 4));
	_binarizer->putByte(data & 0xff);
	_binarizer->putByte((data & 0xff00) >> 8);
	return true;
}

bool i8080asm::compileStax(unsigned int regIdx)
{
	if (regIdx > 1) {
		errorPosition = NULL;
		errorMessage = "Inappropriate register pair for STAX mnemonic";
		return false;
	}

	currentAddress++;
	if (passNumber < 2) {
		binarySize++;
		return true;
	}
	_binarizer->putByte(0x02 | (regIdx << 4));
	return true;
}

bool i8080asm::compileInx(unsigned int regIdx)
{
	currentAddress++;
	if (passNumber < 2) {
		binarySize++;
		return true;
	}
	_binarizer->putByte(0x03 | (regIdx << 4));
	return true;
}

bool i8080asm::compileInr(unsigned int regIdx)
{
	currentAddress++;
	if (passNumber < 2) {
		binarySize++;
		return true;
	}
	_binarizer->putByte(0x04 | (regIdx << 3));
	return true;
}

bool i8080asm::compileDcr(unsigned int regIdx)
{
	currentAddress++;
	if (passNumber < 2) {
		binarySize++;
		return true;
	}
	_binarizer->putByte(0x05 | (regIdx << 3));
	return true;
}

bool i8080asm::compileMvi(unsigned int regIdx, unsigned int imData)
{
	currentAddress += 2;
	if (passNumber < 2) {
		binarySize += 2;
		return true;
	}
	_binarizer->putByte(0x06 | (regIdx << 3));
	_binarizer->putByte(imData);
	return true;
}

bool i8080asm::compileDad(unsigned int regIdx)
{
	currentAddress++;
	if (passNumber < 2) {
		binarySize++;
		return true;
	}
	_binarizer->putByte(0x09 | (regIdx << 4));
	return true;
}

bool i8080asm::compileLdax(unsigned int regIdx)
{
	if (regIdx > 1) {
		errorPosition = NULL;
		errorMessage = "Inappropriate register pair for LDAX mnemonic";
		return false;
	}

	currentAddress++;
	if (passNumber < 2) {
		binarySize++;
		return true;
	}
	_binarizer->putByte(0x0A | (regIdx << 4));
	return true;
}

bool i8080asm::compileDcx(unsigned int regIdx)
{
	currentAddress++;
	if (passNumber < 2) {
		binarySize++;
		return true;
	}
	_binarizer->putByte(0x0B | (regIdx << 4));
	return true;
}

bool i8080asm::compileMov(unsigned int regIdx, unsigned int regIdx2)
{
	// Mov between M and M is a Hlt instruction actually //
	if ((6 == regIdx) && (6 == regIdx2)) {
		errorPosition = NULL;
		errorMessage = "Inappropriate source and target registers";
		return false;
	}

	currentAddress++;
	if (passNumber < 2) {
		binarySize++;
		return true;
	}
	_binarizer->putByte(0x40 | (regIdx << 3) | regIdx2);
	return true;
}

bool i8080asm::compilePop(unsigned int regIdx)
{
	currentAddress++;
	if (passNumber < 2) {
		binarySize++;
		return true;
	}
	_binarizer->putByte(0xc1 | (regIdx << 4));
	return true;
}

bool i8080asm::compilePush(unsigned int regIdx)
{
	currentAddress++;
	if (passNumber < 2) {
		binarySize++;
		return true;
	}
	_binarizer->putByte(0xc5 | (regIdx << 4));
	return true;
}

bool i8080asm::compileRst(unsigned int intIdx)
{
	if (intIdx > 7) {
		errorPosition = NULL;
		errorMessage = "Inappropriate value for restart";
		return false;
	}

	currentAddress++;
	if (passNumber < 2) {
		binarySize++;
		return true;
	}
	_binarizer->putByte(0xc7 | (intIdx << 3));
	return true;
}

bool i8080asm::processOrg(unsigned int orgAddr)
{
	currentAddress = orgAddr;
	if (!originDefined) {
		originAddress = orgAddr;
	}
	return true;
}

bool i8080asm::compileDb(const char *data, const char **newPos)
{
	const char *cPos = data;
	const char *vePos;

	while (true) {
		if ('"' == *cPos) {
			// quoted string //
					
			// cPos will be positioned after final quote character //
			if (false == compileDbString(data, &vePos))
				return false;

		} else {

			unsigned int dbValue;
			unsigned int duplicates = 1;

			// a number //
			if (false == getExpressionValue(cPos, &vePos, &dbValue))
				return false;

			dbValue &= 0xff;

			cPos = overSpaces(vePos);

			if (passNumber > 1) {
				for (unsigned int i = 0; i < duplicates; i++)
					_binarizer->putByte(dbValue);
			}

			if (passNumber < 2)
				binarySize++;

			currentAddress++;
			originDefined = true;
		}

		cPos = overSpaces(vePos);

		if (endOfLine(*cPos)) {
			*newPos = cPos;
			return true;
		}

		if (',' != *cPos) {
			errorPosition = cPos;
			errorMessage = "Syntax error";
			return false;
		}

		cPos++;
		cPos = overSpaces(cPos);
	}

	// must never be happen //
	return false;
}

bool i8080asm::compileDs(const char *data, const char **newPos)
{
	const char *cPos = data;

	unsigned int dsValue;
	unsigned int secondArg;
	unsigned char fillValue;
	const char *vePos;

	// a number of bytes //
	if (false == getExpressionValue(cPos, &vePos, &dsValue))
		return false;

	dsValue &= 0xffff;

	cPos = overSpaces(vePos);

	if (endOfLine(*cPos)) {
		fillValue = 0;
	} else {

		if (',' != *cPos) {
			errorPosition = cPos;
			errorMessage = "Syntax error";
			return false;
		}

		cPos++;
		cPos = overSpaces(cPos);

		// value to fill //
		if (false == getExpressionValue(cPos, &vePos, &secondArg))
			return false;

		fillValue = secondArg & 0xff;

		cPos = overSpaces(vePos);
		if (! endOfLine(*cPos)) {
			errorPosition = cPos;
			errorMessage = "Extra arguments";
			return false;
		}
	}

	if (passNumber > 1) {
		for (unsigned int i = 0 ; i < dsValue; i++)
			_binarizer->putByte(fillValue);
	}

	if (passNumber < 2)
		binarySize += dsValue;

	currentAddress += dsValue;
	originDefined = true;

	*newPos = cPos;

	return true;
}

bool i8080asm::compileDw(const char *data, const char **newPos)
{
	const char *cPos = data;

	while (true) {

		unsigned int dwValue;
		const char *vePos;

		// a number //
		if (false == getExpressionValue(cPos, &vePos, &dwValue))
			return false;

		dwValue &= 0xffff;

		if (passNumber > 1) {
			_binarizer->putByte(dwValue & 0xff);
			_binarizer->putByte((dwValue >> 8) & 0xff);
		}

		if (passNumber < 2)
			binarySize += 2;

		currentAddress += 2;
		originDefined = true;

		cPos = overSpaces(vePos);
		if (endOfLine(*cPos)) {
			*newPos = cPos;
			return true;
		}

		if (',' != *cPos) {
			errorPosition = cPos;
			errorMessage = "Syntax error";
			return false;
		}

		cPos++;
		cPos = overSpaces(cPos);
	}

	// must not execute //
	return false;
}

bool i8080asm::compileDbString(const char *data, const char **sePos)
{
	const char *cPos = data;

	// string must begin with quote character //
	if ('"' != *cPos) {
		errorPosition = cPos;
		errorMessage = "Malformed string";
		return false;
	}

	// walk over opening quote character //
	cPos++;

	while (('"' != *cPos) && ('\0' != *cPos)) {

		// check for escaped character //
		if ('\\' == *cPos) {
			cPos++;

			// check for unexpected end of line //
			if ('\0' == *cPos)
			continue;
		}

		originDefined = true;

		if (passNumber > 1)
			_binarizer->putByte(*cPos);

		if (passNumber < 2)
			binarySize++;

		currentAddress++;
		cPos++;
	}

	if ('\0' == *cPos) {
		errorPosition = cPos;
		errorMessage = "Mission closing quote";
		return false;
	}

	// walk over closing quote character //
	cPos++;

	if (NULL != sePos)
		*sePos = cPos;

	return true;
}

i8080asm::~i8080asm()
{
}

