/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include "qplacemanager_p.h"
#include "qplacemanagerengine.h"
#include "qplacemanagerenginefactory.h"
#include <QPluginLoader>
#include <QDebug>
#include <QStringList>
#include <QCoreApplication>
#include <QObject>
#include <QProcess>
#include <QEventLoop>

#include "qmobilitypluginsearch.h"

QT_USE_NAMESPACE

QPlaceManagerPrivate::QPlaceManagerPrivate()
        : engine(0),
        errorCode(QPlaceManager::NoError),
        isConnected(false),
        q_ptr(0)
{
}

void QPlaceManagerPrivate::createEngine(const QString &managerName, const QMap<QString,QString> &parameters)
{
    Q_Q(QPlaceManager);

    if (!q->availableManagers().contains(managerName)) {
        errorCode = QPlaceManager::NotSupportedError;
        errorString = QString::fromLatin1("The places manager, %1, was not found").arg(managerName);
        qWarning() << errorString;
        engine = 0;
        return;
    }

    QList<QPlaceManagerEngineFactory *> candidates = QPlaceManagerPrivate::factories().values(managerName);

    foreach (QPlaceManagerEngineFactory *f, candidates)
    {
        engine = f->engine(parameters, &errorCode, &errorString);
        if (engine)
            break;
    }

    if (!engine) {
        if (errorCode == QPlaceManager::NoError) {
            errorCode = QPlaceManager::NotSupportedError;
            errorString = QLatin1String("The place manager could not return the requested engine instance");
        }
    }

    // connect signals from engine to public class
    q_ptr->connect(engine, SIGNAL(finished(QPlaceReply*)), SIGNAL(finished(QPlaceReply*)));
    q_ptr->connect(engine, SIGNAL(error(QPlaceReply*,QPlaceReply::Error)),
                   SIGNAL(error(QPlaceReply*,QPlaceReply::Error)));
}

QPlaceManagerEngine* QPlaceManagerPrivate::getEngine(const QPlaceManager* manager)
{
    if (manager)
        return manager->d->engine;
    return 0;
}

 QHash<QString, QPlaceManagerEngineFactory*>  QPlaceManagerPrivate::factories(bool reload)
 {
    static QHash<QString, QPlaceManagerEngineFactory*> factories;
    static bool alreadyDiscovered = false;

    if (reload  == true)
        alreadyDiscovered = false;

    if (!alreadyDiscovered) {
        loadStaticFactories(&factories);
        loadDynamicFactories(&factories);
        alreadyDiscovered = true;
    }
    return factories;
 }

void QPlaceManagerPrivate::loadDynamicFactories(QHash<QString, QPlaceManagerEngineFactory*> *factories)
{
    QStringList paths;
    paths << mobilityPlugins(QLatin1String("places"));

    QPluginLoader qpl;
    for (int i=0;i < paths.count(); ++i) {
        qpl.setFileName(paths.at(i));

        QPlaceManagerEngineFactory *f = qobject_cast<QPlaceManagerEngineFactory*>(qpl.instance());
        if (f) {
            QString name = f->managerName();

#if !defined QT_NO_DEBUG
            const bool showDebug = qgetenv("QT_DEBUG_PLUGINS").toInt() > 0;
            if (showDebug)
                qDebug() << "Dynamic: found an engine plugin with name" << name;
#endif
            factories->insertMulti(name,f);
        }
    }
}

void QPlaceManagerPrivate::loadStaticFactories(QHash<QString, QPlaceManagerEngineFactory*> *factories)
{
#if !defined QT_NO_DEBUG
    const bool showDebug = qgetenv("QT_DEBUG_PLUGINS").toInt() > 0;
#endif

    QObjectList staticPlugins = QPluginLoader::staticInstances();
    for (int i=0; i < staticPlugins.count(); ++i) {
        QPlaceManagerEngineFactory *f = qobject_cast<QPlaceManagerEngineFactory*>(staticPlugins.at(i));
        if (f){
            QString name = f->managerName();

#if !defined QT_NO_DEBUG
            if (showDebug)
                qDebug() << "Static: found an engine plugin with name" << name;
#endif
            if (!name.isEmpty()) {
                factories->insertMulti(name, f);
            }
        }

    }
}
