// Copyright (c) 2000-2008 Peter Karlsson
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
#include <qlabel.h>
#include <qprogressbar.h>
#include <qpushbutton.h>

#include "qtbars.h"
#include "statengine.h"

/** Class used to display bars for frequency graphs. */
class DataBar : public QProgressBar
{
    Q_OBJECT

public:
    /**
     * Standard constructor.
     * @param parent Parent widget owning the bar.
     */
    DataBar(QWidget *parent) : QProgressBar(parent) {};

protected:
    /**
     * Set the value to display with this bar. This overrides the percentage
     * display that is used by default by QProgressBar.
     *
     * @param indicator Reference to string where the caption that should be
     *                  displayed is written into.
     * @param progress  The number of entries associated with this bar.
     * @return True on success (always succeeds).
     */ 
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

BarWindow::BarWindow(QWidget *parent, bar_e bartype)
    : QDialog(parent)
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
	m_numentries = (Days == type) ? 7 : 24;
	m_bars_pp = new QProgressBar *[m_numentries];

    // Create layout
	QGridLayout *layout = new QGridLayout(this);

    if (Days == type)
    {
        // Set window caption
		setWindowTitle(tr("Daily breakdown"));

        // Seven bars, one per weekday
        for (int i = 0; i < 7; i ++)
        {
            // Create data label and bar
            QLabel *l = new QLabel(tr(days[i]), this);
			m_bars_pp[i] = new DataBar(this);
			m_bars_pp[i]->setFormat("");
			m_bars_pp[i]->setMinimumSize(200, 5);
            layout->addWidget(l, i, 0);
			layout->addWidget(m_bars_pp[i], i, 1);
        }
    }
    else
    {
        // Set window caption
        setWindowTitle(tr("Hourly breakdown"));

        QString s;

        // 24 bars, one per hour per day
        for (int i = 0; i < 24; i ++)
        {
            // Create data label and bar
            s.sprintf("%02d:00-%02d:00", i, i + 1);
            QLabel *l = new QLabel(s, this);
			m_bars_pp[i] = new DataBar(this);
			m_bars_pp[i]->setFormat("");
			m_bars_pp[i]->setMinimumSize(200, 5);
            layout->addWidget(l, i, 0);
			layout->addWidget(m_bars_pp[i], i, 1);
        }
    }

    // Add close button
    QPushButton *ok = new QPushButton(tr("Dismiss"), this);
	layout->addWidget(ok, m_numentries, 0, 1, 2);
    connect(ok, SIGNAL(clicked()), SLOT(accept()));
}

BarWindow::~BarWindow()
{
	delete[] m_bars_pp;
}

void BarWindow::fillOut(StatEngine *engine)
{
    int maximum = 1;

    // Load data
	int *data = new int[m_numentries];

	for (int i = 0; i < m_numentries; i ++)
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
	for (int i = 0; i < m_numentries; i ++)
    {
		m_bars_pp[i]->setMaximum(maximum);
		m_bars_pp[i]->setValue(data[i]);
    }

    delete[] data;
}
