#ifndef AMIGAUTILS_H_INCLUDED
#define AMIGAUTILS_H_INCLUDED

#include <string.h>

namespace amigadrive
{
	/*!
	*	stringStore provides a string persistence mechanism for allocated strings.
	*/
	class stringStore
	{
	protected:
		class stringNode
		{
			friend class stringStore;
			private:
				char *m_string;
				stringNode *m_next;

			stringNode(int len)
			{
				if (len > 0)
					m_string = new char [len];
				else
					m_string = nullptr;
				m_next = nullptr;
			}

			~stringNode()
			{
				if (m_next)
				{
					delete m_next;
					m_next = nullptr;
				}

				if (m_string)
				{
					delete m_string;
					m_string = nullptr;
				}
			}
		};

		stringNode *m_first;

	public:
		stringStore()
		{
			m_first = nullptr;
		}

		~stringStore()
		{
			if (m_first)
			{
				delete m_first;
				m_first = nullptr;
			}
		}

		/*
		* 	Returns a character buffer of the given length, which is persistent for
		*	the duration of the stringStore object.
		*/
		char *makeString(int len)
		{
			stringNode *O = m_first;
			stringNode *N = new stringNode(len);

			N->m_next = O;
			m_first = N;

			return N->m_string;
		}

		char *copyString(char *S, int len)
		{
		    char *T = makeString(len);
		    memcpy(T, S, len);

		    return T;
		}
	};
}

#endif // AMIGAUTILS_H_INCLUDED
