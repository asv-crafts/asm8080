
#include <string>
#include <map>
#include <list>
#include <stdio.h>
#include "Binarizer.h"
#include "FileBinarizer.h"
#include "StringBinarizer.h"
#include "DebugInfo.h"
#include "i8080asm.h"
#include "Source.h"
#include "common.h"
#include "Preprocessor.h"

using std::string;
using std::map;
using std::list;
using std::FILE;

const static char *directives[] = { "INCLUDE", "" };

Preprocessor::Preprocessor(string primarySource, i8080asm *sourceCompiler,
		FileBinarizer *dataBinarizer)
:_primarySource(primarySource), compiler(sourceCompiler),
 binarizer(dataBinarizer)
{
	currentSource = NULL;
}

bool Preprocessor::compile()
{
	passNumber = 1;
	if (false == startPass())
		return false;
	passNumber = 2;
	binarizer->setOrigin(compiler->getOrigin());
	binarizer->setBinarySize(compiler->getBinarySize());
	return startPass();
}

bool Preprocessor::startPass()
{
	bool result;
	compiler->setPassNumber(passNumber);
	result = preprocessFile(_primarySource);
	return result;
}

bool Preprocessor::preprocessFile(string sourceFilename)
{
	char lineBuf[1024];
	bool result = true;
	bool completed = false;

	// check, if file with same name already in included files //
	if (find(filesInProcessing.begin(), filesInProcessing.end(),
			sourceFilename) != filesInProcessing.end()) {
		reportError("File " + sourceFilename + " is already included");
		return false;
	}

	filesInProcessing.push_back(sourceFilename);

	currentSource = new Source(sourceFilename);

	while ((true == result) && (false == completed)) {

		result = currentSource->getLine(lineBuf, sizeof(lineBuf));
		if (false == result) {

			if (true == currentSource->endOfFile()) {
				completed = true;
				result = true;
			} else {
				char numBuf[10];

				snprintf(numBuf, sizeof(numBuf), "%u",
						currentSource->getLineNumber());
				errorMessage = currentSource->getErrorMessage();
				result = false;
			}

		} else {

			if (strlen(lineBuf) >= (sizeof(lineBuf) - 1)) {

				reportError("Line too long");
				result = false;

			} else {

				if ('#' == lineBuf[0]) {
					// a preprocessor directive detected //
					result = preprocessLine(lineBuf);
				} else {

					// regular assembly line... probably //
					result = compiler->compileLine(lineBuf);

					if (false == result)
						reportError(compiler->getErrorMessage());

					completed = compiler->isCompleted();
				}
			}
		}
	}

	delete currentSource;
	currentSource = NULL;
	filesInProcessing.pop_back();
	return result;
}

bool Preprocessor::preprocessLine(const char *line)
{
	const char *cPos = line;
	const char *mPos;
	unsigned int directiveCode;
	unsigned parseCode;
	StringBinarizer includeName;
	Source *sourceBackup;
	bool result;

	cPos++;
	cPos = overSpaces(cPos);
	mPos = overWord(cPos);

	if (false == recognizeLexem(directives, cPos, mPos, &directiveCode)) {
		reportError("Invalid preprocessor directive");
		return false;
	}

	if (0 != directiveCode) {
		reportError("Internal error. Unimplememted preprocessor directive");
		return false;
	}

	cPos = overSpaces(mPos);

	parseCode = parseString(cPos, &mPos, false, &includeName, NULL);

	if (0 != parseCode) {
		reportError(getErrorDescription(parseCode));
		return false;
	}

	sourceBackup = currentSource;

	result = preprocessFile(includeName.getContent());

	currentSource = sourceBackup;

	return result;
}

void Preprocessor::reportError(string message)
{
	char numBuf[10];

	snprintf(numBuf, sizeof(numBuf), "%u", currentSource->getLineNumber());
	errorMessage = string("Error at line ") +
			string(numBuf) + string(" of file ") +
			currentSource->getFilename() + string("\n") + 
			message;
}

Preprocessor::~Preprocessor()
{
}
