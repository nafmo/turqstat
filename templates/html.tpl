; HTML template file for Turquoise SuperStat reports
; © 2002-2008 Peter Krefting
;
; Any line starting with a semi-colon (like this one) will be ignored. To
; output a line with an initial semi-colon in the final report, start the
; line with two semi-colons (;;). Empty lines in the template will create
; empty lines in the output.
;
; A trailing backslash (\) means that the line will not be terminated with
; a line break. To include a trailing backslash, terminate line with two
; backslashes (\\).
;
; Variables are inserted using @keyword@. To insert a literal @, write two
; at signs (@@).
;
; The file is read sequentially.
;
; Any line starting with a @place@ variable will create a top-list, repeating
; the pattern of that line. The variables that are dependent on the position
; in the top-list are initialized by the @place@ variable.
;
; First some configuration settings. This section is not replicated in the output,
; rather its contents is used to configure the output engine.
[Settings]
HTML=on
;
; First some localization support. This section is not replicated in the output,
; rather its contents is used to configure the output engine.
[Localization]
; Weekday names
Mon=Monday
Tue=Tuesday
Wed=Wednesday
Thu=Thursday
Fri=Friday
Sat=Saturday
Sun=Sunday
; Then comes the header.
[Common]
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN">
<html lang="en" dir="ltr">
<head>
 <title>Message area statistics - Turquoise SuperStat @version@</title>
 <meta name="generator" title="Turquoise SuperStat @version@">
 <style type="text/css">
  body { background: white; color: black; }
  .blurb {
   border-top: 1px solid #222; color: #222; padding-top: .5em;
   font-size: 90%;
  }
  table.toplist { border-collapse: collapse; empty-cells: show; }
  table.toplist th {
   font-weight: bold; background: #eee; color: black; text-align: left;
   border-bottom: 1px solid black;
  }
  table.toplist td {
   background: white; color: black; text-align: left;
   border-bottom: 1px solid #ccc;
  }
  table.toplist th, table.toplist td { padding-right: .5em; }
  .bar { font-family: monospace; }
  .total, .date { font-weight: bold; }
 </style>
</head>

<body>
 <h1>Message area statistics</h1>
 <p class="summary">
;
; What to do if the message area is empty. The report engine will exit after
; the [IfEmpty] section or the first non-[Common] section.
[IfEmpty]
  The message area is empty.
 </p>
</body>
;
; If it is not empty, we continue.
; @ifareas@ will cause the line to be included only if more than one message
; area or newsgroup was scanned.
[IfNews]
  This report covers <span class="total">@written@</span> messages
@ifareas@  in <span class="total">@totalareas@</span> groups,
[IfNotNews]
  This report covers <span class="total">@written@</span> messages
@ifareas@  in <span class="total">@totalareas@</span> areas,
; @ifreceived@ will cause the line to be included only if information about
; reception dates are available.
[Common]
  that were
@ifreceived@  received at this system between
@ifreceived@  <span class="date">@earliestreceived@</span> and
@ifreceived@  <span class="date">@lastreceived@</span>, and
  written between <span class="date">@earliestwritten@</span> and
  <span class="date">@lastwritten@</span>.
 </p>

[Quoters]
 <h2 id="quotes">Blacklist of quoters</h2>
 <p class="description">
  (of people who have written at least three messages)
 </p>

 <table class="toplist">
  <thead>
   <tr><th>Place</th><th>Name</th><th>Msgs</th><th>Ratio</th></tr>
  </thead>
  <tbody>
; The line with the @place@ token is repeated, so for now this must all be
; on one single line:
   <tr><td>@place@</td><td>@name@</td><td>@written@</td><td>@ratio@</td></tr>
  </tbody>
 </table>

 <p class="summary">
  A total of <span class="total">@bytestotal@</span> bytes were written
  (message bodies only), of which <span class="total">@bytesquoted@</span>,
  or <span class="total">@ratio@</span>, were quotes.
 </p>

[Writers]
 <h2 id="writers">Toplist of writers</h2>
 <table class="toplist">
  <thead>
   <tr><th>Place</th><th>Name</th><th>Msgs</th><th>Bytes</th><th>Quoted</th></tr>
  </thead>
  <tbody>
   <tr><td>@place@</td><td>@name@</td><td>@written@</td><td>@byteswritten@</td><td>@ratio@</td></tr>
  </tbody>
 </table>

