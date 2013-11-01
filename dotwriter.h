#ifndef DOTWRITER_H
#define DOTWRITER_H

#include "nineml_rootcomponentitem.h"

class DotWriter
{
public:
    DotWriter(RootComponentItem *root);
    ~DotWriter();

    bool writeDotFile(QString file_name);

private:
    void writeRegimes();
    void writeTransitions();

private:
    RootComponentItem *root;
    QTextStream *out;
};

#endif // DOTWRITER_H
