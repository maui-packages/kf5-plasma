/***************************************************************************
 *   Copyright 2013 Sebastian Kügler <sebas@kde.org>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "tooltipdialog.h"

#include <QQmlEngine>
#include <QQuickItem>
#include <QDebug>
#include <QPropertyAnimation>

#include "framesvgitem.h"

#include <kdeclarative/qmlobject.h>

Q_GLOBAL_STATIC(ToolTipDialog, toolTipDialogInstance)

ToolTipDialog::ToolTipDialog(QQuickItem  *parent)
    : DialogProxy(parent),
      m_qmlObject(0),
      m_animation(0),
      m_hideTimeout(4000),
      m_direction(Plasma::Types::Up)
{
    setFlags(Qt::ToolTip);
    setLocation(Plasma::Types::Floating);
    setDirection(Plasma::Types::Up);
    m_frameSvgItem->setImagePath("widgets/tooltip");

    m_showTimer = new QTimer(this);
    m_showTimer->setSingleShot(true);
    connect(m_showTimer, &QTimer::timeout, [=]() {
        setVisible(false);
    });
}

ToolTipDialog::~ToolTipDialog()
{
}

QQuickItem *ToolTipDialog::loadDefaultItem()
{
    if (!m_qmlObject) {
        m_qmlObject = new QmlObject(this);
    }

    if (!m_qmlObject->rootObject()) {
        //HACK: search our own import
        foreach (const QString &path, m_qmlObject->engine()->importPathList()) {
            if (QFile::exists(path + "/org/kde/plasma/core")) {
                m_qmlObject->setSource(QUrl::fromLocalFile(path + "/org/kde/plasma/core/private/DefaultToolTip.qml"));
                break;
            }
        }
    }

    return qobject_cast<QQuickItem *>(m_qmlObject->rootObject());
}

Plasma::Types::Direction ToolTipDialog::direction() const
{
    return m_direction;
}

void ToolTipDialog::setDirection(Plasma::Types::Direction loc)
{
    m_direction = loc;
}

void ToolTipDialog::showEvent(QShowEvent *event)
{
    m_showTimer->start(m_hideTimeout);

    DialogProxy::showEvent(event);
}

void ToolTipDialog::adjustPosition(const QPoint &point)
{
    if (isVisible()) {
        if (!m_animation) {
            m_animation = new QPropertyAnimation(this);
            connect(m_animation, SIGNAL(valueChanged(QVariant)),
                    this, SLOT(valueChanged(QVariant)));
            m_animation->setTargetObject(this);
            m_animation->setEasingCurve(QEasingCurve::InOutQuad);
            m_animation->setDuration(250);
        }

        switch (m_direction) {
        case Plasma::Types::Left:
        case Plasma::Types::Right:
            setX(point.x());
            break;
        case Plasma::Types::Up:
        case Plasma::Types::Down:
            setY(point.y());
            break;
        default:
            break;
        }

        m_animation->setStartValue(position());
        m_animation->setEndValue(point);
        m_animation->start();
    } else {
        setPosition(point);
    }
}

void ToolTipDialog::dismiss()
{
    m_showTimer->start(m_hideTimeout / 20); // pretty short: 200ms
}

void ToolTipDialog::keepalive()
{
    m_showTimer->start(m_hideTimeout);
}

void ToolTipDialog::valueChanged(const QVariant &value)
{
    setPosition(value.value<QPoint>());
}

ToolTipDialog* ToolTipDialog::instance()
{
    return toolTipDialogInstance();
}

#include "moc_tooltipdialog.cpp"
