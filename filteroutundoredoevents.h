#ifndef FILTEROUTUNDOREDOEVENTS_H
#define FILTEROUTUNDOREDOEVENTS_H

#include <QObject>

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
