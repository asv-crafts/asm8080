#if !defined(EMU_DEBUGINFO)
#define EMU_DEBUGINFO

#include <string>
#include <map>
#include <set>

using std::string;
using std::map;
using std::set;

class DebugInfo {

public:
	DebugInfo();
	bool loadInfo(const char *filename);
	bool storeInfo(const char *filename);
	void addLabel(string labelName, unsigned int address);
	void addLeftComment(string text, unsigned int address);
	void addRightComment(string text, unsigned int address);
	void addBreakPoint(unsigned int address)
		{ breakPoints.insert(address); bpNo++; }
	bool addBreakPoint(string labelName);
	void removeBreakPoint(unsigned int address);
	bool getLabelAddress(string labelName, unsigned int *outAddr);
	bool isLabelForAddr(unsigned int address);
	bool isLeftCommentForAddr(unsigned int address);
	bool isRightCommentForAddr(unsigned int address);
	inline bool isAnyBreakpoint() { return bpNo; }
	inline bool isBreakPointForAddr(unsigned int address) {
		if (0 == bpNo)
			return false;
		return (breakPoints.find(address) != breakPoints.end());
	};
	string getLabelName(unsigned int address);
	string getCommentText(unsigned int address);
	~DebugInfo();

private:
	map<unsigned int, string> labels;
	map<unsigned int, string> leftComments;
	map<unsigned int, string> rightComments;
	set<unsigned int> breakPoints;
	int bpNo;
};

#endif
