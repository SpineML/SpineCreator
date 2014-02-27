#include "filteroutundoredoevents.h"
#include <QEvent>
#include <QShortcutEvent>
#include <QDebug>
#include <QComboBox>

FilterOutUndoRedoEvents::FilterOutUndoRedoEvents(QObject *parent) :
    QObject(parent)
{
}

/*!
 * \brief FilterOutUndoRedoEvents::eventFilter
 * \param event
 * \return
 * This is a filter to prevent the QWidgets from using their built in undo and redo functions,
 * and to prevent the mousewheels changing the values when scrolling down the page.
 */
bool FilterOutUndoRedoEvents::eventFilter(QObject * obj, QEvent *event)
{
    const QEvent::Type type = event->type();
    switch (type)
    {
    case QEvent::ShortcutOverride:
    {
        QKeyEvent * key = static_cast<QKeyEvent *>(event);

        if (key->key() == Qt::Key_Z) {
            qDebug() << "Z";
            event->ignore();
            return true;
        } else {
            qDebug() << "not Z";
            event->accept();
            return false;
        }
    }
    case QEvent::Wheel:
    {
        // if we are attached to a ComboBox
        QComboBox * combo = qobject_cast<QComboBox*>(obj);
        if (combo) {
            // if the ComboBox is focused then allow scroll
            if (combo->hasFocus()) {
                event->accept();
                return false;
            } else {
                event->ignore();
                return true;
            }
        } else {
            // if it is not a combobox then deny scroll
            event->ignore();
            return true;
        }
    }
    default:
        event->accept();
        return false;
    }

    return false;
}
