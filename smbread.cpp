// Copyright (c) 2007-2008 Stephen Hurd
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
#include <string>
#ifdef HAVE_IOSTREAM
# include <iostream>
#else
# include <iostream.h>
#endif
#include <time.h>
#include <stdio.h>
#include <smblib.h>

#include "smbread.h"
#include "utility.h"
#include "statengine.h"
#include "output.h"

SMBRead::SMBRead(const char *path)
{
    areapath = strdup(path);
}

SMBRead::~SMBRead()
{
    if (areapath) free(areapath);
}

bool SMBRead::Transfer(time_t starttime, time_t endtime,
                       StatEngine &destination)
{
    // Get the output object
    TDisplay *display = TDisplay::GetOutputObject();

    // Check that we got the path correctly in initialization
    if (!areapath)
    {
        display->InternalErrorQuit(TDisplay::area_not_allocated, 1);
    }

	// Initialize the smb object
	memset(&smb,0,sizeof(smb));

    // Open the message area files
    // <name>.jdx - contains one record per message
	sprintf(smb.file,"%.*s",sizeof(smb.file)-1,areapath);
	char *lastdot=strrchr(smb.file, '.');
	if(lastdot && (!stricmp(lastdot,".shd")))
		*lastdot=0;

    if (smb_open(&smb))
    {
        display->ErrorMessage(TDisplay::cannot_open, areapath);
        return false;
    }

    // Read messages one by one
    unsigned active = 0, total = smb.status.total_msgs;
	ulong		msgnum,l;
	smbmsg_t	msg;

    string ctrlinfo, sender, recipient, subject;
    char *buf;

    display->SetMessagesTotal(total);

#if 0
	destination.NewsArea();
#endif

    for(msgnum=0; msgnum < smb.status.total_msgs; msgnum++)
    {
		/* Read the index */
		msg.hdr.number=0;
		msg.offset=msgnum;
		if(smb_getmsgidx(&smb, &msg))
        {
            // Could not read message index, quit
            display->ErrorMessage(TDisplay::cannot_read_header,
                                  msgnum);
            continue;
        }

        if  (!(msg.idx.attr & MSG_DELETE))
        {
            // This is message is active
            active ++;
            display->UpdateProgress(active);

            // Check if message is outside time range
            if (time_t(msg.idx.time) < starttime
	                || time_t(msg.idx.time) > endtime)
                continue;

			// Read message header
			if (smb_getmsghdr(&smb, &msg)) {
        		// Could not read message, quit
        		display->ErrorMessage(TDisplay::cannot_read_header,
                                	  msgnum);
        		continue;
			}

            // Retrieve message body
            buf = smb_getmsgtxt(&smb, &msg, GETMSGTXT_BODY_ONLY);

            if (!buf)
            {
                display->WarningMessage(TDisplay::cannot_allocate_body,
                                        msgnum);
				smb_freemsgmem(&msg);
                continue;
            }

            // Locate sender, recipient, subject and PID and MSGID kludges
            // from subfields.
            sender = (const char *)msg.from;
            recipient = (const char *)msg.to;
            subject = (const char *)msg.subj;
            ctrlinfo = "";
#if 0
			if(msg.ftn_msgid) {
				ctrlinfo += "\1MSGID: ";
				ctrlinfo += (const char *)msg.ftn_msgid;
			}
			else {
				/*
				 * MSGID is hacked up for network addres...
				 * hack harder.
				 */
				if(msg.from_net.type != NET_NONE && msg.from_net.addr && strlen((const char *)msg.from_net.addr)) {
					ctrlinfo += "\1MSGID: ";
					ctrlinfo += (const char *)msg.from_net.addr;
					ctrlinfo += " hackmebaby";
				}
			}
#else
			/*
			 * MSGID is hacked up for network addres...
			 * hack harder.
			 */
			if(msg.from_net.type != NET_NONE) {
				ctrlinfo += "\1MSGID: ";
				ctrlinfo += (const char *)smb_netaddr(&(msg.from_net));
				ctrlinfo += " hackmebaby hack hack";
			}
#endif
			ctrlinfo += "\1From: ";
			ctrlinfo += (const char *)msg.from;
			ctrlinfo += "@";
			if(msg.from_net.type != NET_NONE)
				ctrlinfo += (const char *)smb_netaddr(&(msg.from_net));
			ctrlinfo += "\1To: ";
			ctrlinfo += (const char *)msg.to;
			if(msg.ftn_reply) {
				ctrlinfo += "\1REPLY: ";
				ctrlinfo += (const char *)msg.ftn_reply;
			}
			if(msg.ftn_area) {
				ctrlinfo += "\1AREA: ";
				ctrlinfo += (const char *)msg.ftn_area;
			}
			if(msg.ftn_flags) {
				ctrlinfo += "\1FLAGS: ";
				ctrlinfo += (const char *)msg.ftn_flags;
			}
			if(msg.ftn_pid) {
				ctrlinfo += "\1PID: ";
				ctrlinfo += (const char *)msg.ftn_pid;
			}
			if(msg.ftn_tid) {
				ctrlinfo += "\1TID: ";
				ctrlinfo += (const char *)msg.ftn_tid;
			}

			// Add text headers
		    /* variable fields */
            for(l=0;l<msg.total_hfields;l++) {
				switch(msg.hfield[l].type) {
					case RFC822HEADER:
						ctrlinfo += "\1";
						ctrlinfo += (const char *)msg.hfield_dat[l];
						break;
					case RFC822MSGID:
						ctrlinfo += "\1";
						ctrlinfo += "Message-Id: ";
						ctrlinfo += (const char *)msg.hfield_dat[l];
						break;
					case RFC822REPLYID:
						ctrlinfo += "\1";
						ctrlinfo += "Reply-Id: ";
						ctrlinfo += (const char *)msg.hfield_dat[l];
						break;
					case RFC822TO:
						ctrlinfo += "\1";
						ctrlinfo += "To: ";
						ctrlinfo += (const char *)msg.hfield_dat[l];
						break;
					case RFC822FROM:
						ctrlinfo += "\1";
						ctrlinfo += "From: ";
						ctrlinfo += (const char *)msg.hfield_dat[l];
						break;
					case RFC822REPLYTO:
						ctrlinfo += "\1";
						ctrlinfo += "In-Reply-To: ";
						ctrlinfo += (const char *)msg.hfield_dat[l];
						break;
					/* Don't know how to do this yet...
					case USENETPATH:
						ctrlinfo += "\1";
						ctrlinfo += (const char *)msg.hfield_dat[l];
						break;
					case USENETNEWSGROUPS:
						ctrlinfo += "\1";
						ctrlinfo += (const char *)msg.hfield_dat[l];
						break; */
				}
			}

            destination.AddData(sender, recipient, subject,
                                ctrlinfo, string(buf),
                                msg.hdr.when_written.time,
								msg.idx.time);

            smb_freemsgtxt(buf);
			smb_freemsgmem(&msg);
        }
    }

    return true;
}
