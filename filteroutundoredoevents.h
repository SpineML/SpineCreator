#ifndef FILTEROUTUNDOREDOEVENTS_H
#define FILTEROUTUNDOREDOEVENTS_H

#include <QObject>

/*!
 * \brief The FilterOutUndoRedoEvents class makes sure that undo / redo commands
 * are not sent to the individual QLineEdits etc but are sent to the global SC
 * undo / redo framework
 */

class FilterOutUndoRedoEvents : public QObject
{
    Q_OBJECT
public:
    explicit FilterOutUndoRedoEvents(QObject *parent = 0);
    bool eventFilter(QObject *, QEvent *event);
    
signals:
    
public slots:
    
};

#endif // FILTEROUTUNDOREDOEVENTS_H
