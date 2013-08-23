#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#define BAR_PRINT_PROTOTYPE 1

#include "amigaui.h"

namespace amigadrive
{
	ConsoleUI::ConsoleUI()
	{
		m_listOfStrings = new stringStore();
	}

	ConsoleUI::~ConsoleUI()
	{
		if (m_listOfStrings)
		{
			delete m_listOfStrings;
			m_listOfStrings = nullptr;
		}
	}

	void ConsoleUI::progressBar(int percent)
	{
		static int lastcount=0;

		assert( percent > -1 && percent < 101);

		if (percent > lastcount)
		{
			fputc('.', stdout);
			lastcount = percent;
		}
	}

	void ConsoleUI::textInfo(const char *format, ...)
	{
		char *txtBuffer = m_listOfStrings->makeString(1024);
		va_list ap;

		va_start(ap, format);
		vsnprintf(txtBuffer, 1024, format, ap);
		fputs(txtBuffer, stdout);
		va_end(ap);
	}

	void ConsoleUI::textWarning(const char *format, ...)
	{
		char *txtBuffer = m_listOfStrings->makeString(1024);
		va_list ap;

		va_start(ap, format);
		vsnprintf(txtBuffer, 1024, format, ap);
		fputs(txtBuffer, stderr);
		va_end(ap);
	}

	void ConsoleUI::textError(const char *format, ...)
	{
		char *txtBuffer = m_listOfStrings->makeString(1024);
		va_list ap;

		va_start(ap, format);
		vsnprintf(txtBuffer, 1024, format, ap);
		fputs(txtBuffer, stderr);
		va_end(ap);
	}
}
