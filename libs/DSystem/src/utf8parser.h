//==================================================================
/// utf8parser.h
///
/// Created by Duncan Tebbs
/// See the file "license.txt" that comes with this project for
/// copyright info.
//==================================================================

#ifndef UTF8PARSER_H
#define UTF8PARSER_H

#include <stdint.h>
#include "DAssert.h"

class UTF8Parser
{
public:
    UTF8Parser(const uint8_t *utf8);
    uint32_t GetNextCharacter();

	const uint8_t *GetCurrent() const;

protected:
    const uint8_t *m_current;
};

inline
UTF8Parser::UTF8Parser(const uint8_t *utf8) :
    m_current(utf8)
{
    DASSERT(0 != utf8);
}

inline uint32_t
UTF8Parser::GetNextCharacter()
{
    if (0 == *m_current)
    {
        return 0;
    }

    uint32_t next = (uint32_t )(*m_current);
    ++m_current;

    if (0 == (0x80 & next))   // 1 byte
    {
        return (uint32_t )next;
    }

    uint32_t _next = (uint32_t )*m_current;
    ++m_current;

    DASSERT(0xC0 == (0xC0 &  next));
    DASSERT(0x80 == (0xC0 & _next));

    if (0 == (0x20 & next))   // 2 byte  (110x xxxx) (10xx xxx)
    {
        return ((next & 0x1F) << 6) | (_next & 0x3F);
    }

    uint32_t __next = (uint32_t )*m_current;
    ++m_current;

    DASSERT(0x20 == (0x20 &   next));
    DASSERT(0x80 == (0xC0 & __next));

    if (0 == (0x10 & next))   // 3 byte  (1110 xxxx) (10xx xxxx) (10xx xxxx)
    {
        return ((next & 0x0F) << 12) |
            ((_next & 0x3f) << 6) |
            (__next & 0x3f);
    }

    DASSERT(false);

	// DAVIDE: forcing a return for all control paths
	return 0;
}

inline const uint8_t
*UTF8Parser::GetCurrent() const
{
	return m_current;
}

#endif
