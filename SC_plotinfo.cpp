#include "SC_plotinfo.h"

SingleGraph::SingleGraph()
{
    DBG() << "Constructor for " << this;
    this->type = "";
    this->source = "";
    this->index = -1;
    this->data = (QCPDataMap*)0;
}

SingleGraph::~SingleGraph()
{
    if (this->data != (QCPDataMap*)0) {
        DBG() << "Deconstructor (deletes data) for " << this;
        delete this->data;
    } else {
        DBG() << "Deconstructor (deletes no data) for " << this;
    }
}

SingleGraph::SingleGraph(const SingleGraph& rhs)
{
    DBG() << "Copy constructor for " << this;
    this->type = rhs.type;
    this->source = rhs.source;
    this->index = rhs.index;
    this->setData (rhs.data);
}

void
SingleGraph::setData (QCPDataMap* d)
{
    // Want a copy of the data pointed to by d:
    DBG() << "called for " << this << " to set data map " << d;
    QCPDataMap& dm = *d;
    this->data = new QCPDataMap(dm);
    DBG() << "this->data is now " << this->data;
}

PlotInfo::PlotInfo()
{
    DBG() << "Constructor for " << this;
    this->xlabel = "";
    this->ylabel = "";
    this->title = "none";
}

PlotInfo::PlotInfo(QCustomPlot* p)
{
    DBG() << "Constructor (with QCustomPlot* arg) for " << this;
    this->setupFrom (p);
}

PlotInfo::PlotInfo (const PlotInfo& rhs)
{
    DBG() << "Copy constructor for " << this;
    this->xlabel = rhs.xlabel;
    this->ylabel = rhs.ylabel;
    this->title = rhs.getTitle();
    this->graphs = rhs.graphs;
}

PlotInfo::~PlotInfo()
{
    DBG() << "Deconstructor for " << this;
}

QString
PlotInfo::getTitle (void) const
{
    DBG() << "called for " << this;
    return this->title;
}

void
PlotInfo::setupFrom (QCustomPlot* p)
{
    DBG() << "called for " << this;
    for (int i = 0; i < p->graphCount(); ++i) {
        SingleGraph g;
        g.type = (QString)p->graph(i)->property("type").toString();
        g.source = (QString)p->graph(i)->property("source").toString(); // logFileXMLname
        g.index = (int)p->graph(i)->property("index").toInt();
        g.setData (p->graph(i)->data());
        this->graphs.push_back (g);
    }
    this->xlabel = p->xAxis->label();
    this->ylabel = p->yAxis->label();
    if (p->plotLayout()->hasElement(0, 0)) {
        QCPPlotTitle* t = (QCPPlotTitle*)p->plotLayout()->element(0, 0);
        this->title = t->text();
    }
}
