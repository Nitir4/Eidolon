#include "transportation_printing.hpp"
#include "transportation_screens.hpp"
#include "ui_font.hpp"
#include <tora/core/number_formatter.hpp>
#include <QDateTime>
#include <QFontMetricsF>
#include <QPainter>
#include <QPrinter>
#include <QStringList>
#include <algorithm>
#include <stdexcept>

namespace tora::desktop {
namespace {
QString text(const std::string& value){return QString::fromStdString(value);}
class Page {
public:
    explicit Page(QPrinter& printer):printer_(printer) {
        if(!painter_.begin(&printer_))throw std::runtime_error("Printer is unavailable");
        font_=usableUiFont(QFont(QStringLiteral("Arial"),9));painter_.setFont(font_);
        line_=QFontMetricsF(font_,&printer_).height();
        const auto area=printer_.pageLayout().paintRectPixels(printer_.resolution());
        width_=area.width()*.78;left_=(area.width()-width_)/2;bottom_=area.height()-2*line_;y_=line_;
    }
    bool finish(){const bool ended=painter_.end();return ended && printer_.printerState()!=QPrinter::Error;}
    void newPage(){if(!printer_.newPage())throw std::runtime_error("Printer rejected a page");y_=5*line_;}
    void ensure(qreal height){if(y_+height>bottom_)newPage();}
    void space(qreal lines=1){y_+=line_*lines;}
    void rule(){painter_.drawLine(QPointF(left_,y_),QPointF(left_+width_,y_));}
    void line(const QString& value,Qt::Alignment alignment=Qt::AlignLeft) {
        ensure(line_);painter_.drawText(QRectF(left_,y_,width_,line_),alignment|Qt::AlignVCenter,value);space();
    }
    void cell(qreal x,qreal y,qreal width,qreal height,const QString& value,Qt::Alignment alignment,bool bold=false) {
        auto font=font_;font.setBold(bold);painter_.setFont(font);
        painter_.drawText(QRectF(x,y,width,height),alignment|Qt::AlignVCenter,value);painter_.setFont(font_);
    }
    void original(const transportation::Problem& p,bool finalStyle) {
        line(QStringLiteral("TORA Clone — Transportation"));
        line(QDateTime::currentDateTime().toString(QStringLiteral("dddd, MMMM dd, yyyy HH:mm")));
        space(4);line(finalStyle?QStringLiteral("TRANSPORTATION MODEL -- ORIGINAL DATA"):
            QStringLiteral("TRANSPORTATION MODEL-- ORIGINAL DATA"),Qt::AlignHCenter);
        space();line(QStringLiteral("Title: ")+text(p.title));rule();space(3);
        const tora::core::NumberFormatter f({tora::core::Notation::Decimal,5,2});
        const auto number=[&](double value){return text(f.formatBound(value));};
        const auto n=p.demand.size();
        for(std::size_t first=0;first<n;first+=5) {
            const auto last=std::min(n,first+5);const bool final=last==n;
            const auto columns=2+last-first+(final?1:0);const auto unit=width_/std::max<std::size_t>(7,columns);
            const auto start=left_+(width_-columns*unit)/2;
            ensure((p.supply.size()+4)*line_);
            for(auto j=first;j<last;++j)cell(start+(2+j-first)*unit,y_,unit,line_,QStringLiteral("d%1").arg(j+1),Qt::AlignRight);
            if(final)cell(start+(columns-1)*unit,y_,unit,line_,QStringLiteral("Supply"),Qt::AlignRight);
            space();
            if(first==0)cell(start+unit,y_,unit,line_,QStringLiteral("Name"),Qt::AlignLeft);
            for(auto j=first;j<last;++j)cell(start+(2+j-first)*unit,y_,unit,line_,text(p.destinationNames[j]).toLower(),Qt::AlignRight);
            space(2);
            for(std::size_t i=0;i<p.supply.size();++i) {
                ensure(line_);cell(start,y_,unit,line_,QStringLiteral("s%1").arg(i+1),Qt::AlignLeft);
                cell(start+unit,y_,unit,line_,text(p.sourceNames[i]).toLower(),Qt::AlignLeft);
                for(auto j=first;j<last;++j)cell(start+(2+j-first)*unit,y_,unit,line_,number(p.costs[i][j]),Qt::AlignRight);
                if(final)cell(start+(columns-1)*unit,y_,unit,line_,number(p.supply[i]),Qt::AlignRight,true);
                space();
            }
            space();ensure(line_);cell(start,y_,2*unit,line_,QStringLiteral("Demand"),Qt::AlignLeft);
            for(auto j=first;j<last;++j)cell(start+(2+j-first)*unit,y_,unit,line_,number(p.demand[j]),Qt::AlignRight,true);
            space(2);
        }
    }
    void tableau(const transportation::Problem& p,const transportation::Display& d,std::size_t iteration) {
        line(QStringLiteral("Iteration %1:     ObjVal     ").arg(iteration)+text(d.objective));space();
        const auto n=p.demand.size();std::size_t first=0;
        while(first<n) {
            const bool initial=first==0;const auto last=std::min(n,first+(initial?4:5));const bool final=last==n;
            const std::size_t labels=initial?3:1,columns=labels+last-first+(final?1:0);
            const auto unit=width_/std::max<std::size_t>(7,columns),start=left_+(width_-columns*unit)/2;
            ensure((3*p.supply.size()+5)*line_);auto top=y_;
            for(auto j=first;j<last;++j) {
                const auto x=start+(labels+j-first)*unit;
                cell(x,y_,unit,line_,QStringLiteral("D%1").arg(j+1),Qt::AlignRight);
                cell(x,y_+line_,unit,line_,text(p.destinationNames[j]),Qt::AlignRight);
                cell(x,y_+2*line_,unit,line_,text(d.v[j]),Qt::AlignLeft);
            }
            if(final)cell(start+(columns-1)*unit,y_,unit,line_,QStringLiteral("Supply"),Qt::AlignRight);
            if(initial)cell(start+unit,y_+line_,unit,line_,QStringLiteral("Name"),Qt::AlignCenter);
            space(3);horizontal(start,columns*unit);
            for(std::size_t i=0;i<p.supply.size();++i) {
                // A source's costs, allocations and reduced values stay together.
                if(y_+3*line_>bottom_) {
                    vertical(start,unit,columns,top,y_);newPage();
                    line(QStringLiteral("Tableau (continued)"));top=y_;
                }
                cell(start,y_,unit,3*line_,QStringLiteral("S%1").arg(i+1),Qt::AlignCenter);
                if(initial) {
                    cell(start+unit,y_,unit,3*line_,text(p.sourceNames[i]),Qt::AlignCenter);
                    cell(start+2*unit,y_,unit,3*line_,text(d.u[i]),Qt::AlignRight);
                }
                for(auto j=first;j<last;++j) {
                    const auto x=start+(labels+j-first)*unit;
                    cell(x,y_,unit,line_,text(d.costs[i][j]),Qt::AlignRight);
                    cell(x,y_+line_,unit,line_,text(d.shipments[i][j]),Qt::AlignCenter,true);
                    cell(x,y_+2*line_,unit,line_,text(d.reducedCosts[i][j]),Qt::AlignLeft);
                }
                if(final)cell(start+(columns-1)*unit,y_,unit,3*line_,text(d.supply[i]),Qt::AlignRight,true);
                space(3);horizontal(start,columns*unit);
            }
            vertical(start,unit,columns,top,y_);ensure(line_);
            if(initial)cell(start+unit,y_,unit,line_,QStringLiteral("Demand"),Qt::AlignCenter);
            for(auto j=first;j<last;++j)cell(start+(labels+j-first)*unit,y_,unit,line_,text(d.demand[j]),Qt::AlignCenter,true);
            space(2);first=last;
        }
    }
    void summary(const transportation::FinalSummary& output) {
        space(4);line(QStringLiteral("TRANSPORTATION MODEL OUTPUT SUMMARY"),Qt::AlignHCenter);rule();
        line(QStringLiteral("Title: ")+text(output.title));line(QStringLiteral("Final Iteration No.:  ")+text(output.iteration));
        line(QStringLiteral("Objective Value (minimum cost) =")+text(output.objective));rule();
        summaryHeader();
        for(const auto& row:output.rows) {
            if(y_+line_>bottom_){newPage();summaryHeader();}
            for(std::size_t col=0;col<5;++col)cell(left_+col*width_/5,y_,width_/5,line_,text(row[col]),col<2?Qt::AlignLeft:Qt::AlignRight);
            space();
        }
    }
private:
    void horizontal(qreal start,qreal width){painter_.drawLine(QPointF(start,y_),QPointF(start+width,y_));}
    void vertical(qreal start,qreal unit,std::size_t columns,qreal top,qreal bottom) {
        for(std::size_t col=1;col<=columns;++col)painter_.drawLine(QPointF(start+col*unit,top),QPointF(start+col*unit,bottom));
    }
    void summaryHeader() {
        const QStringList labels{QStringLiteral("From"),QStringLiteral("To"),QStringLiteral("Amt Shipped"),QStringLiteral("Obj Coeff"),QStringLiteral("Obj Contrib")};
        for(int col=0;col<5;++col)cell(left_+col*width_/5,y_,width_/5,line_,labels[col],col<2?Qt::AlignLeft:Qt::AlignRight);
        space();rule();
    }
    QPrinter& printer_;QPainter painter_;QFont font_;qreal line_=0,width_=0,left_=0,bottom_=0,y_=0;
};
QString methodHeading(transportation::StartMethod method) {
    const auto label=method==transportation::StartMethod::Vogel?QStringLiteral("Vogel's Method"):
        method==transportation::StartMethod::LeastCost?QStringLiteral("Least-Cost Method "):QStringLiteral("North-West Corner Method");
    return QStringLiteral("TRANSPORTATION MODEL -- TABLEAUS (")+label+QLatin1Char(')');
}
}
bool printTransportationInput(QPrinter& printer,const transportation::Problem& p,bool finalStyle) {
    try {transportation::validate(p);Page page(printer);page.original(p,finalStyle);return page.finish();}
    catch(const std::exception&){return false;}
}
bool printTransportationTrace(QPrinter& printer,const TransportationController& c,std::size_t shown) {
    if(!c.loaded() || shown==0 || shown>c.trace().size())return false;
    try {
        Page page(printer);page.original(c.problem(),false);
        for(std::size_t k=0;k<shown;++k) {
            page.newPage();
            if(k==0){page.line(methodHeading(c.method()),Qt::AlignHCenter);page.space();page.line(QStringLiteral("Title: ")+text(c.problem().title));page.rule();page.space(2);}
            page.tableau(c.problem(),transportation::display(c.problem(),c.trace()[k],k+1,k==0 && c.multipliersRefreshed()),k+1);
        }
        return page.finish();
    }catch(const std::exception&){return false;}
}
bool printTransportationSummary(QPrinter& printer,const transportation::FinalSummary& output) {
    if(output.rows.empty() || std::any_of(output.rows.begin(),output.rows.end(),[](const auto& row){return row.size()!=5;}))return false;
    try {Page page(printer);page.summary(output);return page.finish();}
    catch(const std::exception&){return false;}
}
PrintOutcome requestTransportationFinalPrint(QWidget* owner,const TransportationController& c,
    const QPrinterInfo& defaultPrinter,const PrintSelector& select) {
    if(!c.loaded() || !c.finalOutput())return PrintOutcome::Failed;
    const auto problem=c.problem();const auto summary=*c.finalOutput();
    const auto input=requestLpPrint(owner,[&](QPrinter& p){return printTransportationInput(p,problem,true);},defaultPrinter,select);
    if(input!=PrintOutcome::Complete)return input;
    return requestLpPrint(owner,[&](QPrinter& p){return printTransportationSummary(p,summary);},defaultPrinter,select);
}
} // namespace tora::desktop
