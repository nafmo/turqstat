; Svensk standardmall för rapporter från Turquoise SuperStat
; © 2002-2007 Peter Karlsson
;
; Rader som inleds med semikolon (som denna) kommer att ignoreras. För att
; skriva en rad med inledande semikolon i den färdiga rapporten inleder
; du raden med två semikolon (;;). Tomma rader i mallen skapar tomma rader
; i utdata.
;
; Ett avslutande omvänt snedstreck (\) betyder att raden inte kommer att
; avslutas med en radbrytning. För att skriva ett avslutande omvänt
; snedstreck avslutar du raden med två omvända snedstreck (\\).
;
; Variabler läggs in på formen @nyckelord@. För att lägga in ett ensamt @
; skriver du två snabel-a:n (@@).
;
; Filen läses sekventiellt.
;
; Alla rader som inleds med variabeln @place@ skapar en topplista, där radens
; mönster repeteras. Variablerna som beror på platsen i topplistan får sina
; värden av @place@-variabeln.
;
; Först kommer stöd för översättning. Denna sektion genererar ingen utdata,
; istället används dess innehåll för att konfigurera utdatamotorn.
[Localization]
; Namn på veckodagar
Mon=Måndag
Tue=Tisdag
Wed=Onsdag
Thu=Torsdag
Fri=Fredag
Sat=Lördag
Sun=Söndag
; Sedan kommer huvudet
[Common]
Turquoise SuperStat @version@ * Statistik för meddelandearea
======================================================

@copyright@

;
; Vad som skall göras om meddelandearean är tom. Rapportmotorn avslutar
; efter [IfEmpty]-sektionen eller den första icke-[Common]-sektionen.
[IfEmpty]
Meddelandearean är tom.
;
; Om den inte är tom fortsätter vi.
; @ifareas@ gör att raden endast tas med om mer än en meddelandearea eller
; diskussionsgrupp lästes in.
[IfNews]
Rapporten täcker @written@ messages \
@ifareas@i @totalareas@ grupper, \
[IfNotNews]
Rapporten täcker @written@ messages \
@ifareas@i @totalareas@ möten, \
; @ifreceived@ gör att raden endast tas med om det finns information om
; mottagningsdatum.
[Common]
som \
@ifreceived@togs emot av detta system mellan @earliestreceived@ och @lastreceived@ och \
skrevs mellan @earliestwritten@ och @lastwritten@.

[Quoters]
-----------------------------------------------------------------------------
Citatsvartlista (bland personer som skrivit minst tre meddelanden)

Plats Namn                                                       Texter Kvot
; Tal inom hakparenteser anger fältbredd.
@place[5]@ @name[58]@@written[5]@ @ratio[7]@

@bytestotal@ byte skrevs totalt (endast meddelandekroppar), av vilka \
@bytesquoted@, eller @ratio@, var citat.

[Writers]
-----------------------------------------------------------------------------
Skribenttopplista

Plats Namn                                               Texter  Byte  Citat
@place[5]@ @name[50]@@written[5]@ @byteswritten[8]@ @ratio[7]@

[IfNotNews]
@totalpeople@ personer identifierades totalt (sändare och mottagare).

[IfNews]
@totalpeople@ personer identifierades totalt.

[Original]
-----------------------------------------------------------------------------
Topplista över originalinnehåll per meddelande (bland personer som skrivit \
minst tre meddelanden)

Plats Namn                                        Orig. / Text = PrTxt Citat
@place[5]@ @name[43]@@bytesoriginal[6]@ /@written[5]@ = @permessage[5]@ @ratio[7]@

[TopNets]
-----------------------------------------------------------------------------
Topplista över nät

Plats   Zon:Nät   Texter      Byte
; Variablen @fidonet@ är speciell. Bredden anger bredden för både zon och
; nät, så den totala bredden av @fidonet[5] är 11.
@place[5]@ @fidonet[5]@ @written[8]@ @byteswritten[8]@

Meddelanden från @totalnets@ olika nät hittades.

[TopDomains]
-----------------------------------------------------------------------------
Topplista över toppdomäner

Plats Domän  Texter      Byte
@place[5]@ @topdomain[7]@@written[8]@ @byteswritten[8]@

Meddelanden från @totaldomains@ olika toppdomäner hittades.

[Received]
-----------------------------------------------------------------------------
Topplista över mottagare

Plats Namn                                Mott. Sänt  Kvot
@place[5]@ @name[35]@@received[5]@ @written[5]@ @receiveratio[6]@

[Subjects]
-----------------------------------------------------------------------------
Topplista över ärenderader

Plats Ärende                                                     Texter Byte
@place[5]@ @subject[57]@@written[6]@ @byteswritten[7]@

@totalsubjects@ olika ärenderader hittades.

[Programs]
-----------------------------------------------------------------------------
Topplista över program

Plats Program                              Texter
@place[5]@ @program[35]@@written[6]@

@totalprograms@ olika program hittades (utan hänsyn till version).

[Week]
; @place@ innehåller dagetikett
-----------------------------------------------------------------------------
Texter per veckodag

Dag        Texter
@place[9]@@written[6]@ @bar[60]@

[Day]
; @place@ innehåller timetikett
-----------------------------------------------------------------------------
Texter per timme

Timme      Texter
@place[9]@@written[6]@ @bar[60]@

[Common]
-----------------------------------------------------------------------------
