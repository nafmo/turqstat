// Copyright (c) 2000 Peter Karlsson
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

/**
 * Class describing a window showing bars with posting frequencies. This
 * class is used in the Qt version to display the postings per weekday and
 * postings per hour graphs.
 */
class BarWindow : public QDialog
{
    Q_OBJECT

public:
    /**
     * Numeric constants describing all available frequency graphs that can
     * be displayed by this class.
     */
    enum bar_e
    {
        Days,   ///< Weekday posting frequencies.
        Hours   ///< Hour posting frequencies.
    };

    /**
     * Standard constructor. Creates a window ready to fill in with toplist
     * data.
     * @param parent  Parent widget owning the window.
     * @param name    Name given to this instance.
     * @param bartype Type of graph to display in the window.
     */
    BarWindow(QWidget *parent, const char *name, bar_e bartype);
    /** Standard destructor. */
    ~BarWindow();

    /**
     * Fill the graph with statistics from loaded message bases.
     * @param engine Pointer to the engine containing the statistics.
     */
    void fillOut(StatEngine *engine);

protected:
    /** Array of progress bars (ab)used to display frequencies. */
    QProgressBar **bars;

    /** The kind of frequencies associated with this object. */
    enum bar_e type;

    /** Number of entries in the QProgressBar array. */
    int numentries;
};

#endif
