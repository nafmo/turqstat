// Copyright (c) 2000 Peter Karlsson
//
// $Id$
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

#ifndef __QTBARS_H
#define __QTBARS_H

#include <qdialog.h>

class StatEngine;
class QProgressBar;

class BarWindow : public QDialog
{
    Q_OBJECT

public:
    enum bar_e
    {
        Days, Hours
    };

    BarWindow(QWidget *parent, const char *name, bar_e bartype);
    ~BarWindow();

    void fillOut(StatEngine *engine);

protected:
    QProgressBar **bars;

    enum bar_e type;
    int numentries;
};

#endif
