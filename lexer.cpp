// Copyright (c) 2002-2007 Peter Karlsson
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

#include <stdlib.h>

#include "utility.h"
#include "lexer.h"

#include <iostream>

Token *Token::Parse(const string &line, bool &error)
{
    if (';' == line[0])
    {
        if (';' == line[1])
        {
std::cout << "DEBUG: DOUBLE SEMICOLON \"" << line << '"' << std::endl;
            // Double semi-colon means that line starts with a semi-colon
            return Parse2(line.substr(1), error);
        }
        else
        {
std::cout << "DEBUG: COMMENT \"" << line << '"' << std::endl;
            // Line is a comment
            return NULL;
        }
    }
    else if ('[' == line[0])
    {
        // Line is a section name
        string::size_type endbracket = line.find(']');
        if (string::npos == endbracket)
        {
std::cout << "DEBUG: MALFORMED SECTION \"" << line << '"' << std::endl;
            error = true;
            return NULL;
        }
        else
        {
std::cout << "DEBUG: SECTION \"" << line << '"' << std::endl;
            return new Section(line.substr(1, endbracket - 1), error);
        }
    }

    // Call real parser
    return Parse2(line, error);
}

Token *Token::Parse2(string line, bool &error)
{
std::cout << "DEBUG: Parse2(\"" << line << "\", bool error = " << error << ")" << std::endl;

    string::size_type pos = 0;
    Token *head = NULL;

    if ('@' == line[0])
    {
        // Find end of variable and parse it
        pos = line.find('@', 1);
        if (1 == pos)
        {
            // Literal @ sign.
            pos = line.find('@', 2);
            if (string::npos == pos)
            {
                head = new Literal(line.substr(1));
            }
            else
            {
                head = new Literal(line.substr(1, pos - 1));
            }
        }
        else if (string::npos == pos)
        {
            // Incorrect syntax
            error = true;
            return NULL;
        }
        else
        {
            // Real variable
            head = new Variable(line.substr(1, pos - 1), error);
            pos ++;
        }
    }
    else
    {
        // String literal up to next variable or end-of-line
        pos = line.find('@');
        if (string::npos == pos)
        {
            head = new Literal(line);
        }
        else
        {
            head = new Literal(line.substr(0, pos));
        }
    }

    if (pos != string::npos && pos < line.length())
    {
        head->m_next_p = Parse2(line.substr(pos), error);
    }

    return head;
}

Section::Section(string s, bool &error)
    : Token()
{
std::cout << "DEBUG: Section::Section(\"" << s << "\", bool error = " << error << ")" << std::endl;

    if (0 == fcompare(s, "Common"))          m_section = Common;
    else if (0 == fcompare(s, "IfEmpty"))    m_section = IfEmpty;
    else if (0 == fcompare(s, "IfNotNews"))  m_section = IfNotNews;
    else if (0 == fcompare(s, "IfNews"))     m_section = IfNews;
    else if (0 == fcompare(s, "Original"))   m_section = Original;
    else if (0 == fcompare(s, "Quoters"))    m_section = Quoters;
    else if (0 == fcompare(s, "Writers"))    m_section = Writers;
    else if (0 == fcompare(s, "TopNets"))    m_section = TopNets;
    else if (0 == fcompare(s, "TopDomains")) m_section = TopDomains;
    else if (0 == fcompare(s, "Received"))   m_section = Received;
    else if (0 == fcompare(s, "Subjects"))   m_section = Subjects;
    else if (0 == fcompare(s, "Programs"))   m_section = Programs;
    else if (0 == fcompare(s, "Week"))       m_section = Week;
    else if (0 == fcompare(s, "Day"))        m_section = Day;
    else error = true;

std::cout << "  ==> section == " << (int) m_section << "; error = " << error << std::endl;
}

