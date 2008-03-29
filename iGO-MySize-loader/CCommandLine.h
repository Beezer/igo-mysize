      
// CCommandLine.h

// By Johan

#include "stdafx.h"

class CCommandLine
{
protected:
	char *m_CommandLine;

	char *m_EntryValue;
	
public:
	CCommandLine();
	~CCommandLine();

	void Initiate(const char *CmdLine);

	char *FindEntryc(const char *Entry);
	int FindEntryi(const char *Entry);
	float FindEntryf(const char *Entry);

	void Free(void);
};
  
