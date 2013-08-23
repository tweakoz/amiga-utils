#ifndef EXCEPTION_H_INCLUDED
#define EXCEPTION_H_INCLUDED

#include <assert.h>

namespace amigadrive
{
	/*!
	*	\addtogroup Exceptions
	*
	*	A class defining a simple exception error reporting mechanism.
	*/
	class Exception
	{
		private:
			UI *m_messenger;
			const char *m_text;

		public:
		Exception(UI *messenger, const char *text)
		{
			assert(messenger);
			assert(text);
			m_messenger = messenger;
			m_text = text;
		}

		void textMsg(void)
		{
			m_messenger->textError(m_text);
		}
	};
}

#endif // EXCEPTION_H_INCLUDED
