
#include <string>
#include "Binarizer.h"
#include "StringBinarizer.h"

using std::string;

StringBinarizer::StringBinarizer()
{
}

bool StringBinarizer::putByte(unsigned char data)
{
	content += data;
	return true;
}

StringBinarizer::~StringBinarizer()
{
}

