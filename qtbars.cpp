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

#include <qlayout.h>
#include <qlabel.h>
#include <qprogressbar.h>
#include <qpushbutton.h>

#include "qtbars.h"
#include "statengine.h"

class DataBar : public QProgressBar
{
    Q_OBJECT

public:
    DataBar(QWidget *parent) : QProgressBar(parent) {};

protected:
    bool setIndicator(QString &indicator, int progress, int)
    {
        if (progress >= 0)
        {
            indicator = QString::number(progress);
        }
        else
        {
            indicator = "0";
        }
        return true;
    }
};

#include "qtbars.moc"

BarWindow::BarWindow(QWidget *parent, const char *name, bar_e bartype)
{
    static const char *days[7] =
    {
        QT_TR_NOOP("Monday"),
        QT_TR_NOOP("Tuesday"),
        QT_TR_NOOP("Wednesday"),
        QT_TR_NOOP("Thursday"),
        QT_TR_NOOP("Friday"),
        QT_TR_NOOP("Saturday"),
        QT_TR_NOOP("Sunday"),
    };

    // Determine number of bars and create the pointer array
    type = bartype;
    numentries = (Days == type) ? 7 : 24;
    bars = new QProgressBar *[numentries];

    // Create layout
    QGridLayout *layout = new QGridLayout(this, numentries + 1, 2);

    if (Days == type)
    {
        // Set window caption
        setCaption(tr("Daily breakdown"));

        // Seven bars, one per weekday
        for (int i = 0; i < 7; i ++)
        {
            // Create data label and bar
            QLabel *l = new QLabel(tr(days[i]), this);
            bars[i] = new DataBar(this);
            bars[i]->setCenterIndicator(false);
            bars[i]->setMinimumSize(200, 5);
            layout->addWidget(l, i, 0);
            layout->addWidget(bars[i], i, 1);
        }
    }
    else
    {
        // Set window caption
        setCaption(tr("Hourly breakdown"));

        QString s;

        // 24 bars, one per hour per day
        for (int i = 0; i < 24; i ++)
        {
            // Create data label and bar
            s.sprintf("%02d:00-%02d:00", i, i + 1);
            QLabel *l = new QLabel(s, this);
            bars[i] = new DataBar(this);
            bars[i]->setCenterIndicator(false);
            bars[i]->setMinimumSize(200, 5);
            layout->addWidget(l, i, 0);
            layout->addWidget(bars[i], i, 1);
        }
    }

    // Add close button
    QPushButton *ok = new QPushButton(tr("Dismiss"), this);
    layout->addMultiCellWidget(ok, numentries, numentries, 0, 1);
    connect(ok, SIGNAL(clicked()), SLOT(accept()));
}

BarWindow::~BarWindow()
{
    delete[] bars;
}

void BarWindow::fillOut(StatEngine *engine)
{
    int maximum = 1;

    // Load data
    int *data = new int[numentries];

    for (int i = 0; i < numentries; i ++)
    {
        if (Days == type)
        {
            data[i] = engine->GetDayMsgs((i + 1) % 7);
        }
        else
        {
            data[i] = engine->GetHourMsgs(i);
        }

        if (data[i] > maximum)
        {
            maximum = data[i];
        }
    }

    // Setup the bars
    for (int i = 0; i < numentries; i ++)
    {
        bars[i]->setTotalSteps(maximum);
        bars[i]->setProgress(data[i]);
    }

    delete[] data;
}
