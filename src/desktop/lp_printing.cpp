#include "lp_printing.hpp"
#include "ui_font.hpp"
#include "lp_input_controller.hpp"
#include <QDateTime>
#include <QFontMetricsF>
#include <QPainter>
#include <QPrinter>
#include <QPrintDialog>
#include <QStringList>
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace tora::desktop {
PrintOutcome requestLpPrint(QWidget* owner,const std::function<bool(QPrinter&)>& render,
                            const QPrinterInfo& defaultPrinter) {
    QPrinter printer(defaultPrinter,QPrinter::HighResolution);
    // TORA writes directly to its default printer. If this machine has no
    // default, offer native selection/PDF output instead of a legacy crash.
    if(defaultPrinter.isNull()) {
        QPrintDialog dialog(&printer,owner);polishUsableFonts(dialog);
        if(dialog.exec()!=QDialog::Accepted)return PrintOutcome::Cancelled;
    }
    return render(printer)?PrintOutcome::Complete:PrintOutcome::Failed;
}
namespace {
class PrintPage {
public:
    PrintPage(QPrinter& printer,const QFont& font):printer_(printer) {
        if(!painter_.begin(&printer_))throw std::runtime_error("Printer is unavailable");
        Q_UNUSED(font);
        // Match the observed installed sans-serif print face. Qt substitutes
        // an installed font when Arial is unavailable; no font asset is copied.
        font_=usableUiFont(QFont(QStringLiteral("Arial"),9));painter_.setFont(font_);
        const QFontMetricsF metrics(font_,&printer_);
        line_=metrics.height();digit_=metrics.horizontalAdvance(QLatin1Char('0'));
        const auto area=printer_.pageLayout().paintRectPixels(printer_.resolution());
        width_=area.width()*.78;left_=(area.width()-width_)/2;bottom_=area.height()-line_*2;
        y_=line_;
    }
    bool finish(){return painter_.end();}
    void newPage() {
        if(!printer_.newPage())throw std::runtime_error("Printer rejected a page");
        y_=line_*5;
    }
    void space(qreal lines=1){y_+=lines*line_;}
    void rule() {ensure(line_*.1);painter_.drawLine(QPointF(left_,y_),QPointF(left_+width_,y_));}
    void text(const QString& value,Qt::Alignment alignment=Qt::AlignLeft) {
        ensure(line_);painter_.drawText(QRectF(left_,y_,width_,line_),alignment|Qt::AlignVCenter,value);y_+=line_;
    }
    void cells(const QStringList& values,const std::vector<qreal>& widths,bool numeric=true,bool basicRight=false) {
        ensure(line_);qreal x=left_;
        for(int j=0;j<values.size();++j) {
            const auto width=widths.at(static_cast<std::size_t>(j));
            painter_.drawText(QRectF(x,y_,width,line_),
                             ((numeric && j>0) || (basicRight && j==0)?Qt::AlignRight:Qt::AlignLeft)|Qt::AlignVCenter,values[j]);
            x+=width;
        }
        y_+=line_;
    }
    void original(const tora::lp::LpProblem& model,tora::core::NumberFormat format,int extraFieldDigits=0) {
        text(QStringLiteral("TORA Clone — Linear Programming"));
        text(QDateTime::currentDateTime().toString(QStringLiteral("dddd, MMMM dd, yyyy HH:mm")));
        space(4);text(QStringLiteral("LINEAR PROGRAM -- ORIGINAL DATA"),Qt::AlignHCenter);
        space(1);text(QStringLiteral("Title: ")+legacyText(model.title));rule();space(3);
        const tora::core::NumberFormatter f(format);
        const auto number=[&f](double value){return QString::fromStdString(f.formatBound(value));};
        const std::size_t count=model.variables.size();
        // Printed labels remain x1/x2 even when input aliases exist.
        const auto labelWidth=QFontMetricsF(font_,&printer_).horizontalAdvance(QStringLiteral("Unrestr'd (y/n)?"))+digit_*4;
        qreal valueWidth=std::max(digit_*(format.n+format.d+5+extraFieldDigits),digit_*10);
        const QFontMetricsF metrics(font_,&printer_);
        const auto widen=[&](double value){valueWidth=std::max(valueWidth,metrics.horizontalAdvance(number(value))+digit_);};
        for(auto value:model.objective)widen(value);
        for(const auto& row:model.constraints){for(auto value:row.coefficients)widen(value);widen(row.rhs);}
        for(const auto& v:model.variables){widen(v.lowerBound);widen(v.upperBound);}
        // A wide original model uses successive variable bands with repeated
        // row descriptions; the relation and RHS belong to the final band.
        const auto perBand=std::max<std::size_t>(1,static_cast<std::size_t>(std::max(qreal(1),std::floor((width_-labelWidth)/valueWidth))));
        for(std::size_t first=0;first<count;first+=perBand) {
            const auto last=std::min(count,first+perBand);
            const bool finalBand=last==count;
            const auto columns=last-first+(finalBand?2:0);
            const auto available=width_-labelWidth;
            const auto fitted=std::min(valueWidth,available/std::max<std::size_t>(1,columns));
            std::vector<qreal> widths(columns+1,fitted);widths[0]=labelWidth;
            QStringList header{QString()};for(auto j=first;j<last;++j)header<<QString::fromStdString(model.variableLabel(j));
            cells(header,widths);
            QStringList objective{model.sense==tora::lp::ObjectiveSense::Maximize?QStringLiteral("Maximize"):QStringLiteral("Minimize")};
            for(auto j=first;j<last;++j)objective<<number(model.objective[j]);
            cells(objective,widths);
            text(QStringLiteral("Subject to"));
            for(std::size_t i=0;i<model.constraints.size();++i) {
                const auto& row=model.constraints[i];QStringList values{QStringLiteral("( %1)").arg(i+1)};
                for(auto j=first;j<last;++j)values<<number(row.coefficients[j]);
                if(finalBand)values<<(row.relation==tora::lp::Relation::LessEqual?QStringLiteral("<="):row.relation==tora::lp::Relation::GreaterEqual?QStringLiteral(">="):QStringLiteral("="))<<number(row.rhs);
                cells(values,widths);
            }
            space();
            for(int row=0;row<3;++row) {
                QStringList values{row==0?QStringLiteral("Lower Bound"):row==1?QStringLiteral("Upper Bound"):QStringLiteral("Unrestr'd (y/n)?")};
                for(auto j=first;j<last;++j) {
                    const auto& v=model.variables[j];
                    values<<(row==0?number(v.lowerBound):row==1?number(v.upperBound):v.unrestricted?QStringLiteral("y"):QStringLiteral("n"));
                }
                cells(values,widths);
            }
            space(2);
        }
    }
    std::vector<qreal> summaryWidths() const {return std::vector<qreal>(5,width_/5);}
    qreal width() const {return width_;}
    qreal digit() const {return digit_;}
    void plot(const std::function<void(QPainter&,const QRectF&,const QFont&)>& draw) {
        const auto side=std::min(width_*.83,(bottom_-y_)/1.08);
        if(side<=0)throw std::runtime_error("No printable graph area");
        draw(painter_,QRectF(left_+(width_-side)/2,y_,side,side*1.08),font_);
    }
private:
    void ensure(qreal height){if(y_+height>bottom_)newPage();}
    QPrinter& printer_;QPainter painter_;QFont font_;
    qreal line_=0,digit_=0,width_=0,left_=0,bottom_=0,y_=0;
};
QStringList rowCells(const tora::lp::SimplexDisplayRow& row) {
    QStringList cells{QString::fromStdString(row.label)};
    for(const auto& value:row.values)cells<<QString::fromStdString(value);
    return cells;
}
}

bool printLpSummary(QPrinter& printer,const tora::lp::FinalSolutionSession& session,
                    tora::core::NumberFormat format,const QFont& font) {
    auto summary=session.display(format,true);
    if(summary.sections.empty())return false;
    try {
        // The original printer restores negative-lower-bound values in slack
        // calculations. Its screen instead retains the captured shifted-value
        // quirk. Keep that distinction confined to printer presentation.
        const tora::core::NumberFormatter formatter(format);
        const auto& problem=session.problem();
        for(std::size_t i=0;i<problem.constraints.size();++i) {
            const auto& row=problem.constraints[i];double lhs=0,shiftedRhs=row.rhs;
            for(std::size_t j=0;j<problem.variables.size();++j) {
                lhs+=row.coefficients[j]*session.values()[j];
                if(!problem.variables[j].unrestricted)shiftedRhs-=row.coefficients[j]*session.solver().problem().variables[j].lowerBound;
            }
            const auto gap=row.relation==tora::lp::Relation::GreaterEqual?lhs-row.rhs:row.rhs-lhs;
            auto slack=formatter.format(std::abs(gap));
            if(gap!=0 && row.relation!=tora::lp::Relation::Equal)
                slack+=(row.relation==tora::lp::Relation::GreaterEqual)!=(shiftedRhs<0)?"+":"-";
            summary.sections[1].rows[i].values[1]=slack;
        }
        PrintPage page(printer,font);page.original(session.problem(),format);page.newPage();
        page.text(QString::fromStdString(summary.heading),Qt::AlignHCenter);page.rule();
        page.text(QStringLiteral("Title: ")+legacyText(summary.title));
        page.text(QStringLiteral("Final Iteration No.: ")+QString::number(summary.iteration));
        auto objective=session.solver().history().back().tableau.rhs(0);
        for(std::size_t j=0;j<session.problem().variables.size();++j) {
            const auto& v=session.problem().variables[j];
            if(!v.unrestricted && v.lowerBound<0)objective+=session.problem().objective[j]*v.lowerBound;
        }
        page.text(QStringLiteral("Objective Value = ")+QString::number(objective,'g',15));page.rule();
        const auto widths=page.summaryWidths();
        for(const auto& section:summary.sections) {
            if(!section.heading.empty()){page.space(3);page.text(QString::fromStdString(section.heading),Qt::AlignHCenter);}
            page.rule();QStringList header;for(const auto& label:section.columns)header<<QString::fromStdString(label);
            page.cells(header,widths);page.rule();
            for(const auto& row:section.rows)page.cells(rowCells(row),widths);
            page.rule();
        }
        // The original printer omits the interactive alternative-optimum notice.
        return page.finish();
    }catch(const std::exception&){return false;}
}

bool printLpTrace(QPrinter& printer,const tora::lp::PrimalSimplexSession& session,
                  tora::core::NumberFormat format,const QFont& font) {
    try {
        using Method=tora::lp::PrimalMethod;
        const auto method=session.method();
        PrintPage page(printer,font);page.original(session.problem(),format,method==Method::TwoPhase?3:0);page.newPage();
        page.text(method==Method::MMethod?QStringLiteral("SIMPLEX TABLEAUS -- (M-Method)"):
                  method==Method::TwoPhase?QStringLiteral("SIMPLEX TABLEAUS -- (Two-Phase Method)"):
                  method==Method::Dual?QStringLiteral("SIMPLEX TABLEAUS -- (Dual Simplex Method)"):
                  QStringLiteral("SIMPLEX TABLEAUS -- (Starting All-Slack Method)"),Qt::AlignHCenter);
        page.space();page.text(QStringLiteral("Title: ")+legacyText(session.problem().title));page.space(2);
        // Phase labels require a wider printed field than Iteration labels.
        const auto field=page.digit()*(format.n+format.d+5+(method==Method::TwoPhase?3:0));
        const auto bandSize=std::max<std::size_t>(1,static_cast<std::size_t>(std::max(qreal(1),std::floor(page.width()/field)-1)));
        for(std::size_t index=0;index<session.history().size();++index) {
            const auto table=session.display(index,format);page.text(QString::fromStdString(table.heading));
            for(std::size_t first=0;first<table.columns.size();first+=bandSize) {
                const auto last=std::min(table.columns.size(),first+bandSize);
                const auto width=std::min(field,page.width()/static_cast<qreal>(last-first+1));
                const std::vector<qreal> widths(last-first+1,width);
                QStringList header{QStringLiteral("Basic")};for(auto j=first;j<last;++j)header<<QString::fromStdString(table.columns[j]);
                page.cells(header,widths,true,true);
                for(const auto& row:table.rows) {
                    bool populated=false;QStringList cells{QString::fromStdString(row.label)};
                    for(auto j=first;j<last;++j){populated=populated || !row.values[j].empty();cells<<QString::fromStdString(row.values[j]);}
                    if(populated)page.cells(cells,widths,true,true);
                }
                page.space(2);
            }
        }
        return page.finish();
    }catch(const std::exception&){return false;}
}
bool printLpGraph(QPrinter& printer,const tora::lp::GraphicalSession& session,const QFont& font,
                  const std::function<void(QPainter&,const QRectF&,const QFont&)>& draw) {
    try {
        PrintPage page(printer,font);page.original(session.problem(),session.format());page.newPage();
        page.text(QStringLiteral("LINEAR PROGRAMMING -- GRAPHICAL SOLUTION"),Qt::AlignHCenter);
        page.space();page.text(QStringLiteral("Title: ")+legacyText(session.problem().title));page.rule();page.space();
        if(session.objectiveVisible()) {
            page.text(QStringLiteral("Summary of Optimal Solution:"));
            const auto& result=session.result();
            if(result.message.empty()) {
                page.text(QStringLiteral("Objective Value = ")+QString::fromStdString(result.objective));
                page.text(QStringLiteral("x1 = ")+QString::fromStdString(result.x1));
                page.text(QStringLiteral("x2 = ")+QString::fromStdString(result.x2));
            } else page.text(QString::fromStdString(result.message));
        }
        page.rule();page.space(12);page.plot(draw);
        return page.finish();
    }catch(const std::exception&){return false;}
}
} // namespace tora::desktop
