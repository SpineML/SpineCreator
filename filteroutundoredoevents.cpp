#include "filteroutundoredoevents.h"
#include <QEvent>
#include <QShortcutEvent>
#include <QDebug>

FilterOutUndoRedoEvents::FilterOutUndoRedoEvents(QObject *parent) :
    QObject(parent)
{
}

bool FilterOutUndoRedoEvents::eventFilter(QObject *, QEvent *event)
{
    const QEvent::Type type = event->type();
    switch (type)
    {
        case QEvent::Shortcut:
            {
        qDebug() << "moo";
            //QShortcutEvent * key = static_cast<QShortcutEvent *>(event);
            /*if (key->key() == QKeySequence::Undo || key->key() == QKeySequence::Redo) {
                return true;
            } else {
                return false;
            }*/
    }
    case QEvent::ShortcutOverride:
        {
        QKeyEvent * key = static_cast<QKeyEvent *>(event);

        if (key->key() == Qt::Key_Z) {
            qDebug() << "Z";
            return true;
        } else {
            qDebug() << "not Z";
            return false;
        }
    }
        default:
            return false;
    }
    return false;
}
