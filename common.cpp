
#include <stdlib.h>
#include <ctype.h>
#include <string>
#include "Binarizer.h"

using std::string;

#define ERR_MALSTRING     1
#define ERR_MIS_END_QUOTE 2
#define MAX_ERR_CODE      2

char *commonErrorMessages[] = {
	"No error",
	"Malformed string",
	"Missing closing quote"
};


bool isAlpha(char character)
{
	bool var1 = (('A' <= character) && ('Z' >= character));
	bool var2 = (('a' <= character) && ('z' >= character));
	return (var1 || var2);
}

bool isDigit(char character)
{
	return (('0' <= character) && ('9' >= character));
}

bool isHexDigit(char character)
{
	return (('0' <= character) && ('9' >= character)) ||
		(('A' <= character) && ('F' >= character)) ||
		(('a' <= character) && ('f' >= character));
}

bool endOfLine(char character)
{
	return (('\n' == character) || (';' == character) ||
		('\0' == character));
}

const char *overWord(const char *inputLine)
{
	const char *nPos = inputLine;

	while (isAlpha(*nPos) || isDigit(*nPos) ||
				('_' == *nPos) || ('@' == *nPos))
		nPos++;

	return nPos;
}

const char *overSpaces(const char *inputLine)
{
	const char *nPos = inputLine;
	while ((' ' == *nPos) || ('\t' == *nPos))
		nPos++;

	return nPos;
}

unsigned int getDigitValue(char digit)
{
	if (digit <= '9') {
		return digit - '0';
	} else {
		return toupper(digit) - 'A' + 10;
	}
}

bool recognizeLexem(const char *lexems[], const char *mPos,
			const char *tPos, unsigned int *lexemCode)
{
	// iterator on a lexem from lexems list //
	const char *samplePos = NULL;

	// iterator on recognizing lexem //
	const char *lexemPos = NULL;

	bool found = false;

	// current lexem index in lexems list //
	unsigned int idx = 0;

	while (!found && (lexems[idx][0] != '\0')) {

		// init iterators for comparing lexems //
		samplePos = lexems[idx];
		lexemPos = mPos;

		// compare lexems char by char with "ingnore case" manner //
		while ((toupper(*lexemPos) == *samplePos) &&
				('\0' != *samplePos)) {
			lexemPos++;
			samplePos++;
		}

		// lexem is recognized only in case when both //
		// iterators are at the end of lexems         //
		if (('\0' == *samplePos) && (lexemPos == tPos)) {
			found = true;
		} else {
			idx++;
		}
	}

	if ((found) && (NULL != lexemCode))
		*lexemCode = idx;

	return found;
}

// if parseOnly is true, no data will be sent to binarizer //
int parseString(const char *data, const char **sePos, bool parseOnly,
	Binarizer *binarizer, unsigned int *stringSize)
{
	const char *cPos = data;
	unsigned int parsedStringSize = 0;

	// string must begin with quote character //
	if ('"' != *cPos) {
		if (NULL != sePos)
			*sePos = cPos;
		return ERR_MALSTRING;
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

		if (! parseOnly) {
			binarizer->putByte(*cPos);
			parsedStringSize++;
		}

		cPos++;
	}

	if ('\0' == *cPos) {
		if (NULL != sePos)
			*sePos = cPos;
		return ERR_MIS_END_QUOTE;
	}

	// walk over closing quote character //
	cPos++;

	if (NULL != sePos)
		*sePos = cPos;

	if (NULL != stringSize)
		*stringSize = parsedStringSize;

	return 0;
}

const char *getErrorDescription(unsigned int errorCode)
{
	if (errorCode > MAX_ERR_CODE)
		return NULL;

	return commonErrorMessages[errorCode];
}