[IfNotNews]
 <p class="summary">
  A total of <span class="total">@totalpeople@</span> people were identified
  (senders and recipients).
 </p>

[IfNews]
 <p class="summary">
  A total of <span class="total">@totalpeople@</span> people were identified.
 </p>

[Original]
 <h2 id="original">Toplist of original content per message</h2>
 <p class="description">
  (of people who have written at least three messages)
 </p>

 <table class="toplist">
  <thead>
   <tr><th>Place</th><th>Name</th><th>Orig./Msgs</th><th>= PrMsg</th><th>Quoted</th></tr>
  </thead>
  <tbody>
   <tr><td>@place@</td><td>@name@</td><td>@bytesoriginal@/@written@</td><td>@permessage@</td><td>@ratio@</td></tr>
  </tbody>
 </table>

[TopNets]
 <h2 id="nets">Toplist of nets</h2>

 <table class="toplist">
  <thead>
   <tr><th>Place</th><th>Zone:Net</th><th>Msgs</th><th>Bytes</th></tr>
  </thead>
  <tbody>
   <tr><td>@place@</td><td>@fidonet@</td><td>@written@</td><td>@byteswritten@</td></tr>
  </tbody>
 </table>
 
 <p class="summary">
  Msgs from <span class="total">@totalnets@</span> different nets were found.
 </p>

[TopDomains]
 <h2 id="domains">Toplist of topdomains</h2>

 <table class="toplist">
  <thead>
   <tr><th>Place</th><th>Domain</th><th>Msgs</th><th>Bytes</th></tr>
  </thead>
  <tbody>
   <tr><td>@place@</td><td>@topdomain@</td><td>@written@</td><td>@byteswritten@</td></tr>
  </tbody>
 </table>

 <p class="summary">
  Messages from <span class="total">@totaldomains@</span> different top
  domains were found.
 </p>

[Received]
 <h2 id="receivers">Toplist of receivers</h2>

 <table class="toplist">
  <thead>
   <tr><th>Place</th><th>Name</th><th>Rcvd</th><th>Sent</th><th>Ratio</th></tr>
  </thead>
  <tbody>
   <tr><td>@place@</td><td>@name@</td><td>@received@</td><td>@written@</td><td>@receiveratio@</td></tr>
  </tbody>
 </table>

[Subjects]
 <h2 id="subjects">Toplist of subjects</h2>

 <table class="toplist">
  <thead>
   <tr><th>Place</th><th>Subject</th><th>Msgs</th><th>Bytes</th></tr>
  </thead>
  <tbody>
   <tr><td>@place@</td><td>@subject@</td><td>@written@</td><td>@byteswritten@</td></tr>
  </tbody>
 </table>

 <p class="summary">
  A total of <span class="total">@totalsubjects@</span> subjects were
  identified.
 </p>

[Programs]
 <h2 id="programs">Toplist of programs</h2>

 <table class="toplist">
  <thead>
   <tr><th>Place</th><th>Program</th><th>Msgs</th></tr>
  </thead>
  <tbody>
   <tr><td>@place@</td><td>@program@</td><td>@written@</td><td>
  </tbody>
 </table>

 <p class="summary">
  A total of <span class="total">@totalprograms@</span> different programs
  (not counting different versions) were identified.
 </p>

[Week]
; @place@ contains the day token.
 <h2 id="weekdays">Postings per weekday</h2>

 <table class="toplist">
  <thead>
   <tr><th>Day</th><th colspan="2">Msgs</th></tr>
  </thead>
  <tbody>
   <tr><td>@place@</td><td>@written@</td><td class="bar">@bar[40]@</td>
  </tbody>
 </table>

[Day]
; @place@ contains the hour token.
 <h2 id="hours">Postings per hour</h2>

 <table class="toplist">
  <thead>
   <tr><th>Hour</th><th colspan="2">Msgs</th></tr>
  </thead>
  <tbody>
   <tr><td>@place@</td><td>@written@</td><td class="bar">@bar[40]@</td>
  </tbody>
 </table>

[Common]

 <p class="blurb">
  Generated by Turquoise SuperStat @version@
  <br>@copyright@
 </p>
</body>
