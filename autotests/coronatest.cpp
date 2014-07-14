/********************************************************************************
*   Copyright 2010 by Martin Blumenstingl <darklight.xdarklight@googlemail.com> *
*                                                                               *
*   This library is free software; you can redistribute it and/or               *
*   modify it under the terms of the GNU Library General Public                 *
*   License as published by the Free Software Foundation; either                *
*   version 2 of the License, or (at your option) any later version.            *
*                                                                               *
*   This library is distributed in the hope that it will be useful,             *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of              *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU            *
*   Library General Public License for more details.                            *
*                                                                               *
*   You should have received a copy of the GNU Library General Public License   *
*   along with this library; see the file COPYING.LIB.  If not, write to        *
*   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,        *
*   Boston, MA 02110-1301, USA.                                                 *
*********************************************************************************/

#include "coronatest.h"
#include <ksycoca.h>
#include <QStandardPaths>
#include <QApplication>

Plasma::Applet *SimpleLoader::internalLoadApplet(const QString &name, uint appletId,
                                   const QVariantList &args)
{
    return new SimpleApplet();
}

SimpleCorona::SimpleCorona(QObject *parent)
    : Plasma::Corona(parent)
{
    Plasma::PluginLoader::setPluginLoader(new SimpleLoader);
}

SimpleCorona::~SimpleCorona()
{}


QRect SimpleCorona::screenGeometry(int screen) const
{
    //completely arbitrary, still not tested
    return QRect(100*screen, 100, 100, 100);
}

SimpleApplet::SimpleApplet(QObject *parent , const QString &serviceId, uint appletId)
    : Plasma::Applet(parent, serviceId, appletId)
{
    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());

    //updateConstraints(Plasma::Types::UiReadyConstraint);
    m_timer.setSingleShot(true);
    m_timer.setInterval(qrand() % ((500 + 1) - 100) + 100);
    m_timer.start();
    connect(&m_timer, &QTimer::timeout, [=]() {
        updateConstraints(Plasma::Types::UiReadyConstraint);
    });
}



static void runKBuildSycoca()
{
    QProcess proc;
    const QString kbuildsycoca = QStandardPaths::findExecutable(KBUILDSYCOCA_EXENAME);
    QVERIFY(!kbuildsycoca.isEmpty());
    QStringList args;
    args << "--testmode";
    proc.setProcessChannelMode(QProcess::MergedChannels); // silence kbuildsycoca output
    proc.start(kbuildsycoca, args);

    //qDebug() << "waiting for signal";
    //QSignalSpy spy(KSycoca::self(), SIGNAL(databaseChanged(QStringList)));
    //QVERIFY(spy.wait(10000));
    //qDebug() << "got signal";

    proc.waitForFinished();
    QCOMPARE(proc.exitStatus(), QProcess::NormalExit);
}


void CoronaTest::initTestCase()
{
    if (!KSycoca::isAvailable()) {
        runKBuildSycoca();
    }

    QStandardPaths::setTestModeEnabled(true);
    m_corona = new SimpleCorona;

    m_configDir = QDir(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation));
    m_configDir.removeRecursively();

    QVERIFY(m_configDir.mkpath("."));

    QVERIFY(QFile::copy(QStringLiteral(":/plasma-test-appletsrc"), m_configDir.filePath(QStringLiteral("plasma-test-appletsrc"))));
}

void CoronaTest::cleanupTestCase()
{
    m_configDir.removeRecursively();
}

void CoronaTest::restore()
{
    m_corona->loadLayout("plasma-test-appletsrc");
    QCOMPARE(m_corona->containments().count(), 1);
    QCOMPARE(m_corona->containments().first()->applets().count(), 2);
}

void CoronaTest::startupCompletion()
{
    QVERIFY(!m_corona->isStartupCompleted());
    QVERIFY(!m_corona->containments().first()->isUiReady());

    QSignalSpy spy(m_corona, SIGNAL(startupCompleted()));
    QVERIFY(spy.wait(1000));

    QVERIFY(m_corona->isStartupCompleted());
    QVERIFY(m_corona->containments().first()->isUiReady());

    //TODO: applet creation and deletion
}

//this test has to be the last, since systemimmutability
//can't be programmatically unlocked
void CoronaTest::immutability()
{
    //immutability
    QCOMPARE(m_corona->immutability(), Plasma::Types::Mutable);
    m_corona->setImmutability(Plasma::Types::UserImmutable);
    QCOMPARE(m_corona->immutability(), Plasma::Types::UserImmutable);

    m_corona->setImmutability(Plasma::Types::Mutable);
    QCOMPARE(m_corona->immutability(), Plasma::Types::Mutable);

    m_corona->setImmutability(Plasma::Types::SystemImmutable);
    QCOMPARE(m_corona->immutability(), Plasma::Types::SystemImmutable);
    //can't unlock systemimmutable
    m_corona->setImmutability(Plasma::Types::Mutable);
    QCOMPARE(m_corona->immutability(), Plasma::Types::SystemImmutable);
}

QTEST_MAIN(CoronaTest)

