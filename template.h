// Copyright (c) 2007 Peter Karlsson
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#ifndef __TEMPLATE_H
#define __TEMPLATE_H

#if !defined(HAVE_IMPLICIT_NAMESPACE)
using namespace std;
#endif

#include <string>
#include "lexer.h"

class Token;

/**
 * Linked list of lines representing a template file. This class describes
 * the list of lines, using the Token class, which is tokenized from a
 * template file.
 */
class Template
{
public:
    /** Standard destructor. This will destruct the entire linked list pointed
      * to by this node. Must only be called on the first node in the list.
      */
    ~Template() { delete m_token_p; delete m_next_p; };

    /** Retrieve the current tokenized line. */
    inline Token *Line() const { return m_token_p; };

    /** Retrieve pointer to the next token in the list. */
    inline Template *Next() const { return m_next_p; };

    /** Parse a template. To parse a template file, this method is called.
      *
      * @param file The name of the file to parse.
      * @param error
      *   Set to true if a parsing error occurs, or the file could not be
      *   found.
      * @return A linked list of template lines describing this file.
      */
    static Template *Parse(const string &file, bool &error);

private:
    /** Pointer to the next line in the list. */
    Template *m_next_p;

    /** Pointer to the first token on the line. */
    Token *m_token_p;

    /** Constructor. */
    Template(Token *token_p)
        : m_next_p(NULL)
        , m_token_p(token_p)
        {};
};

#endif
