#include "SC_plotinfo.h"

SingleGraph::SingleGraph()
{
    DBG() << "Constructor";
    this->type = "";
    this->source = "";
    this->index = -1;
    this->data = (QCPDataMap*)0;
}

SingleGraph::~SingleGraph()
{
    if (this->data != (QCPDataMap*)0) {
        DBG() << "Deconstructor (deletes data)";
        delete this->data;
    } else {
        DBG() << "Deconstructor (deletes no data)";
    }
}

SingleGraph::SingleGraph(const SingleGraph& rhs)
{
    this->type = rhs.type;
    this->source = rhs.source;
    this->index = rhs.index;
    this->setData (rhs.data);
}

void
SingleGraph::setData (QCPDataMap* d)
{
    // Want a copy of the data pointed to by d:
    this->data = new QCPDataMap(*d);
}

PlotInfo::PlotInfo()
{
    DBG() << "Constructor";
    this->xlabel = "";
    this->ylabel = "";
    this->title = "none";
}

PlotInfo::PlotInfo(QCustomPlot* p)
{
    DBG() << "Constructor (with QCustomPlot* arg)";
    this->setupFrom (p);
}

PlotInfo::PlotInfo (const PlotInfo& rhs)
{
    this->xlabel = rhs.xlabel;
    this->ylabel = rhs.ylabel;
    this->title = rhs.getTitle();
    this->graphs = rhs.graphs;
}

PlotInfo::~PlotInfo()
{
        DBG() << "Deconstructor";
}

QString
PlotInfo::getTitle (void) const
{
    return this->title;
}

void
PlotInfo::setupFrom (QCustomPlot* p)
{
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
