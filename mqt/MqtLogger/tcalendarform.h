/*====================================================================================
    This file is part of AdjQt, the QT based version of the RSGB
    contest adjudication software.

    AdjQt and its predecessor AdjSQL are Copyright 1992 - 2016 Mike Goodey G0GJV

    AdjQt is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AdjQt is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AdjQt in file gpl.txt.  If not, see <http://www.gnu.org/licenses/>.

======================================================================================*/

#ifndef TCALENDARFORM_H
#define TCALENDARFORM_H

#include "base_pch.h"
#include "Calendar.h"
#include "CalendarList.h"

class CalendarYear;

namespace Ui
{
    class TCalendarForm;
}

class TCalendarForm : public QDialog
{
        Q_OBJECT

        Ui::TCalendarForm *ui;
    private:  	// User declarations

        void FormShow ( );

        bool loadYear ( Calendar &hf, int year );
        void LoadGrid ( Calendar &cal );

        QVector<QSharedPointer<CalendarYear> > yearList;

        void doCloseEvent();

public Q_SLOTS:
        virtual void accept() override;
        virtual void reject() override;

    public:  		// User declarations
        explicit TCalendarForm( QWidget *parent, CalType calType );
        ~TCalendarForm() override;
        CalType calType;
        Calendar vhf;
        Calendar hf;
        Calendar mwave;
        Calendar hfother;
        Calendar vhfother;
        Calendar hfbartg;
        IndividualContest ic;
        void downloadFiles();

        QString description;
        QDateTime sdate;
        QString band;

        virtual int exec() override;

    private slots:
        void on_CloseButton_clicked();

        void on_SelectButton_clicked();

        void on_GetCalendarButton_clicked();

        void on_YearDownButton_clicked();

        void on_YearUpButton_clicked();

        void on_CalendarGrid_doubleClicked(const QModelIndex &index);

};

#endif // TCALENDARFORM_H
