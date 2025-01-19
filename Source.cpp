
#include <string>
#include <stdio.h>
#include "Source.h"

using std::string;
using std::FILE;

Source::Source(string filename)
{
	eof = false;
	sourceFilename = filename;
	sourceFile = NULL;
	lineNumber = 0;
}

bool Source::getLine(char *lineHolder, unsigned int holderSize)
{
	// it is incorrrect to increase line number after reading line  //
	// so, we start count from zero and increase line number before //
	// line reading                                                 //
	lineNumber++;

	if ((NULL == lineHolder) || (0 == holderSize)) {
		errorMessage = "Internal error at Source::getLine() - bad arguments given";
		return false;
	}

	if (NULL == sourceFile) {
		sourceFile = fopen(sourceFilename.c_str(), "r");
		if (NULL == sourceFile) {
			errorMessage = string("Could not open file ") + sourceFilename;
			return false;
		}
	}

	if (NULL == fgets(lineHolder, holderSize, sourceFile)) {

		if (0 != feof(sourceFile)) {
			eof = true;
			return false;
		}

		if (0 != ferror(sourceFile)) {
			errorMessage = string("Error reading file ") + sourceFilename;
			return false;
		}

		errorMessage = string("Undefined error occured while reading file ") +
				sourceFilename;
		return false;
	}

	return true;
}

bool Source::close()
{
	if (NULL != sourceFile) {
		fclose(sourceFile);
		sourceFile = NULL;
	}
	return true;
}

Source::~Source()
{
	if (NULL != sourceFile)
		fclose(sourceFile);
}