Variable::Variable(string s, bool &error)
    : Token(),
      m_width(0)
{
std::cout << "DEBUG: Variable::Variable(\"" << s << "\", bool error = " << error << ")" << std::endl;

    string::size_type bracket1 = s.find('[');
    string::size_type bracket2 = s.find(']', bracket1 + 1);
    string::size_type namelen = bracket1;

    while (bracket1 != string::npos && bracket2 != string::npos &&
           bracket2 > bracket1 + 1)
    {
        // Valid indexing
        if (isdigit(s[bracket1 + 1]))
        {
            SetWidth(s.substr(bracket1 + 1, bracket2 - bracket1 - 1));
        }
        else
        {
            error = true;
        }

        // Find next bracket, if any
        bracket1 = s.find('[', bracket2 + 1);
        bracket2 = s.find(']', bracket1 + 1);
    }

    if (namelen != string::npos)
    {
        SetVariable(s.substr(0, namelen), error);
    }
    else
    {
        SetVariable(s, error);
    }

std::cout << "  ==> type = " << m_type << ", width = " << m_width << ", error = " << error << ")" << std::endl;
}

void Variable::SetWidth(string s)
{
std::cout << " Variable::SetWidth(\"" << s << "\")" << std::endl;
    m_width = atoi(s.c_str());
}

void Variable::SetVariable(string s, bool &error)
{
std::cout << " Variable::SetVariable(\"" << s << "\")" << std::endl;
    if (0 == fcompare(s, "Version"))                 m_type = Version;
    else if (0 == fcompare(s, "Copyright"))          m_type = Copyright;
    else if (0 == fcompare(s, "IfReceived"))         m_type = IfReceived;
    else if (0 == fcompare(s, "IfAreas"))            m_type = IfAreas;
    else if (0 == fcompare(s, "Place"))              m_type = Place;
    else if (0 == fcompare(s, "Name"))               m_type = Name;
    else if (0 == fcompare(s, "Written"))            m_type = Written;
    else if (0 == fcompare(s, "BytesWritten"))       m_type = BytesWritten;
    else if (0 == fcompare(s, "Ratio"))              m_type = Ratio;
    else if (0 == fcompare(s, "BytesTotal"))         m_type = BytesTotal;
    else if (0 == fcompare(s, "BytesQuoted"))        m_type = BytesQuoted;
    else if (0 == fcompare(s, "BytesQuotedPercent")) m_type = BytesQuotedPercent;
    else if (0 == fcompare(s, "TotalMessages"))      m_type = TotalMessages;
    else if (0 == fcompare(s, "TotalAreas"))         m_type = TotalAreas;
    else if (0 == fcompare(s, "TotalPeople"))        m_type = TotalPeople;
    else if (0 == fcompare(s, "TotalNets"))          m_type = TotalNets;
    else if (0 == fcompare(s, "TotalDomains"))       m_type = TotalDomains;
    else if (0 == fcompare(s, "TotalSubjects"))      m_type = TotalSubjects;
    else if (0 == fcompare(s, "TotalPrograms"))      m_type = TotalPrograms;
    else if (0 == fcompare(s, "EarliestReceived"))   m_type = EarliestReceived;
    else if (0 == fcompare(s, "LastReceived"))       m_type = LastReceived;
    else if (0 == fcompare(s, "EarliestWritten"))    m_type = EarliestWritten;
    else if (0 == fcompare(s, "LastWritten"))        m_type = LastWritten;
    else if (0 == fcompare(s, "BytesOriginal"))      m_type = BytesOriginal;
    else if (0 == fcompare(s, "PerMessage"))         m_type = PerMessage;
    else if (0 == fcompare(s, "Fidonet"))            m_type = Fidonet;
    else if (0 == fcompare(s, "TopDomain"))          m_type = TopDomain;
    else if (0 == fcompare(s, "Received"))           m_type = Received;
    else if (0 == fcompare(s, "ReceiveRatio"))       m_type = ReceiveRatio;
    else if (0 == fcompare(s, "Subject"))            m_type = Subject;
    else if (0 == fcompare(s, "Program"))            m_type = Program;
    else if (0 == fcompare(s, "Day"))                m_type = Day;
    else if (0 == fcompare(s, "Bar"))                m_type = Bar;
    else if (0 == fcompare(s, "Hour"))               m_type = Hour;
    else error = true;
}
