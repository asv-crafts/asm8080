
#define TAB_SIZE  8
#define COMMENTS_COLUMN 40

#include "errno.h"
#include <string>
#include <map>
#include "DebugInfo.h"

DebugInfo::DebugInfo()
:bpNo(0)
{
}

bool DebugInfo::loadInfo(const char *filename)
{
	FILE *hints;
	char hintLine[256];
	char *cPos;
	string labelName;
	string comment;
	unsigned int labelAddress;

	if (NULL == filename)
		return false;

	hints = fopen(filename, "r");

	if (NULL == hints) {
		printf("Could not open hints file %s", filename);
		return false;
	}

	while (NULL != fgets(hintLine, sizeof(hintLine), hints)) {

		cPos = hintLine;
		labelName = "";
		labelAddress = 0;
		comment = "";

		// First field of hints is a label name. //
		// Note, that for inline comments label  //
		// may have empty name.                  //
		while ((':' != *cPos) && ('\0' != *cPos)) {
			labelName += *cPos;
			cPos++;
		}

		// if label name does not terminated by semicolon //
		// ignore this line                               //
		if (':' != *cPos)
			continue;

		cPos++;

		// second field is label or comment address //
		while ((('0' <= *cPos) && ('9' >= *cPos)) ||
			   (('A' <= toupper(*cPos)) && ('F' >= toupper(*cPos)))) {

			unsigned int currDigit = toupper(*cPos);

			if (toupper(*cPos) <= '9') {
				currDigit = *cPos - '0';
			} else {
				currDigit = toupper(*cPos) - 'A' + 10;
			}

			labelAddress *= 16;
			labelAddress += currDigit;
			cPos++;
		}

		if (';' == *cPos) {
			// there is also comment for this label //
			while ((*cPos != '\n') && (*cPos != '\r') && (*cPos != '\0')) {
				comment += *cPos;
				cPos++;
				// check if this comment continues on next line //
				if (';' == *cPos) {
						comment += '\n';
						if (0 == labelName.length()) {
							// inline commemts require alignment //
							comment += '\t';
							for (unsigned int i = TAB_SIZE; i <  COMMENTS_COLUMN;
									i++)
								comment += ' ';
						}
				}
			}
		}

		// check for junk after label or comment address //
		if ((*cPos != '\n') && (*cPos != '\r') && (*cPos != '\0'))
			// junk detected, ignore line //
			continue;

		if (0 != labelName.length()) {
			if (labels.find(labelAddress) == labels.end())
				labels[labelAddress] = labelName;
		}

		if (0 != comment.length()) {

			if (0 != labelName.length()) {
				// comment for label //
				// left-aligned      //
				if (leftComments.find(labelAddress) == leftComments.end()) {
					leftComments[labelAddress] = comment;
				} else {
					leftComments[labelAddress] += '\n' + comment;
				}
			}

			if (rightComments.find(labelAddress) == rightComments.end()) {
				rightComments[labelAddress] = comment;
			} else {
				rightComments[labelAddress] += '\n' + comment;
			}
		}
	}
	fclose(hints);
	return true;
}

bool DebugInfo::storeInfo(const char *hintsFilename)
{
	FILE *hintsFile;
	std::map<unsigned int, std::string>::iterator labIter;

	hintsFile = fopen(hintsFilename, "w");
	if (NULL == hintsFile) {
		printf("Cannot write debug information to %s:\n%s\n",
			hintsFilename, strerror(errno));
		return false;
	}

	// dump labels and labels with comments //
	for (labIter = labels.begin(); labIter != labels.end(); labIter++) {
		char lineBuf[1024];
		unsigned int address = (*labIter).first;

		if (isLeftCommentForAddr(address)) {

			std::map<unsigned int, std::string>::iterator cmtIter;

			cmtIter = leftComments.find(address);

			snprintf(lineBuf, sizeof(lineBuf), "%s:%04x%s\n",
				(*labIter).second.c_str(), address, (*cmtIter).second.c_str());
		} else {
			snprintf(lineBuf, sizeof(lineBuf), "%s:%04x\n",
				(*labIter).second.c_str(), address);
		}
		fputs(lineBuf, hintsFile);
	}

	// dump left side comments that are not assigned to a label //
	std::map<unsigned int, std::string>::iterator cmtIter;
	for (cmtIter = leftComments.begin(); cmtIter != leftComments.end(); cmtIter++) {
		char lineBuf[1024];
		unsigned int address = (*cmtIter).first;

		if (isLabelForAddr(address))
			continue;

		snprintf(lineBuf, sizeof(lineBuf), ":%04x;%s\n",
			address, (*cmtIter).second.c_str());

		fputs(lineBuf, hintsFile);
	}

	for (cmtIter = rightComments.begin(); cmtIter != rightComments.end(); cmtIter++) {
		char lineBuf[1024];
		unsigned int address = (*cmtIter).first;

		snprintf(lineBuf, sizeof(lineBuf), ":%04x%s\n",
			address, (*cmtIter).second.c_str());

		fputs(lineBuf, hintsFile);
	}

	fclose(hintsFile);
	return true;
}

void DebugInfo::addLabel(string labelName, unsigned int address)
{
	labels[address] = labelName;
}

void DebugInfo::addLeftComment(string text, unsigned int address)
{
	leftComments[address] += text;
}

void DebugInfo::addRightComment(string text, unsigned int address)
{
	rightComments[address] += text;
}

bool DebugInfo::isLabelForAddr(unsigned int address)
{
	if (labels.find(address) == labels.end())
		return false;
	
	return true;
}

bool DebugInfo::isLeftCommentForAddr(unsigned int address)
{
	if (leftComments.find(address) == leftComments.end())
		return false;
	
	return true;
}

bool DebugInfo::isRightCommentForAddr(unsigned int address)
{
	if (rightComments.find(address) == rightComments.end())
		return false;
	
	return true;
}

string DebugInfo::getLabelName(unsigned int address)
{
	return labels[address];
}

string DebugInfo::getCommentText(unsigned int address)
{
	return labels[address];
}

bool DebugInfo::addBreakPoint(string labelName)
{
	map<unsigned int, string>::iterator labelItr;

	labelItr = labels.begin();
	while ((labelItr != labels.end()) &&
				(labelName != (*labelItr).second))
		   labelItr++;

	if (labelItr != labels.end()) {
		breakPoints.insert((*labelItr).first);
		bpNo++;
		return true;
	}

	return false;
}

bool DebugInfo::getLabelAddress(string labelName, unsigned int *outAddr)
{
	map<unsigned int, string>::iterator labelItr;

	labelItr = labels.begin();
	while ((labelItr != labels.end()) &&
				(labelName != (*labelItr).second))
		   labelItr++;

	if (labelItr != labels.end()) {
		if (NULL != outAddr)
			*outAddr = (*labelItr).first;
		return true;
	}

	return false;
}

void DebugInfo::removeBreakPoint(unsigned int address)
{
	set<unsigned int>::iterator bpEntry;
	bpEntry = breakPoints.find(address);
	if (breakPoints.end() == bpEntry)
		return;

	breakPoints.erase(bpEntry);
}

DebugInfo::~DebugInfo()
{
}

