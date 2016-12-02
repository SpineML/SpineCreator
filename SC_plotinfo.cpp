#include "SC_plotinfo.h"

SingleGraph::SingleGraph()
{
    //DBG() << "Constructor for " << this;
    this->type = "";
    this->source = "";
    this->index = -1;
}

SingleGraph::~SingleGraph()
{
    DBG() << "Deconstructor (deletes no data) for " << this;
}

SingleGraph::SingleGraph(const SingleGraph& rhs)
{
    DBG() << "Copy constructor for " << this;
    this->type = rhs.type;
    this->source = rhs.source;
    this->index = rhs.index;
    this->data = rhs.data;
}

void
SingleGraph::setData (const QCPDataMap* d)
{
    DBG() << "Copy QCPDataMap...";
    this->data.clear();

    // QCPDataMap is a typedef in qcustomplot.h for QMap<double, QCPData>
    QMap<double, QCPData>::const_iterator i = d->constBegin();

    while (i != d->constEnd()) {
        this->data.insert((double)i.key(), (QCPData)i.value());
        ++i;
    }
}

PlotInfo::PlotInfo()
{
    //DBG() << "Constructor for " << this;
    this->xlabel = "";
    this->ylabel = "";
    this->xrangeupper = 0;
    this->xrangelower = 0;
    this->yrangeupper = 0;
    this->yrangelower = 0;
    this->xtickstep = 0;
    this->ytickstep = 0;
    this->title = "none";
}

PlotInfo::PlotInfo(QCustomPlot* p)
{
    //DBG() << "Constructor (with QCustomPlot* arg) for " << this;
    this->setupFrom (p);
}

PlotInfo::PlotInfo (const PlotInfo& rhs)
{
    //DBG() << "Copy constructor for " << this;
    this->xlabel = rhs.xlabel;
    this->ylabel = rhs.ylabel;
    this->xrangeupper = rhs.xrangeupper;
    this->xrangelower = rhs.xrangelower;
    this->yrangeupper = rhs.yrangeupper;
    this->yrangelower = rhs.yrangelower;
    this->xtickstep = rhs.xtickstep;
    this->ytickstep = rhs.ytickstep;
    this->title = rhs.getTitle();
    this->graphs = rhs.graphs;
}

PlotInfo::~PlotInfo()
{
    //DBG() << "Deconstructor for " << this;
}

QString
PlotInfo::getTitle (void) const
{
    //DBG() << "called for " << this;
    return this->title;
}

void
PlotInfo::setupFrom (QCustomPlot* p)
{
    //DBG() << "called for " << this;
    for (int i = 0; i < p->graphCount(); ++i) {
        SingleGraph g;
        g.type = (QString)p->graph(i)->property("type").toString();
        g.source = (QString)p->graph(i)->property("source").toString(); // logFileXMLname
        g.index = (int)p->graph(i)->property("index").toInt();
        g.setData (p->graph(i)->data());
        this->graphs.push_back (g);
    }

    if (p->plotLayout()->hasElement(0, 0)) {
        QCPPlotTitle* t = (QCPPlotTitle*)p->plotLayout()->element(0, 0);
        this->title = t->text();
    }

    this->xlabel = p->xAxis->label();
    this->ylabel = p->yAxis->label();

    this->xrangelower = p->xAxis->range().lower;
    this->xrangeupper = p->xAxis->range().upper;
    this->xrangelower = p->yAxis->range().lower;
    this->xrangeupper = p->yAxis->range().upper;

    this->xtickstep = p->xAxis->tickStep();
    this->ytickstep = p->yAxis->tickStep();
}
