// Copyright (c) 1998-2007 Peter Karlsson
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
#if !defined(HAVE_FSTREAM_FORM_METHOD)
# include <ios>
# include <iomanip>
#endif
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

	// Remember toplist position, non-zero for looped lines
	unsigned int place = 0;
	unsigned int toplist_length = 0;
    while (current_line_p)
    {
		StatEngine::persstat_s current_person;
		StatEngine::netstat_s current_net;
		StatEngine::domainstat_s current_domain;
		StatEngine::subjstat_s current_subject;
		StatEngine::progstat_s current_program;
		unsigned int current_day_or_hour = UINT_MAX;
		unsigned int maxposts = 1; // Max posts for week/daystat bars

        // Loop over the tokens on this line
		do
		{
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
						report << literal_p->GetLiteral();
						if (literal_p->HasLineBreak())
						{
							report << endl;
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

						report << left;
						size_t width = variable_p->GetWidth();
						if (width > 0)
						{
							report << setw(width);
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
							// FIXME: Reimplement check if this entry has the
							// same primary-key value as previous entry,
							// and then hide number unless showallnums is
							// enabled.
							++ place;
							data << place;
							report << right;
							toplist_length = maxnumber;
							known_data = true;

							// Fetch the appropriate data record
							switch (current_section)
							{
							case Section::Original:
								{
									// Constraint: Only people having posted
									// at least three messages are considered.
									bool first = 1 == place;
									do
									{
										end_iteration =
											!engine->GetTopOriginalPerMsg(first, current_person);
										first = false;
									} while (!end_iteration && current_person.messageswritten < 3);
								}
								break;

							case Section::Quoters:
								{
									// Constraint: Only people having quoted
									// and having posted at least three
									// messages are listed.
									bool first = 1 == place;
									do
									{
										end_iteration =
												!engine->GetTopQuoters(first, current_person);
										first = false;
									} while (!end_iteration &&
									         (current_person.messageswritten < 3 || current_person.bytesquoted <= 0));
								}
								break;

							case Section::Writers:
								{
									// Constraint: Must have written at least
									// one message.
									bool first = 1 == place;
									do
									{
										end_iteration =
											!engine->GetTopWriters(first, current_person);
										first = false;
									} while (!end_iteration && current_person.messageswritten == 0);
								}
								break;

							case Section::TopNets:
								end_iteration =
									!engine->GetTopNets(1 == place, current_net);
								break;

							case Section::TopDomains:
								end_iteration =
									!engine->GetTopDomains(1 == place, current_domain);
								break;

							case Section::Received:
								end_iteration =
									!engine->GetTopReceivers(1 == place, current_person);
								break;

							case Section::Subjects:
								end_iteration =
									!engine->GetTopSubjects(1 == place, current_subject);
								break;

							case Section::Programs:
								end_iteration =
									!engine->GetTopPrograms(1 == place, current_program);
								break;

							case Section::Week:
								// This toplist is always seven entries
								toplist_length = 7;
								report << left;
								data << days[place - 1];
								current_day_or_hour =
									engine->GetDayMsgs(place - 1);

								// Find max width
								if (1 == place)
								{
									maxposts = current_day_or_hour;
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
								report << left;
								data << setfill('0') << setw(2) << (place - 1)
								     << "00-" << setw(2) << (place - 1);
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
							break;

						case Variable::Name:
							// Name of person.
							if (current_person.name.length())
							{
								string name =
									encoder_p->Encode(current_person.name);
								if (name != current_person.address)
								{
									data << name << " <" <<
									        current_person.address << ">";
								}
								else
								{
									data << name;
								}
								known_data = true;
							}
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
							report << right;
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
							report << right;
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

								// Percent x 1000
								unsigned int percentquotes =
									totalwrittenbytes
									? ((100000 * totalquotedbytes) / totalwrittenbytes)
									: 0;

								// Calculate integer and fraction percent
								unsigned int integ = percentquotes / 1000;
								unsigned int fract =
									(percentquotes - integ * 1000 + 5) / 10;
								if (100 == fract)
								{
									fract = 0;
									integ ++;
								}

								// Percent string
								data << setw(3) << integ << '.'
								     << setfill('0') << setw(2) << fract << '%';
							}
							report << right;
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
							known_data = true;
							break;

						case Variable::PerMessage:
							data <<
								(current_person.byteswritten - current_person.bytesquoted)
								/ current_person.messageswritten;
							known_data = true;
							break;

						case Variable::Fidonet:
						case Variable::TopDomain:
						case Variable::Received:
						case Variable::ReceiveRatio:
						case Variable::Subject:
						case Variable::Program:
						case Variable::Bar:
							break;
						}

						if (end_iteration)
						{
							// No more data to output, either not a top-list
							// section, or we ran out of data early.
							toplist_length = place - 1;
							current_token_p = NULL;
						}
						else if (next_iteration)
						{
							// No data to show for this iteration (a day or
							// hour without postings).
							current_token_p = NULL;
						}
						else
						{
							if (!known_data)
							{
								data << "?";
							}
							report << data.str();
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

		// Repeat the current line if in a toplist.
		} while (place > 0 && place <= toplist_length);

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
    if (quoters)
    {
        report << "-------------------------------------------"
                  "----------------------------------" << endl;

        report << "Blacklist of quoters (of people who have written at least "
                  "three messages)" << endl;
        report << endl;

        unsigned place = 1;
        bool restart = true;
        StatEngine::persstat_s data;
        unsigned oldpercent = 10100;
        while (place <= maxnumber && engine->GetTopQuoters(restart, data))
        {
            if (restart)
            {
                restart = false;
                report << "Place Name                                "
                          "                       Msgs   Ratio" << endl;
            }

            if (data.messageswritten > 2 && data.bytesquoted > 0)
            {
                unsigned percentquotes =
                    data.byteswritten ?
                      ((100000 * (unsigned long long) data.bytesquoted) /
                       (unsigned long long) data.byteswritten)
                      : 0;
                if (!showallnums && percentquotes == oldpercent)
                {
                    report << "      ";
                }
                else
                {
#if defined(HAVE_FSTREAM_FORM_METHOD)
                    report.form("%4u. ", place);
#else
                    report << setw(4) << place << ". ";
#endif
                }

                unsigned integ = percentquotes / 1000;
                unsigned fract = (percentquotes - integ * 1000 + 5) / 10;
                if (100 == fract)
                {
                    fract = 0;
                    integ ++;
                }

                string name = encoder_p->Encode(data.name);
                if (name != data.address)
                    tmp = string(name + " <") +
                          string(data.address + ">");
                else
                    tmp = name;

#if defined(HAVE_FSTREAM_FORM_METHOD)
                report.form("%-58.58s%5u %3u.%02u%%",
                            tmp.c_str(), data.messageswritten,
                            integ, fract);
#else
                if (tmp.length() > 58) tmp.erase(58);
                report << left << setw(58) << tmp
                       << right << setw(5) << data.messageswritten << ' '
                       << setw(3) << integ << '.'
                       << setfill('0') << setw(2) << fract << '%'
                       << setfill(' ');
#endif

                report << endl;

                place ++;
                oldpercent = percentquotes;
            }
        }

        report << endl;
        engine->DoneTopPeople();

        unsigned long long totalbytes = engine->GetTotalBytes();
        if (totalbytes)
        {
            unsigned long long totalqbytes = engine->GetTotalQBytes();

            report << "A total of " << totalbytes << " bytes were written "
                      "(message bodies only), of which " << totalqbytes
                   << ", or ";

            unsigned percentquotes =
                ((100000 * totalqbytes) / totalbytes);
            unsigned integ = percentquotes / 1000;
            unsigned fract = (percentquotes - integ * 1000 + 5) / 10;
            if (100 == fract)
            {
                fract = 0;
                integ ++;
            }
#if defined(HAVE_FSTREAM_FORM_METHOD)
            report.form("%u.%02u%%,", integ, fract)
                   << " were quotes." << endl;
#else
            report << integ << "." << setfill('0') << setw(2) << fract
                   << "%, were quotes." << setfill(' ') << endl;
#endif

            report << endl;
        }
    }

    if (topwritten)
    {
        report << "-------------------------------------------"
                  "----------------------------------" << endl;

        report << "Toplist of writers" << endl;
        report << endl;

        unsigned place = 1;
        bool restart = true;
        StatEngine::persstat_s data;
        unsigned oldwritten = 0;
        while (place <= maxnumber && engine->GetTopWriters(restart, data))
        {
            if (!data.messageswritten) break;

            if (restart)
            {
                restart = false;
                report << "Place Name                                "
                          "               Msgs    Bytes Quoted"  << endl;
            }

            if (!showallnums && data.messageswritten == oldwritten)
            {
                report << "      ";
            }
            else
            {
#if defined(HAVE_FSTREAM_FORM_METHOD)
                report.form("%4u. ", place);
#else
                report << setw(4) << place << ". ";
#endif
            }

            unsigned long long percentquotes =
                data.byteswritten ? ((10000 * data.bytesquoted) /
                                     data.byteswritten)
                             : 0;
            unsigned integ = percentquotes / 100;
            unsigned fract = (percentquotes - integ * 100 + 5) / 10;
            if (10 == fract)
            {
                fract = 0;
                integ ++;
            }

            string name = encoder_p->Encode(data.name);
            if (name != data.address)
                tmp = string(name + " <") +
                      string(data.address + ">");
            else
                tmp = name;

#if defined(HAVE_FSTREAM_FORM_METHOD)
            report.form("%-50.50s%5u %8u",
                        tmp.c_str(), data.messageswritten,
                        data.byteswritten);
#else
            if (tmp.length() > 50) tmp.erase(50);
            report << left << setw(50) << tmp
                   << right << setw(5) << data.messageswritten << ' '
                   << setw(8) << data.byteswritten;
#endif

            if (data.bytesquoted > 0)
            {
#if defined(HAVE_FSTREAM_FORM_METHOD)
                report.form(" %3u.%1u%%",
                             integ, fract);
#else
                report << " " << setw(3) << integ
                       << "." << setw(1) << fract << "%";
#endif
            }
            else if (showallnums)
            {
                report << "    N/A";
            }
            report << endl;

            place ++;
            oldwritten = data.messageswritten;
        }

        report << endl;
        engine->DoneTopPeople();

        if (news || !topreceived)
        {
            report << "A total of " << engine->GetTotalPeople()
                   << " people were identified";
            if (!news)
                report << " (senders and recipients)";
            report << '.' << endl;
            report << endl;
        }
    }

    if (toporiginal)
    {
        report << "-------------------------------------------"
                  "----------------------------------" << endl;

        report << "Toplist of original content per message (of people "
                  "who have written at least three messages)" << endl;
        report << endl;

        unsigned place = 1;
        bool restart = true;
        bool showheader = true;
        StatEngine::persstat_s data;
        unsigned oldoriginalpermsg = 0;
        while (place <= maxnumber &&
               engine->GetTopOriginalPerMsg(restart, data))
        {
            restart = false;
            if (data.messageswritten < 3)
            {
                continue;
            }

            if (showheader)
            {
                showheader = false;
                report << "Place Name                        "
                          "                Orig. / Msgs = PrMsg Quoted"
                       << endl;
            }

            unsigned originalpermsg =
                (data.byteswritten - data.bytesquoted) / data.messageswritten;

            if (!showallnums && originalpermsg == oldoriginalpermsg)
            {
                report << "      ";
            }
            else
            {
#if defined(HAVE_FSTREAM_FORM_METHOD)
                report.form("%4u. ", place);
#else
                report << setw(4) << place << ". ";
#endif
            }

            unsigned long long percentquotes =
                data.byteswritten ? ((10000 * data.bytesquoted) /
                                     data.byteswritten)
                             : 0;
            unsigned integ = percentquotes / 100;
            unsigned fract = (percentquotes - integ * 100 + 5) / 10;
            if (10 == fract)
            {
                fract = 0;
                integ ++;
            }

            string name = encoder_p->Encode(data.name);
            if (name != data.address)
                tmp = string(name + " <") +
                      string(data.address + ">");
            else
                tmp = name;

#if defined(HAVE_FSTREAM_FORM_METHOD)
            report.form("%-43.43s%6u /%5u = %5u",
                        tmp.c_str(),
                        data.byteswritten - data.bytesquoted,
                        data.messageswritten, originalpermsg);
#else
            if (tmp.length() > 43) tmp.erase(43);
            report << left << setw(43) << tmp
                   << right << setw(6) << data.byteswritten - data.bytesquoted
                   << " /"
                   << setw(5) << data.messageswritten << " = "
                   << setw(5) << originalpermsg;
#endif

            if (data.bytesquoted > 0)
            {
#if defined(HAVE_FSTREAM_FORM_METHOD)
                report.form(" %3u.%1u%%",
                             integ, fract);
#else
                report << " " << setw(3) << integ
                       << "." << setw(1) << fract << "%";
#endif
            }
            else if (showallnums)
            {
                report << "    N/A";
            }
            report << endl;

            place ++;
            oldoriginalpermsg = originalpermsg;
        }

        report << endl;
        engine->DoneTopPeople();
    }

    if (topnets && !news)
    {
        report << "-------------------------------------------"
                  "----------------------------------" << endl;

        report << "Toplist of nets" << endl;
        report << endl;

        unsigned place = 1;
        bool restart = true;
        StatEngine::netstat_s data;
        unsigned oldcount = 0;
        while (place <= maxnumber && engine->GetTopNets(restart, data))
        {
            if (restart)
            {
                restart = false;

                report << "Place  Zone:Net   Messages    Bytes" << endl;
            }

            if (!showallnums && data.messages == oldcount)
            {
                report << "      ";
            }
            else
            {
#if defined(HAVE_FSTREAM_FORM_METHOD)
                report.form("%4u. ", place);
#else
                report << setw(4) << place << ". ";
#endif
            }

#if defined(HAVE_FSTREAM_FORM_METHOD)
            report.form("%5u:%-5u %8u %8u",
                        data.zone, data.net, data.messages, data.bytes);
#else
            report << setw(5) << data.zone << ':'
                   << left << setw(5) << data.net << ' '
                   << right << setw(8) << data.messages << ' '
                   << setw(8) << data.bytes;
#endif

            report << endl;

            place ++;
            oldcount = data.messages;
        }

        report << endl;
        engine->DoneTopNets();

        report << "Messages from " << engine->GetTotalNets()
               << " different nets were found." << endl;
        report << endl;
    }

    if (topdomains && (news || engine->GetTotalDomains()))
    {
        report << "-------------------------------------------"
                  "----------------------------------" << endl;

        report << "Toplist of topdomains" << endl;
        report << endl;

        unsigned place = 1;
        bool restart = true;
        StatEngine::domainstat_s data;
        unsigned oldcount = 0;
        while (place <= maxnumber && engine->GetTopDomains(restart, data))
        {
            if (restart)
            {
                restart = false;

                report << "Place Domain Messages    Bytes" << endl;
            }

            if (!showallnums && data.messages == oldcount)
            {
                report << "      ";
            }
            else
            {
#if defined(HAVE_FSTREAM_FORM_METHOD)
                report.form("%4u. ", place);
#else
                report << setw(4) << place << ". ";
#endif
            }

#if defined(HAVE_FSTREAM_FORM_METHOD)
            report.form("%-7.7s%8u %8u",
                        data.topdomain.c_str(), data.messages, data.bytes);
#else
            string tmp(data.topdomain, 0, 7);
            report << left << setw(7) << tmp
                   << right << setw(8) << data.messages << " "
                   << setw(8) << data.bytes << endl;
#endif

            report << endl;

            place ++;
            oldcount = data.messages;
        }

        report << endl;
        engine->DoneTopDomains();

        report << "Messages from " << engine->GetTotalDomains()
               << " different top domains were found." << endl;
        report << endl;
    }

    if (topreceived && !news)
    {
        report << "-------------------------------------------"
                  "----------------------------------" << endl;

        report << "Toplist of receivers" << endl;
        report << endl;

        unsigned place = 1;
        bool restart = true;
        StatEngine::persstat_s data;
        unsigned oldrecvd = 0;
        while (place <= maxnumber && engine->GetTopReceivers(restart, data))
        {
            if (!data.messagesreceived) break;

            if (restart)
            {
                restart = false;
                report << "Place Name                                "
                          "Rcvd  Sent   Ratio" << endl;
            }

            if (!showallnums && data.messagesreceived == oldrecvd)
            {
                report << "      ";
            }
            else
            {
#if defined(HAVE_FSTREAM_FORM_METHOD)
                report.form("%4u. ", place);
#else
                report << setw(4) << place << ". ";
#endif
            }

            string name = encoder_p->Encode(data.name);
            if (!name.empty())
                tmp = name;
            else
                tmp = "(none)";

#if defined(HAVE_FSTREAM_FORM_METHOD)
            report.form("%-35.35s%5u %5u",
                        tmp.c_str(),
                        data.messagesreceived,
                        data.messageswritten);
#else
            if (tmp.length() > 35) tmp.erase(35);
            report << left << setw(35) << tmp
                   << right << setw(5) << data.messagesreceived << ' '
                   << setw(5) << data.messageswritten;
#endif

            if (data.messageswritten)
            {
#if defined(HAVE_FSTREAM_FORM_METHOD)
                report.form(" %6u%%", (100 * data.messagesreceived) /
                                       data.messageswritten);
#else
                report << " " << setw(6)
                       << (100 * data.messagesreceived) /
                           data.messageswritten
                       << "%";
#endif
            }
            else if (showallnums)
            {
                report << "     N/A";
            }

            report << endl;

            place ++;
            oldrecvd = data.messagesreceived;
        }

        report << endl;
        engine->DoneTopPeople();
    }

    if (!news && topreceived)
    {
        report << "A total of " << engine->GetTotalPeople()
               << " people were identified (senders and recipients)." << endl;
        report << endl;
    }

    if (topsubjects)
    {
        report << "-------------------------------------------"
                  "----------------------------------" << endl;

        report << "Toplist of subjects" << endl;
        report << endl;

        unsigned place = 1;
        bool restart = true;
        StatEngine::subjstat_s data;
        unsigned oldcount = 0;
        while (place <= maxnumber && engine->GetTopSubjects(restart, data))
        {
            if (restart)
            {
                restart = false;
                report << "Place Subject                                  "
                          "                  Msgs   Bytes" << endl;
            }

            if (!showallnums && data.count == oldcount)
            {
                report << "      ";
            }
            else
            {
#if defined(HAVE_FSTREAM_FORM_METHOD)
                report.form("%4u. ", place);
#else
                report << setw(4) << place << ". ";
#endif
            }

            string subject = encoder_p->Encode(data.subject);
            if (subject != "")
                tmp = subject;
            else
                tmp = "(none)";

#if defined(HAVE_FSTREAM_FORM_METHOD)
            report.form("%-57.57s%6u %7u",
                        tmp.c_str(), data.count, data.bytes);
#else
            if (tmp.length() > 57) tmp.erase(57);
            report << left << setw(57) << tmp
                   << right << setw(6) << data.count << ' '
                   << setw(7) << data.bytes;
#endif

            report << endl;

            place ++;
            oldcount = data.count;
        }

        report << endl;
        engine->DoneTopSubjects();

        report << "A total of " << engine->GetTotalSubjects()
               << " subjects were identified." << endl;
        report << endl;
    }

    if (topprograms)
    {
        report << "-------------------------------------------"
                  "----------------------------------" << endl;

        report << "Toplist of programs" << endl;
        report << endl;

        unsigned place = 1;
        bool restart = true;
        StatEngine::progstat_s data;
        unsigned oldcount = 0;
        while (place <= maxnumber && engine->GetTopPrograms(restart, data))
        {
            if (restart)
            {
                restart = false;
                report << "Place Program                              Msgs"
                       << endl;
            }

            if (!showallnums && data.count == oldcount)
            {
                report << "      ";
            }
            else
            {
#if defined(HAVE_FSTREAM_FORM_METHOD)
                report.form("%4u. ", place);
#else
                report << setw(4) << place << ". ";
#endif
            }

            string program = encoder_p->Encode(data.program);
#if defined(HAVE_FSTREAM_FORM_METHOD)
            report.form("%-35.35s%6u", program.c_str(), data.count);
#else
            if (program.length() > 35) program.erase(35);
            report << left << setw(35) << program
                   << right << setw(6) << data.count;
#endif
            report << endl;

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

            place ++;
            oldcount = data.count;
        }

        report << endl;
        engine->DoneTopPrograms();

        report << "A total of " << engine->GetTotalPrograms()
               << " different programs (not counting different versions) "
                  "were identified." << endl;
        report << endl;
    }

    if (weekstats)
    {
        report << "-------------------------------------------"
                  "----------------------------------" << endl;

        report << "Postings per weekday" << endl;
        report << endl;

        unsigned day[7], max = 1;
        for (int i = 0; i < 7; i ++)
        {
            day[i] = engine->GetDayMsgs(i);
            if (day[i] > max) max = day[i];
        }

        report << "Day        Msgs" << endl;

        for (int i = 0; i < 7; i ++)
        {
            int d = (i + 1) % 7;
            if (day[d] || showallnums)
            {
                report << days[i];
#if defined(HAVE_FSTREAM_FORM_METHOD)
                report.form("%6u ", day[d]);
#else
                report << setw(6) << day[d] << ' ';
#endif
                int len = (60 * day[d]) / max;
                for (int j = 0; j < len; j ++) report << '*';
                report << endl;
            }
        }

        report << endl;
    }

    if (daystats)
    {
        report << "-------------------------------------------"
                  "----------------------------------" << endl;

        report << "Postings per hour" << endl;
        report << endl;

        unsigned hour[24], max = 1;
        for (int i = 0; i < 24; i ++)
        {
            hour[i] = engine->GetHourMsgs(i);
            if (hour[i] > max) max = hour[i];
        }

        report << "Hour       Msgs" << endl;

        for (int i = 0; i < 24; i ++)
        {
            if (hour[i] || showallnums)
            {
#if defined(HAVE_FSTREAM_FORM_METHOD)
                report.form("%02d00-%02d59%6u ", i, i, hour[i]);
#else
                report << setfill('0') << setw(2) << i << "00-"
                       << setw(2) << i << "59" << setfill(' ')
                       << setw(6) << hour[i] << ' ';
#endif
                int len = (60 * hour[i]) / max;
                for (int j = 0; j < len; j ++) report << '*';
                report << endl;
            }
        }

        report << endl;
    }

    report << "-------------------------------------------"
              "----------------------------------" << endl;
#endif // 0

    report.close();

    delete encoder_p;

    return true;
}
