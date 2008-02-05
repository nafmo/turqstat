// Copyright (c) 1998-2008 Peter Karlsson
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

#include <config.h>
#ifdef HAVE_FSTREAM
# include <fstream>
#else
# include <fstream.h>
#endif
#include <limits.h>
#include <time.h>
#include <ios>
#include <iomanip>
#include <sstream>

#include "statview.h"
#include "statengine.h"
#include "utility.h"
#include "version.h"
#include "template.h"
#include "lexer.h"
#include "output.h"

static const char *days[] =
    { "Monday   ",
      "Tuesday  ",
      "Wednesday",
      "Thursday ",
      "Friday   ",
      "Saturday ",
      "Sunday   " };

StatView::StatView()
{
    // Initialize all values
    quoters = false;
    topwritten = false;
    topreceived = false;
    toporiginal = false;
    topnets = false;
    topsubjects = false;
    topprograms = false;
    topdomains = false;
    weekstats = false;
    daystats = false;

    showversions = false;
    showallnums = false;
    maxnumber = 15;

    m_template = NULL;
}

bool StatView::CreateReport(StatEngine *engine, string filename)
{
    // If we give 0 as maximum entries, we want unlimited (=UINT_MAX...)
    if (0 == maxnumber) maxnumber = UINT_MAX;

    // Create a report file
    fstream report(filename.c_str(), ios::out);
    if (!(report.is_open())) return false;

    // Create an output text encoder
    Encoder *encoder_p = Encoder::GetEncoderByName(charset.c_str());

    // Include data as given from the statistics engine
    bool news = engine->IsNewsArea();

    // Loop over template, outputting the requested sections
    const Template *current_line_p = m_template;
    bool include_section = false;
    bool quit_after_section = false;
    bool area_is_empty = 0 == engine->GetTotalNumber();
	Section::sectiontype current_section = Section::Common;

	// Default to English day names
	for (int i = 0; i < 7; ++ i)
	{
		m_dayname[i] = days[i];
	}

	// Remember toplist position, non-zero for looped lines
	unsigned int place = 0;
	unsigned int toplist_length = 0;
	stringstream reportline;
	while (current_line_p)
    {
		StatEngine::persstat_s current_person;
		StatEngine::netstat_s current_net;
		StatEngine::domainstat_s current_domain;
		StatEngine::subjstat_s current_subject;
		StatEngine::progstat_s current_program;
		unsigned int current_day_or_hour = UINT_MAX;
		unsigned int maxposts = 1; // Max posts for week/daystat bars

        // Possibly repeat the current line
		do
		{
			bool have_linebreak = false;

			// Loop over the tokens on this line
			const Token *current_token_p = current_line_p->Line();
			while (current_token_p)
			{
				if (current_token_p->IsSection())
				{
					// Stop processing if requested
					if (quit_after_section)
					{
						current_line_p = NULL;
						break;
					}

					// Get section number
					const Section *section_p =
						static_cast<const Section *>(current_token_p);
					current_section = section_p->GetSection();

					// If area is empty, we quit after the first non-"Common",
					// non-"IfEmpty" section.
					if (area_is_empty &&
					   (current_section != Section::Common &&
					    current_section != Section::IfEmpty))
					{
						quit_after_section = true;
					}

					// Check if we should include this section or not. We always
					// include Common sections.
					switch (current_section)
					{
					case Section::Common:		include_section = true; break;
					case Section::IfEmpty:		include_section = area_is_empty; break;
					case Section::IfNotNews:	include_section = !news; break;
					case Section::IfNews:		include_section = news; break;
					case Section::Original:		include_section = toporiginal; break;
					case Section::Quoters:		include_section = quoters; break;
					case Section::Writers:		include_section = topwritten; break;
					case Section::TopNets:		include_section = topnets && !news; break;
					case Section::TopDomains:	include_section = topdomains && (news || engine->GetTotalDomains()); break;
					case Section::Received:		include_section = topreceived && !news; break;
					case Section::Subjects:		include_section = topsubjects; break;
					case Section::Programs:		include_section = topprograms; break;
					case Section::Week:			include_section = weekstats; break;
					case Section::Day:			include_section = daystats; break;
					case Section::Localization:	include_section = true; break;
					default:
						// Unknown section.
						TDisplay::GetOutputObject()->InternalErrorQuit(TDisplay::program_halted, 1);
					}
                }
                else if (include_section && !current_token_p->IsSection())
                {
					// This line contains information we should output.

					if (current_token_p->IsLiteral())
					{
						// Get literal string
						const Literal *literal_p =
							static_cast<const Literal *>(current_token_p);
						reportline << literal_p->GetLiteral();
						if (literal_p->HasLineBreak())
						{
							have_linebreak = true;
						}
					}
					else if (current_token_p->IsVariable())
					{
						// If we encounter a "place" variable, we start
						// counting the toplist position. If place is
						// non-zero, we will continue outputting this
						// line until we have output enough entries.
						// Most of the other variables depend on the
						// current section context.
						const Variable *variable_p =
							static_cast<const Variable *>(current_token_p);

						// Default to unknown data, left justification and given width
						stringstream data;
						bool known_data = false;

						// Default to entry not being identical to last
						bool identical_entry = false;

						reportline << left;
						streamsize width = variable_p->GetWidth();
						if (width > 0)
						{
							reportline << setw(width);
						}

						// Extract data.
						bool end_iteration = false; // End iteration now.
						bool next_iteration = false; // Skip to next iteration.
						switch (variable_p->GetType())
						{
						case Variable::Version:
							known_data = true;
							data << version;
							break;

						case Variable::Copyright:
							known_data = true;
							data << copyright;
							break;

						case Variable::IfReceived:
							if (!engine->HasArrivalTime())
							{
								// Break out of current line if there is no
								// arrival time information available.
								current_token_p = NULL;
								data << "";
							}
							known_data = true;
							break;

						case Variable::IfAreas:
							if (engine->GetTotalAreas() <= 1)
							{
								// Break out of current line if there is
								// only one (or no) areas scanned.
								current_token_p = NULL;
								data << "";
							}
							known_data = true;
							break;

						case Variable::Place:
							// Start counting the top-list position.
							// Triggers repetition.
							++ place;
							data << place << '.';
							reportline << right;
							toplist_length = maxnumber;
							known_data = true;

							// Fetch the appropriate data record
							switch (current_section)
							{
							case Section::Original:
								{
									// Cache
									unsigned long bytesoriginal =
										static_cast<unsigned long>(-1);
									unsigned long messageswritten =
										static_cast<unsigned long>(-1);

									bool first = 1 == place;
									if (!first)
									{
										bytesoriginal =
											current_person.byteswritten -
											current_person.bytesquoted;
										messageswritten =
											current_person.messageswritten;
									}

									// Constraint: Only people having posted
									// at least three messages are considered.
									do
									{
										end_iteration =
											!engine->GetTopOriginalPerMsg(first, current_person);
										first = false;
									} while (!end_iteration && current_person.messageswritten < 3);

									// Compare ratio (bytesoriginal / messageswritten)
									identical_entry =
										(bytesoriginal * current_person.messageswritten) ==
										(current_person.byteswritten - current_person.bytesquoted) * messageswritten;
								}
								break;

							case Section::Quoters:
								{
									// Cache
									unsigned long bytesquoted =
										static_cast<unsigned long>(-1);
									unsigned long byteswritten =
										static_cast<unsigned long>(-1);

									bool first = 1 == place;
									if (!first)
									{
										bytesquoted =
											current_person.bytesquoted;
										byteswritten =
											current_person.byteswritten;
									}

									// Constraint: Only people having quoted
									// and having posted at least three
									// messages are listed.
									do
									{
										end_iteration =
												!engine->GetTopQuoters(first, current_person);
										first = false;
									} while (!end_iteration &&
									         (current_person.messageswritten < 3 || current_person.bytesquoted <= 0));

									// Compare ratio (bytesquoted / byteswritten)
									identical_entry =
										(bytesquoted * current_person.byteswritten) ==
										(current_person.bytesquoted * byteswritten);
								}
								break;

							case Section::Writers:
								{
									// Cache
									unsigned long messageswritten =
										static_cast<unsigned long>(-1);

									bool first = 1 == place;
									if (!first)
									{
										messageswritten =
											current_person.messageswritten;
									}

									// Constraint: Must have written at least
									// one message.
									do
									{
										end_iteration =
											!engine->GetTopWriters(first, current_person);
										first = false;
									} while (!end_iteration && current_person.messageswritten == 0);

									// Compare messages written
									identical_entry =
										messageswritten == current_person.messageswritten;
								}
								break;

							case Section::TopNets:
								{
									unsigned long messages =
										static_cast<unsigned long>(-1);

									bool first = 1 == place;
									if (!first)
									{
										messages = current_net.messages;
									}

									end_iteration =
										!engine->GetTopNets(first, current_net);

									identical_entry =
										messages == current_net.messages;
								}
								break;

							case Section::TopDomains:
								{
									unsigned long messages =
										static_cast<unsigned long>(-1);

									bool first = 1 == place;
									if (!first)
									{
										messages = current_domain.messages;
									}

									end_iteration =
										!engine->GetTopDomains(first, current_domain);

									identical_entry =
										messages == current_domain.messages;
								}
								break;

							case Section::Received:
								{
									unsigned long messagesreceived =
										static_cast<unsigned long>(-1);

									bool first = 1 == place;
									if (!first)
									{
										messagesreceived =
											current_person.messagesreceived;
									}

									end_iteration =
										!engine->GetTopReceivers(first, current_person);

									identical_entry =
										messagesreceived == current_person.messagesreceived;
								}
								break;

							case Section::Subjects:
								{
									unsigned long count =
										static_cast<unsigned long>(-1);

									bool first = 1 == place;
									if (!first)
									{
										count = current_subject.count;
									}

									end_iteration =
										!engine->GetTopSubjects(first, current_subject);

									identical_entry =
										count == current_subject.count;
								}
								break;

							case Section::Programs:
								// FIXME: In the old format, this toplist
								// included version information. This is not
								// in the templatized form. See below.
								{
									unsigned long count =
										static_cast<unsigned long>(-1);

									bool first = 1 == place;
									if (!first)
									{
										count = current_program.count;
									}

									end_iteration =
										!engine->GetTopPrograms(1 == place, current_program);

									identical_entry =
										count == current_program.count;
								}
								break;

							case Section::Week:
								// This toplist is always seven entries
								toplist_length = 7;
								reportline << left;

								// Replace place number with name of day
								data.str(string());
								data << m_dayname[place - 1];

								current_day_or_hour =
									engine->GetDayMsgs(place % 7);

								// Find max width
								if (1 == place)
								{
									maxposts = engine->GetDayMsgs(0);
									for (int i = 1; i < 7; ++ i)
									{
										unsigned int posts = engine->GetDayMsgs(i);
										if (posts > maxposts)
										{
											maxposts = posts;
										}
									}
								}

								if (!showallnums && !current_day_or_hour)
								{
									next_iteration = true;
								}
								break;

							case Section::Day:
								// This toplist is always 24 entries
								toplist_length = 24;
								reportline << left;

								// Replace place number with hour string
								data.str(string());
								data << setfill('0') << setw(2) << (place - 1)
								     << "00-" << setw(2) << (place - 1) << "59";

								current_day_or_hour =
									engine->GetHourMsgs(place - 1);

								// Find max width
								if (1 == place)
								{
									maxposts = current_day_or_hour;
									for (int i = 1; i < 24; ++ i)
									{
										unsigned int posts = engine->GetHourMsgs(i);
										if (posts > maxposts)
										{
											maxposts = posts;
										}
									}
								}

								if (!showallnums && !current_day_or_hour)
								{
									next_iteration = true;
								}
								break;

							default:
								// Section without a toplist.
								end_iteration = true;
								break;
							}

							// Clear token if entry is identical to last
							if (identical_entry && !showallnums && place > 1)
							{
								data.str(string());
							}
							break;

						case Variable::Name:
							// Name of person.
							if (current_person.name.length())
							{
								string name =
									encoder_p->Encode(current_person.name);
								if (name != current_person.address &&
								    current_section != Section::Received)
								{
									data << name << " <" <<
									        current_person.address << ">";
								}
								else
								{
									data << name;
								}
							}
							known_data = true;
							break;

						case Variable::Written:
							// Number of messages written.
							switch (current_section)
							{
							case Section::Original:
							case Section::Quoters:
							case Section::Writers:
							case Section::Received:
								data << current_person.messageswritten;
								known_data = true;
								break;

							case Section::TopNets:
								data << current_net.messages;
								known_data = true;
								break;

							case Section::TopDomains:
								data << current_domain.messages;
								known_data = true;
								break;

							case Section::Subjects:
								data << current_subject.count;
								known_data = true;
								break;

							case Section::Programs:
								data << current_program.count;
								known_data = true;
								break;

							case Section::Week:
							case Section::Day:
								if (current_day_or_hour != UINT_MAX)
								{
									data << current_day_or_hour;
								}
								known_data = true;
								break;

							default:
								// No top-list, return totals
								data << engine->GetTotalNumber();
								known_data = true;
								break;
							}
							reportline << right;
							break;

						case Variable::BytesWritten:
							// Number of bytes written.
							switch (current_section)
							{
							case Section::Original:
							case Section::Quoters:
							case Section::Writers:
							case Section::Received:
								data << current_person.byteswritten;
								known_data = true;
								break;

							case Section::TopNets:
								data << current_net.bytes;
								known_data = true;
								break;

							case Section::TopDomains:
								data << current_domain.bytes;
								known_data = true;
								break;

							case Section::Subjects:
								data << current_subject.bytes;
								known_data = true;
								break;

							default:
								break;
							}
							reportline << right;
							break;

						case Variable::Ratio:
							// Ratio between bytes written and bytes quoted.
							{
								// Total bytes written (person or total)
								unsigned long long totalwrittenbytes =
									(place > 0)
									? current_person.byteswritten
									: engine->GetTotalBytes();

								// Total bytes quoted (person or total)
								unsigned long long totalquotedbytes =
									(place > 0)
									? current_person.bytesquoted
									: engine->GetTotalQBytes();

								if (totalquotedbytes)
								{
									// Percent x 1000
									unsigned long long percentquotes =
										totalwrittenbytes
										? ((100000 * totalquotedbytes) / totalwrittenbytes)
										: 0;

									// Calculate integer and fraction percent
									unsigned long long integ = percentquotes / 1000;
									unsigned long long fract =
										(percentquotes - integ * 1000 + 5) / 10;
									if (100 == fract)
									{
										fract = 0;
										integ ++;
									}

									// Percent string
									data << integ << '.'
									     << setfill('0') << setw(2) << fract << '%';
								}
								else
								{
									data << "";
								}
							}
							reportline << right;
							known_data = true;
							break;

						case Variable::BytesTotal:
							if (place > 0)
							{
								data << current_person.byteswritten;
							}
							else
							{
								data << engine->GetTotalBytes();
							}
							known_data = true;
							break;

						case Variable::BytesQuoted:
							if (place > 0)
							{
								data << current_person.bytesquoted;
							}
							else
							{
								data << engine->GetTotalQBytes();
							}
							known_data = true;
							break;

						case Variable::TotalAreas:
							data << engine->GetTotalAreas();
							known_data = true;
							break;

						case Variable::TotalPeople:
							data << engine->GetTotalPeople();
							known_data = true;
							break;

						case Variable::TotalNets:
							data << engine->GetTotalNets();
							known_data = true;
							break;

						case Variable::TotalDomains:
							data << engine->GetTotalDomains();
							known_data = true;
							break;

						case Variable::TotalSubjects:
							data << engine->GetTotalSubjects();
							known_data = true;
							break;

						case Variable::TotalPrograms:
							data << engine->GetTotalPrograms();
							known_data = true;
							break;

						case Variable::EarliestReceived:
						case Variable::LastReceived:
						case Variable::EarliestWritten:
						case Variable::LastWritten:
							// Timestamp, formatted according to the supplied
							// options.
							{
								// Get the appropriate timestamp
								time_t timestamp = static_cast<time_t>(-1);
								switch (variable_p->GetType())
								{
								case Variable::EarliestReceived:
									timestamp = engine->GetEarliestReceived();
									break;

								case Variable::LastReceived:
									timestamp = engine->GetLastReceived();
									break;

								case Variable::EarliestWritten:
									timestamp = engine->GetEarliestWritten();
									break;

								case Variable::LastWritten:
									timestamp = engine->GetLastWritten();
									break;

								default:
									break;
								}

								if (timestamp != static_cast<time_t>(-1))
								{
									// Format the timestamp accoding to the locale
									struct tm *p1 = gmtime(&timestamp);
									char date[64];
#if defined(HAVE_LOCALE_H) || defined(HAVE_OS2_COUNTRYINFO) || defined(HAVE_WIN32_LOCALEINFO)
									if (uselocale)
										localetimestring(p1, 64, date);
									else
#endif
										strftime(date, 64, "%Y-%m-%d %H:%M:%S", p1);

									data << date;
									known_data = true;
								}
							}
							break;

						case Variable::BytesOriginal:
							data <<
								current_person.byteswritten -
								current_person.bytesquoted;
							reportline << right;
							known_data = true;
							break;

						case Variable::PerMessage:
							data <<
								(current_person.byteswritten - current_person.bytesquoted)
								/ current_person.messageswritten;
							reportline << right;
							known_data = true;
							break;

						case Variable::Fidonet:
							// This variable is special, the width defines
							// the right-aligned zone and the left-aligned
							// network field. We output the first part here,
							// and put the rest in data.
							reportline << right << current_net.zone << ':';
							reportline << setw(width) << left;
							data << current_net.net;
							known_data = true;
							break;
						
						case Variable::TopDomain:
							data << current_domain.topdomain;
							known_data = true;
							break;

						case Variable::Received:
							reportline << right;
							data << current_person.messagesreceived;
							known_data = true;
							break;

						case Variable::ReceiveRatio:
							reportline << right;
							if (current_person.messageswritten)
							{
								data <<
									(100 * current_person.messagesreceived) /
									current_person.messageswritten << "%";
							}
							else if (showallnums)
							{
								data << "N/A";
							}
							known_data = true;
							break;
							
						case Variable::Subject:
							{
								string subject = encoder_p->Encode(current_subject.subject);
								if (subject != "")
								{
									data << subject;
								}
								else
								{
									data << "(none)";
								}
								known_data = true;
							}

						case Variable::Program:
							{
								string program = encoder_p->Encode(current_program.program);
								data << program;
								known_data = true;
							}
							break;

						case Variable::Bar:
							{
								unsigned int len = width;
								if (maxposts)
								{
									len = (60 * current_day_or_hour) / maxposts;
								}
								for (unsigned int i = 0; i < len; i ++)
								{
									data << '*';
								}
								known_data = true;
							}
							break;
						}

						if (end_iteration)
						{
							// No more data to output, either not a top-list
							// section, or we ran out of data early.
							toplist_length = place - 1;
							current_token_p = NULL;
							reportline.str(string());
						}
						else if (next_iteration)
						{
							// No data to show for this iteration (a day or
							// hour without postings).
							current_token_p = NULL;
						}
						else
						{
							string s(known_data ? data.str() : "?");
							if (width && static_cast<streamsize>(data.str().length()) > width)
							{
								s.erase(width);
							}
							reportline << s;
						}
					}
					else if (current_token_p->IsSetting())
					{
						const Setting *setting_p =
							static_cast<const Setting *>(current_token_p);
						int idx = 0;
						switch (setting_p->GetType())
						{
							case Setting::Sunday:
								++ idx;
							case Setting::Saturday:
								++ idx;
							case Setting::Friday:
								++ idx;
							case Setting::Thursday:
								++ idx;
							case Setting::Wednesday:
								++ idx;
							case Setting::Tuesday:
								++ idx;
							case Setting::Monday:
								m_dayname[idx] = setting_p->GetValue();
								break;

							default:
								// This cannot happen, there are no more settings types.
								TDisplay::GetOutputObject()->InternalErrorQuit(TDisplay::program_halted, 1);
								break;
						}
					}
					else
					{
						// This cannot happen, there are no more token types.
						TDisplay::GetOutputObject()->InternalErrorQuit(TDisplay::program_halted, 1);
					}
	            }

				// Advance to next token
				if (current_token_p)
				{
					current_token_p = current_token_p->Next();
				}
			}

			if (have_linebreak)
			{
				// Remove any trailing spaces from the report line
				string::size_type lastnonspace = reportline.str().find_last_not_of(" ");
				if (lastnonspace != string::npos)
				{
					report << reportline.str().substr(0, lastnonspace + 1);
				}
				report << endl;

				// Clear line
				reportline.str(string());
			}

		// Repeat the current line if in a toplist.
		} while (place > 0 && place < toplist_length);

		if (place > 0)
		{
			// Reset
			place = 0;

			// Close the iterator
			switch (current_section)
			{
				case Section::Original:
				case Section::Quoters:
				case Section::Writers:
				case Section::Received:
					engine->DoneTopPeople();
					break;

				case Section::TopNets:
					engine->DoneTopNets();
					break;

				case Section::TopDomains:
					engine->DoneTopDomains();
					break;

				case Section::Subjects:
					engine->DoneTopSubjects();
					break;

				case Section::Programs:
					engine->DoneTopPrograms();
					break;

				case Section::Week:
				case Section::Day:
								// Nothing to do
					break;

				default:
								// Section without a toplist.
					break;
			}
		}

        // Advance to the next line
        current_line_p = current_line_p->Next();
    }

#if 0
	// Old code to handle versions in the top list of programs:
	if (showversions)
	{
		bool restartv = true;
		StatEngine::verstat_s vdata;
		while (engine->GetProgramVersions(restartv, vdata))
		{
			if (restartv)
			{
				restartv = false;
				report << "      ";
			}
			string version = encoder_p->Encode(vdata.version);
			report << ' ' << version << ':' << vdata.count;
		}
		if (!restartv) report << endl;
	}
#endif

    report.close();

    delete encoder_p;

    return true;
}
