/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Mobility Components.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QDECLARATIVEGEOMAPMOUSEAREA_H
#define QDECLARATIVEGEOMAPMOUSEAREA_H

#include "qdeclarativecoordinate_p.h"
#include "qdeclarative3dgraphicsgeomap_p.h"
#include "qdeclarativegeomapmouseevent_p.h"

// !!! IMPORTANT !!!
//
// Inheriting from QSGItem here
// is just a workaround to have non-gui related (ie where visualization is not
// the main thing) autotests to pass in QML2 environment.
// Real QML2 Map support (and related map object is a work in progress elsewhere.
// This Map element instantiates but does not do anything meaningful from app dev
// perspective.
//
// !!! IMPORTANT !!!

#include <QtDeclarative/QSGItem>

QT_BEGIN_NAMESPACE

class QDeclarativeGeoMapMouseArea : public QSGItem
{
    Q_OBJECT

    Q_PROPERTY(qreal mouseX READ mouseX NOTIFY mousePositionChanged)
    Q_PROPERTY(qreal mouseY READ mouseY NOTIFY mousePositionChanged)
    Q_PROPERTY(bool containsMouse READ hovered NOTIFY hoveredChanged)
    Q_PROPERTY(bool pressed READ pressed NOTIFY pressedChanged)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(Qt::MouseButton pressedButton READ pressedButton NOTIFY pressedButtonChanged)
    Q_PROPERTY(Qt::MouseButtons acceptedButtons READ acceptedButtons WRITE setAcceptedButtons NOTIFY acceptedButtonsChanged)
    Q_PROPERTY(bool hoverEnabled READ hoverEnabled WRITE setHoverEnabled NOTIFY hoverEnabledChanged)

public:
    QDeclarativeGeoMapMouseArea(QSGItem *parent = 0);
    ~QDeclarativeGeoMapMouseArea();

    void setMap(QDeclarative3DGraphicsGeoMap *map);
    QDeclarative3DGraphicsGeoMap* map() const;

    qreal mouseX() const;
    qreal mouseY() const;
    bool hovered() const;
    bool pressed() const;

    bool isEnabled() const;
    void setEnabled(bool enabled);

    Qt::MouseButton pressedButton() const;

    bool hoverEnabled() const;
    void setHoverEnabled(bool hoverEnabled);

    void setAcceptedButtons(Qt::MouseButtons acceptedButtons);
    Qt::MouseButtons acceptedButtons() const;

    void doubleClickEvent(QDeclarativeGeoMapMouseEvent *event);
    void pressEvent(QDeclarativeGeoMapMouseEvent *event);
    void releaseEvent(QDeclarativeGeoMapMouseEvent *event);
    void enterEvent();
    void exitEvent();
    void moveEvent(QDeclarativeGeoMapMouseEvent *event);

Q_SIGNALS:
    void mousePositionChanged();
    void hoveredChanged(bool hovered);
    void pressedChanged(bool pressed);
    void enabledChanged(bool enabled);
    void pressedButtonChanged(Qt::MouseButtons pressedButton);
    void acceptedButtonsChanged(Qt::MouseButtons acceptedButtons);
    void hoverEnabledChanged(bool hoverEnabled);

    void positionChanged(QDeclarativeGeoMapMouseEvent *mouse);
    void pressed(QDeclarativeGeoMapMouseEvent *mouse);
//    void pressAndHold(QDeclarativeGeoMapMouseEvent *mouse);
    void released(QDeclarativeGeoMapMouseEvent *mouse);
    void clicked(QDeclarativeGeoMapMouseEvent *mouse);
    void doubleClicked(QDeclarativeGeoMapMouseEvent *mouse);
    void entered();
    void exited();
//    void cancelled();

private:
    bool setPressed(bool pressed, QDeclarativeGeoMapMouseEvent *event);
    void setHovered(bool hovered);

    bool hovered_;
    bool enabled_;
    bool hoverEnabled_;
    qreal mouseX_;
    qreal mouseY_;
    bool pressed_;
    bool longPress_;
    bool doubleClick_;
    Qt::MouseButtons acceptedButtons_;
    Qt::MouseButton pressedButton_;
    Qt::KeyboardModifiers modifiers_;

//    qreal startX_;
//    qreal startY_;
//    QPointF lastPos_;
//    QPointF lastScenePos_;
//    Qt::MouseButton lastButton_;
//    Qt::MouseButtons lastButtons_;
//    Qt::KeyboardModifiers lastModifiers_;

    QDeclarative3DGraphicsGeoMap* map_;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QT_PREPEND_NAMESPACE(QDeclarativeGeoMapMouseArea));

#endif
