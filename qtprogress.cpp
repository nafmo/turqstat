// Copyright (c) 2002 Peter Karlsson
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

#include <qlayout.h>
#include <qstring.h>

#include "qtprogress.h"

ProgressText::ProgressText(QWidget *parent, const char *name,
                           const QString &title)
    : QDialog(parent, name, false)
{
    // Create vertical layout
    QBoxLayout *vlayout = new QVBoxLayout(this);
    QLabel *t = new QLabel(title, this);
    vlayout->addWidget(t);

    // Create horizontal layout
    QBoxLayout *hlayout = new QHBoxLayout(vlayout);
    progress = new QLabel("0", this);
    hlayout->addWidget(progress);
    QLabel *l = new QLabel(tr("message(s) read"), this);
    hlayout->addWidget(l);
}
