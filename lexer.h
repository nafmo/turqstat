// Copyright (c) 2002 Peter Karlsson
//
// $Id$
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

#include <string>

/**
 * Linked list of lexical token. This class describes a linked list of tokens
 * returned from the lexical analyzer, which is run on the template files.
 * This class will never be instantiated by itself, only subclasses of it will
 * ever be instantiated.
 */
class Token
{
public:
    /** Standard destructor. This will destruct the entire linked list pointed
      * to by this node. Must only be called on the first node in the list.
      */
    virtual ~Token() { delete m_next_p; };

    /** Poor man's RTTI. Check if this is a literal string token. */
    virtual bool IsLiteral() const { return false; }
    /** Poor man's RTTI. Check if this is a section token. */
    virtual bool IsSection() const { return false; }
    /** Poor man's RTTI. Check if this is a variable token. */
    virtual bool IsVariable() const { return false; }

    /** Retrieve pointer to next token in the list. */
    Token *Next() const { return m_next_p; }

    /** Parse a line. To parse a template line, this method is called.
      *
      * @param line A line of the template file to parse.
      * @param error Set to true if a parsing error occurs.
      * @return A linked list of tokens describing this line.
      */
    static Token *Parse(const string &line, bool &error);

protected:
    /** Standard constructor. Not accessible to any class, except for its
      * subclasses, to prevent it from being misused, as this class should
      * never be instantiated directly. */
    Token() : m_next_p(NULL) {};

private:
    /** Pointer to next token in the list. */
    Token *m_next_p;

    /** Helper method for parser. @see Parse */
    static Token *Parse2(const string line, bool &error);
};

/** Lexical token describing a literal string. */
class Literal : public Token
{
public:
    /** Poor man's RTTI. Check if this is a literal string token. */
    virtual bool IsLiteral() const { return true; }
    /** Retrieve the string literal described by this token. */
    string GetLiteral() const { return m_literal; }

private:
    friend class Token;
    /** Constructor. Initialize the string literal. */
    Literal(string s) : Token(), m_literal(s) {}

    /** String literal described by this token. */
    string m_literal;    
};

/** Lexical token describing a report section. */
class Section : public Token
{
public:
    /** List of sections known. */
    enum sectiontype
    {
        // Common sections
        Common, IfEmpty, IfNotNews, IfNews,
        // Toplist sections
        Quoters, Writers, TopNets, TopDomains, Received, Subjects, Programs,
        Week, Day
    };

    /** Poor man's RTTI. Check if this is a section token. */
    virtual bool IsSection() const { return true; }
    /** Retrieve report section described by this token. */
    sectiontype GetSection() const { return m_section; }

private:
    friend class Token;
    /** Constructor. Initialize the section from the string describing it. */
    Section(string s, bool &error);

    /** Report section described by this token. */
    sectiontype m_section;
};

class Variable : public Token
{
public:
    /** List of variable types known. */
    enum variabletype
    {
        Totals, Place, Name, Written, BytesWritten, Ratio, BytesTotal,
        BytesQuoted, BytesQuotedPercent, TotalPeople, BytesOriginal,
        PerMessage, Fidonet, TopDomain, Received, ReceiveRatio, Subject,
        Program, Day, Bar, Hour
    };

    /** Poor man's RTTI. Check if this is a variable token. */
    virtual bool IsVariable() const { return true; }
    /** Retrieve variable type described by this token. */
    variabletype GetType() const { return m_type; }
    /** Retrieve display width set for this token, or zero if none. */
    size_t GetWidth() const { return m_width; }
    /** Retrieve language set for this token, or empty string if none. */
    string GetLanguageToken() const { return m_languagetoken; }

private:
    friend class Token;
    /** Constructor. Initialize the variable from the string describing it. */
    Variable(string s, bool &error);
    /** Helper method to the constructor. */
    void Variable::SetWidth(string);
    /** Helper method to the constructor. */
    void Variable::SetLanguage(string);
    /** Helper method to the constructor. */
    void Variable::SetVariable(string, bool &error);

    /** Variable described by this token. */
    variabletype m_type;
    /** Display width set for this token. */
    int m_width;
    /** Language set for this token. */
    string m_languagetoken;
};
