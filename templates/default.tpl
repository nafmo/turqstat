; Default template file for Turquoise SuperStat reports
; © 2002-2007 Peter Karlsson
;
; Any line starting with a semi-colon will be ignored. To start a line with
; a semi-colon in the final report, start the line with two semi-colons. Empty
; lines in the input will create empty lines in the output.
;
; Special tokens are inserted using @keyword@. To insert a literal @, double it.
;
; The file is read sequentially.
;
; First comes the header
[Common]
Turquoise SuperStat @version@ * Message area statistics
=================================================

;
; What to do if the message area is empty. The report engine will exit after
; the [IfEmpty] section or the first non-[Common] section.
[IfEmpty]
Message area is empty.
;
; If it is not empty, we continue.
[Common]
; Display total values. The string within the brackets is the language for the
; string. Please note that the language must be supported for this to work.
@totals[en]@

[Quoters]
-----------------------------------------------------------------------------
Blacklist of quoters (of people who have written at least three messages)

Place Name                                                       Msgs   Ratio
; Numbers in brackets denote field width. The ratio is always seven characters.
@place[4]@. @name[58]@@written[5]@ @ratio@

A total of @bytestotal@ bytes were written (message bodies only), of which @bytesquoted@, or @bytesquotedpercent@ were quotes.

[Writers]
-----------------------------------------------------------------------------
Toplist of writers

Place Name                                               Msgs    Bytes Quoted
@place[4]@. @name[50]@@written[5]@ @byteswritten[8]@ @ratio@

[IfNotNews]
A total of @totalpeople@ people were identified (senders and recipients).
[IfNews]
A total of @totalpeople@ people were idenetified.

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

Place Name                                Rcvd  Sent   Ratio
@place[4]@. @name[35]@@received[5]@ @written[5]@ @receiveratio@

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
@place[4]@ @program[35]@@written[6]@

A total of @totalprograms@ different programs (not counting different versions) were identified.

[Week]
-----------------------------------------------------------------------------
Postings per weekday

Day        Msgs
@day[en][9]@@written[6]@ @bar[60]@

[Day]
-----------------------------------------------------------------------------
Postings per hour

Hour       Msgs
@hour@@written[6]@ @bar[60]@

[Common]
-----------------------------------------------------------------------------
