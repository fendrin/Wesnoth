/*
   Copyright (C) 2014 - 2015 by Fabian Mueller <fabianmueller5@gmx.de>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
 */

#ifndef QT_WIDGET_INCLUDED
#define QT_WIDGET_INCLUDED

#include <QtCore/QObject>
#include <QtQuick/QQuickItem>

namespace gui_qt {

class widget : public QObject {
    Q_OBJECT

public:

    explicit widget (QObject* parent = 0) : QObject(parent) {}
    Q_INVOKABLE int test() {
        return 1;
    }

public slots:

    void cppSlot(const QVariant &v);

};

}

#endif

