; Default template file for Turquoise SuperStat reports
; © 2002-2007 Peter Karlsson
;
; Any line starting with a semi-colon will be ignored. To start a line with
; a semi-colon in the final report, start the line with two semi-colons. Empty
; lines in the input will create empty lines in the output.
;
; A trailing backslash means that the line will not be terminated with a line
; break. To include a trailing backslash, end the line with two backslashes.
;
; Special tokens are inserted using @keyword@. To insert a literal @, double it.
;
; The file is read sequentially.
;
; Any line with a @place@ variable will create a top-list, repeating the
; pattern of the line. The statistics is only initialized for variables
; following the @place@ variable.
;
; First comes the header
[Common]
Turquoise SuperStat @version@ * Message area statistics
=================================================

@copyright@

;
; What to do if the message area is empty. The report engine will exit after
; the [IfEmpty] section or the first non-[Common] section.
[IfEmpty]
Message area is empty.
;
; If it is not empty, we continue.
; @ifareas@ will cause the line to be included only if more than one message
; area or newsgroup was scanned.
[IfNews]
This report covers @written@ messages \
@ifareas@in @totalareas@ groups, \
[IfNotNews]
This report covers @written@ messages \
@ifareas@in @totalareas@ areas, \
; @ifreceived@ will cause the line to be included only if information about
; reception dates are available.
[Common]
that were \
@ifreceived@received at this system between @earliestreceived@ and @lastreceived@ and \
written between @earliestwritten@ and @lastwritten@.

[Quoters]
-----------------------------------------------------------------------------
Blacklist of quoters (of people who have written at least three messages)

Place Name                                                       Msgs   Ratio
; Numbers in brackets denote field width. The ratio is always seven characters.
@place[4]@. @name[58]@@written[5]@ @ratio@

A total of @bytestotal@ bytes were written (message bodies only), of which @bytesquoted@, or @ratio@ were quotes.

[Writers]
-----------------------------------------------------------------------------
Toplist of writers

Place Name                                               Msgs    Bytes Quoted
@place[4]@. @name[50]@@written[5]@ @byteswritten[8]@ @ratio@

[IfNotNews]
A total of @totalpeople@ people were identified (senders and recipients).

[IfNews]
A total of @totalpeople@ people were identified.

[Original]
-----------------------------------------------------------------------------
Toplist of original content per message (of people who have written at least three messages)

Place Name                                        Orig. / Msgs = PrMsg Quoted
@place[4]@. @name[43]@@bytesoriginal[6]@ /@written[5]@ = @permessage[5]@ @ratio@

[TopNets]
-----------------------------------------------------------------------------
Toplist of nets

Place  Zone:Net   Messages    Bytes
; The @fidonet@ token is special. The width specifies the width of both the
; zone and net fields, so the total width of @fidonet[5]@ will be 11.
@place[4]@. @fidonet[5]@ @written[8]@ @byteswritten[8]@

Messages from @totalnets@ different nets were found.

[TopDomains]
-----------------------------------------------------------------------------
Toplist of topdomains

Place Domain Messages    Bytes
@place[4]@. @topdomain[7]@@written[8]@ @byteswritten[8]@

Messages from @totaldomains@ different top domains were found

[Received]
-----------------------------------------------------------------------------
Toplist of receivers

Place Name                                Rcvd  Sent  Ratio
@place[4]@. @name[35]@@received[5]@ @written[5]@ @receiveratio[6]@

[Subjects]
-----------------------------------------------------------------------------
Toplist of subjects

Place Subject                                                    Msgs   Bytes
@place[4]@. @subject[57]@@written[6]@ @byteswritten[7]@

A total of @totalsubjects@ subjects were identified.

[Programs]
-----------------------------------------------------------------------------
Toplist of programs

Place Program                              Msgs
@place[4]@. @program[35]@@written[6]@

A total of @totalprograms@ different programs (not counting different versions) were identified.

[Week]
; @place@ contains the day token.
-----------------------------------------------------------------------------
Postings per weekday

Day        Msgs
@place[9]@@written[6]@ @bar[60]@

[Day]
; @place@ contains the hour token.
-----------------------------------------------------------------------------
Postings per hour

Hour       Msgs
@place@ @written[6]@ @bar[60]@

[Common]
-----------------------------------------------------------------------------
