// Copyright (c) 2002-2008 Peter Karlsson
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

ProgressText::ProgressText(QWidget *parent, const QString &title)
	: QDialog(parent)
{
	QGridLayout *layout_p = new QGridLayout(this);
	QLabel *t_p = new QLabel(title, this);
	layout_p->addWidget(t_p, 0, 0, 1, 2);

	// Create horizontal layout
	m_progress_p = new QLabel("0", this);
	layout_p->addWidget(m_progress_p, 1, 0);
	QLabel *l_p = new QLabel(tr("message(s) read"), this);
	layout_p->addWidget(m_progress_p, 1, 1);
}
