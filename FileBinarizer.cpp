
#include <stdio.h>
#include <string>
#include "Binarizer.h"
#include "FileBinarizer.h"

using std::string;
using std::FILE;

FileBinarizer::FileBinarizer(string target)
{
	binaryFilename = target;
	binaryFile = NULL;
	binarizerState = INITIAl_STATE;
}

bool FileBinarizer::putByte(unsigned char data)
{
	switch(binarizerState) {

		case INITIAl_STATE:
			// open the target binary file //
			binaryFile = fopen(binaryFilename.c_str(),
					"wb");
			if (NULL == binaryFile) {
				binarizerState = FAILED_STATE;
				errorMessage = string("Could not open file for writing ") +
					binaryFilename;
				return false;
			}
			
			binarizerState = PROCESSING_STATE;

			// form RKS binary header //
			if (false == _putByte(originAddress & 0xff))
				return false;

			if (false == _putByte((originAddress >> 8) & 0xff))
				return false;

			if (false == _putByte((originAddress + binarySize - 1) & 0xff))
				return false;

			if (false == _putByte(((originAddress + binarySize - 1) >> 8) & 0xff))
				return false;

		case PROCESSING_STATE:
			return _putByte(data);
		break;

		case FAILED_STATE:
			return false;
		break;
	}

	errorMessage = "Internal error in binarizer: unknown state";
	return false;
}

bool FileBinarizer::_putByte(unsigned char data)
{
	if (EOF == fputc(data, binaryFile)) {
		binarizerState = FAILED_STATE;
		errorMessage = string("Error occured during writing file") +
			binaryFilename;
		return false;
	}
	return true;
}

FileBinarizer::~FileBinarizer()
{
	if (NULL != binaryFile) {
		// FIXME: need a real checksum //
		_putByte(0);
		_putByte(0);
		fclose(binaryFile);
	}
}

