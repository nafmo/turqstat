#include "lexer.h"
#include <iostream>
#include <fstream>

using namespace std;

int main()
{
	cout << "opening..." << endl;
	fstream templ("templates/default.tpl", ios::in);
	if (!templ.is_open()) return 1;

	bool insettings = false;
	while (!templ.eof())
	{
		string line;
		getline(templ, line);

		cout << "<-- " << line << endl;

		bool iserror = false;
		Token *list = Token::Parse(line, insettings, iserror);
		if (iserror)
		{
			cout << "--> parse error\n";
		}
		else
		{
			cout << "--> ";
			if (!list)
			{
				cout << "empty";
			}

			Token *t = list;
			while (t)
			{
				if (t->IsLiteral())
				{
					cout << '"' << static_cast<Literal *>(t)->GetLiteral() << '"';
				}
				else if (t->IsSection())
				{
					insettings = false;
					cout << '[';
					switch (static_cast<Section *>(t)->GetSection())
					{
					case Section::Common: cout << "Common"; break;
					case Section::IfEmpty: cout << "IfEmpty"; break;
					case Section::IfNotNews: cout << "IfNotNews"; break;
					case Section::IfNews: cout << "IfNews"; break;
					case Section::Quoters: cout << "Quoters"; break;
					case Section::Writers: cout << "Writers"; break;
					case Section::TopNets: cout << "TopNets"; break;
					case Section::TopDomains: cout << "TopDomains"; break;
					case Section::Received: cout << "Received"; break;
					case Section::Subjects: cout << "Subjects"; break;
					case Section::Programs: cout << "Programs"; break;
					case Section::Week: cout << "Week"; break;
					case Section::Day: cout << "Day"; break;
					case Section::Localization: cout << "Localization"; insettings = true; break;
					default: cout << "??"; break;
					}
					cout << ']';
				}
				else if (t->IsVariable())
				{
					cout << "@{type="
					     << static_cast<int>(static_cast<Variable *>(t)->GetType())
					     << "}{width="
					     << static_cast<int>(static_cast<Variable *>(t)->GetWidth())
//					     << "}{lang="
//					     << static_cast<Variable *>(t)->GetLanguageToken()
					     << "}@";
				}

				t = t->Next();
				if (t)
				{
					cout << ',';
				}
			}
			cout << endl;
		}
		delete list;
	}

	return 0;
}
