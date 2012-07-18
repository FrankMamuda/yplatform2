/*
===========================================================================
Copyright (C) 2012 Edd 'Double Dee' Psycho

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see http://www.gnu.org/licenses/.

===========================================================================
*/

#ifndef NF_SCANNER_H
#define NF_SCANNER_H

//
// includes
//
#include "module_global.h"
#include <QThread>

//
// class:Scanner
//
class Scanner : public QThread {
    Q_OBJECT
    Q_PROPERTY( QString path READ path WRITE setPath )

public:
    void run();
    void scan();
    void filter( const QString &, const QString & );
    QString path() const { return this->m_path; }
    bool checkTags( const QString & );

public slots:
    void setPath( const QString &path ) { this->m_path = path; }
    void setBlockables( const QStringList &blockables ) { this->m_blockables = blockables; }

signals:
    void panic( const QString & );

private:
    QString m_path;
    QStringList m_blockables;
};

#endif // NF_SCANNER_H
