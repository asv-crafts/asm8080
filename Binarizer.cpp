
#include <string>
#include "Binarizer.h"

using std::string;

Binarizer::Binarizer()
{
}

bool Binarizer::putByte(unsigned char data)
{
	errorMessage = "Internal error: Base Binarizer class has been used";
	return false;
}

Binarizer::~Binarizer()
{
}


