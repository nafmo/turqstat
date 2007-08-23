// Copyright (c) 2002-2007 Peter Karlsson
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

#ifndef __QTPROGRESS_H
#define __QTPROGRESS_

#include <qdialog.h>
#include <qlabel.h>

/**
 * Class describing a window showing the progress when reading an area where
 * the total number of messages is unknown. The dialog will show the number
 * of read messages as text, instead of drawing a bar like QProgressBar.
 */
class ProgressText : public QDialog
{
    Q_OBJECT

protected:
    /** The label used to display the progress. */
    QLabel *progress;

public:
    /**
     * Standard constructor. Create a window ready to display the progress
     * of the message base reader.
     * @param parent Parent widget owning the window.
     * @param name   Name given to this instance.
     * @param title  Title to display in window.
     */
    ProgressText(QWidget *parent, const char *name, const QString &title);

    /**
     * Update the progress indicator.
     * @param number Number of messages read so far.
     */
    void setProgress(int number) { progress->setNum(number); }
};

#endif
