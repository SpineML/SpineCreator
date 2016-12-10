#include "SC_plotinfo.h"

SingleGraph::SingleGraph()
{
    this->type = "";
    this->source = "";
    this->index = -1;
}

SingleGraph::~SingleGraph()
{
}

SingleGraph::SingleGraph(const SingleGraph& rhs)
{
    this->type = rhs.type;
    this->source = rhs.source;
    this->index = rhs.index;
    this->data = rhs.data;
}

void
SingleGraph::setData (const QCPDataMap* d)
{
    this->data.clear();

    // QCPDataMap is a typedef in qcustomplot.h for QMap<double, QCPData>
    QMap<double, QCPData>::const_iterator i = d->constBegin();

    while (i != d->constEnd()) {
        this->data.insert(i.value().key, i.value().value);
        ++i;
    }
}

PlotInfo::PlotInfo()
{
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
    this->setupFrom (p);
}

PlotInfo::PlotInfo (const PlotInfo& rhs)
{
    this->xlabel = rhs.xlabel;
    this->ylabel = rhs.ylabel;
    this->xrangeupper = rhs.xrangeupper;
    this->xrangelower = rhs.xrangelower;
    this->yrangeupper = rhs.yrangeupper;
    this->yrangelower = rhs.yrangelower;
    this->xtickstep = rhs.xtickstep;
    this->ytickstep = rhs.ytickstep;
    this->xrangedrag = rhs.xrangedrag;
    this->yrangedrag = rhs.yrangedrag;
    this->xrangezoom = rhs.xrangezoom;
    this->yrangezoom = rhs.yrangezoom;
    this->xrangezoomfactor = rhs.xrangezoomfactor;
    this->yrangezoomfactor = rhs.yrangezoomfactor;
    this->xoffset = rhs.xoffset;
    this->yoffset = rhs.yoffset;
    this->title = rhs.getTitle();
    this->graphs = rhs.graphs;
}

PlotInfo::~PlotInfo()
{
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

    if (p->plotLayout()->hasElement(0, 0)) {
        QCPPlotTitle* t = (QCPPlotTitle*)p->plotLayout()->element(0, 0);
        this->title = t->text();
    }

    this->xlabel = p->xAxis->label();
    this->ylabel = p->yAxis->label();

    this->xtickstep = p->xAxis->tickStep();
    this->ytickstep = p->yAxis->tickStep();

    this->xrangelower = p->xAxis->range().lower;
    this->xrangeupper = p->xAxis->range().upper;
    this->yrangelower = p->yAxis->range().lower;
    this->yrangeupper = p->yAxis->range().upper;

    // These are the setting for the axis - can the axis be dragged, zoomed etc:
    this->xrangedrag = p->xAxis->axisRect()->rangeDrag();
    this->yrangedrag = p->yAxis->axisRect()->rangeDrag();
    this->xrangezoom = p->xAxis->axisRect()->rangeZoom();
    this->yrangezoom = p->yAxis->axisRect()->rangeZoom();
    this->xrangezoomfactor = p->xAxis->axisRect()->rangeZoomFactor(Qt::Horizontal);
    this->yrangezoomfactor = p->yAxis->axisRect()->rangeZoomFactor(Qt::Vertical);

    // Don't know what offset does; it's stored here but not restored
    // in viewGVpropertieslayout::setGraphSettingsCommon
    this->xoffset = p->xAxis->offset();
    this->yoffset = p->yAxis->offset();
}
