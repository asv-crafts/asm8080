
using namespace std;

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <list>
#include <map>
#include "Binarizer.h"
#include "FileBinarizer.h"
#include "DebugInfo.h"
#include "i8080asm.h"
#include "Source.h"
#include "Preprocessor.h"

using std::string;
using std::list;
using std::map;

int main(int argc, char *argv[])
{

	if (argc < 3) {
		printf("Usage:\n   i8080asm <input source file> <output RKS file> \\\n"
				"		<optional ouput debug information file>\n");
		return 1;
	}

	string primarySource;
	primarySource = argv[1];
	
	FileBinarizer binarizer(argv[2]);
	i8080asm compiler(&binarizer);
	Preprocessor preprocessor(primarySource, &compiler, &binarizer);

	if (false == preprocessor.compile()) {
		printf("%s\n", preprocessor.getErrorMessage().c_str());
		return 1;
	}

	if (argc > 3)
		// dump debug information to hints file //
		compiler.dumpDebugInfo(argv[3]);

	return 0;
}
