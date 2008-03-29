// CCommandLine.cpp

// By Johan
#include "stdafx.h"
#include "CCommandLine.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>


#define MAX_CMDLENGTH 256
#define MAX_ENTRYLENGTH 10

CCommandLine::CCommandLine()
{
	m_CommandLine = NULL;
	m_EntryValue = NULL;
}

CCommandLine::~CCommandLine()
{
	Free();
}

void CCommandLine::Free()
{
	m_CommandLine = NULL;
	delete [] m_CommandLine;

	m_EntryValue = NULL;
	delete [] m_EntryValue;
}

void CCommandLine::Initiate(const char *CmdLine)
{
	m_CommandLine = new char[MAX_CMDLENGTH];
	m_EntryValue = new char[MAX_ENTRYLENGTH];

	strcpy(m_CommandLine, CmdLine);
}

char *CCommandLine::FindEntryc(const char *Entry)
{
	char *pEntry;

	if (Entry == NULL || m_CommandLine == NULL)
		return NULL;
	
	// search for entry in command line

	if ((pEntry = strstr(m_CommandLine, Entry)) == NULL)
		return NULL;

	// <pEntry> now points to the beginning of <Entry>

	// move pointer forward until '=' character

	while (*pEntry != '=')
		*pEntry++;

	// <pEntry> now points to the '=' character

	// move pointer forward one step to the next character

	*pEntry++;

	int steps = 0;

	// <pEntry> now points to the beginning of the <entry value>

	// find next blank space

	for (; ; *pEntry++, steps++)
	{
		// break on EOF

		if (*pEntry == '\0')
			break;
		// break on blank space

		else if (isspace(*pEntry))
			break;
	}

	// <pEntry> to points to a blank space

	// move back pointer to the '=' character

	while (*pEntry != '=')
		*pEntry--;

	// <pEntry> now points to the '=' character

	// move pointer forward one step to the next character

	*pEntry++;

	// copy <pEntry> and <steps> characters forward

	strncpy(m_EntryValue, pEntry, steps);
	m_EntryValue[steps] = '\0';

	return m_EntryValue;
}

int CCommandLine::FindEntryi(const char *Entry)
{
	char *entry;

	if ((entry = FindEntryc(Entry)) == NULL)
		return NULL;

	return atoi(entry);
}

float CCommandLine::FindEntryf(const char *Entry)
{
	char *entry;

	if ((entry = FindEntryc(Entry)) == NULL)
		return NULL;

	return atof(entry);
}
  
