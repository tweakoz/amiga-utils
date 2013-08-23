#ifndef AMIGAUI_H_INCLUDED
#define AMIGAUI_H_INCLUDED

#include "amigautils.h"

namespace amigadrive
{
	/*!
	*	Abstract base class defining the user interaction code.
	*/
	class UI
	{
		public:
			UI() {;};
			~UI() {;};

			/*!
			*	Call this member to output error messages formatted in the style of 'printf'.
			*
			*	\param format - a format string - see 'printf'.
			*	\param ... - 0-N additional arguments corresponding to the format string specification.
			*/
			virtual void textError(const char *format, ...) = 0;

			/*!
			*	Call this member to output warning messages formatted in the style of 'printf'.
			*
			*	\param format - a format string - see 'printf'.
			*	\param ... - 0-N additional arguments corresponding to the format string specification.
			*/
			virtual void textWarning(const char *format, ...) = 0;

			/*!
			*	Call this member to output info messages formatted in the style of 'printf'.
			*
			*	\param format - a format string - see 'printf'.
			*	\param ... - 0-N additional arguments corresponding to the format string specification.
			*/
			virtual void textInfo(const char *format, ...) = 0;

			/*!
			* 	Display progress.
			*
			*	\param percent - integer containing the percent progress
			*/
			virtual void progressBar(int percent) = 0;
	};

	/*!
	*	Implementation class for console message output.
	*/
	class ConsoleUI: public UI
	{
		private:
			stringStore *m_listOfStrings;

		public:
			ConsoleUI();
			~ConsoleUI();

			/*!
			*	A progress bar function - this implementation merely writes a , o the screen for
			*	every percent increment.
			*
			*	\param percent - percentage progress.
			*/
			virtual void progressBar(int percent);

			/*!
			*	Call this member to output error messages formatted in the style of 'printf'.
			*
			*	\param format - a format string - see 'printf'.
			*	\param ... - 0-N additional arguments corresponding to the format string specification.
			*/
			virtual void textError(const char *format, ...);

			/*!
			*	Call this member to output warning messages formatted in the style of 'printf'.
			*
			*	\param format - a format string - see 'printf'.
			*	\param ... - 0-N additional arguments corresponding to the format string specification.
			*/
			virtual void textWarning(const char *format, ...);

			/*!
			*	Call this member to output info messages formatted in the style of 'printf'.
			*
			*	\param format - a format string - see 'printf'.
			*	\param ... - 0-N additional arguments corresponding to the format string specification.
			*/
			virtual void textInfo(const char *format, ...);
	};
};

#endif // AMIGAUI_H_INCLUDED
