#include "equations_screens.hpp"
#include "ui_font.hpp"
#include "lp_printing.hpp"
#include "result_tables.hpp"
#include <QAbstractTableModel>
#include <QGroupBox>
#include <QSignalBlocker>
#include <QTabWidget>
#include <QPainter>
#include <QPrinter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QPlainTextEdit>
#include <QMenuBar>
#include <QMenu>
#include <QLabel>
#include <QDialog>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QTextLayout>
#include <QTextOption>
#include <memory>
namespace tora::desktop {
namespace {
void failure(EquationsController& c,const QString& e){if(!e.isEmpty())c.dialogs().showError("Linear Equations",e);}
void solve(ApplicationShell& shell,bool inverse){auto& c=equationsController(shell);QString e;if(c.solve(e,inverse))shell.navigate("equations/result");else failure(c,e);}
QWidget* input(ApplicationShell& shell){
 auto& c=equationsController(shell);auto* page=new QWidget;auto* layout=new QVBoxLayout(page);auto* form=new QFormLayout;
 auto* title=new QLineEdit;title->setObjectName("equations-title");auto* count=new QSpinBox;count->setRange(1,100);count->setValue(2);count->setObjectName("equations-variables");auto* kind=new QComboBox;kind->setObjectName("equations-kind");kind->addItems({"Linear Equations Solution","Matrix Inversion"});form->addRow("Problem",kind);form->addRow("Title",title);form->addRow("Number of variables",count);layout->addLayout(form);
 auto* init=new QPushButton("Initialize");init->setObjectName("equations-initialize");layout->addWidget(init);
 auto* grid=new QTableWidget;grid->setObjectName("equations-input-grid");grid->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);layout->addWidget(grid,1);
 auto updating=std::make_shared<bool>(false);
 auto render=[&c,title,count,grid,updating,kind]{*updating=true;if(const auto* d=c.document()){title->setText(QString::fromStdString(d->title()));kind->setCurrentIndex(d->matrixOnly()?1:0);const int n=static_cast<int>(d->augmented().rows());count->setValue(n);grid->setRowCount(n);grid->setColumnCount(d->matrixOnly()?n:n+1);for(int i=0;i<n;++i){grid->setVerticalHeaderItem(i,new QTableWidgetItem(QString("Equation %1").arg(i+1)));for(int j=0;j<grid->columnCount();++j)grid->setItem(i,j,new QTableWidgetItem(QString::fromStdString(d->cell(i,j))));}for(int j=0;j<n;++j)grid->setHorizontalHeaderItem(j,new QTableWidgetItem(QString("x%1").arg(j+1)));if(!d->matrixOnly())grid->setHorizontalHeaderItem(n,new QTableWidgetItem("RHS"));}else{grid->setRowCount(0);grid->setColumnCount(0);title->clear();}grid->setEditTriggers(c.readOnly()?QAbstractItemView::NoEditTriggers:(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed | QAbstractItemView::AnyKeyPressed));title->setReadOnly(c.readOnly());*updating=false;};
 QObject::connect(&c,&EquationsController::inputReset,page,render);
 QObject::connect(init,&QPushButton::clicked,page,[&c,title,count,kind]{QString e;if(!c.initialize(title->text(),count->value(),e,kind->currentIndex()==1))failure(c,e);});
 QObject::connect(grid,&QTableWidget::cellChanged,page,[&c,grid,updating](int i,int j){if(*updating)return;QString e;if(!c.editCell(i,j,grid->item(i,j)->text(),e))failure(c,e);});
 // Moving focus commits the active delegate before save, solve, or navigation.
 auto commit=[&c,title,grid]{grid->setFocus();QString e;if(!c.setTitle(title->text(),e)){failure(c,e);return;}if(c.readOnly())return;for(int i=0;i<grid->rowCount();++i)for(int j=0;j<grid->columnCount();++j)if(!c.editCell(i,j,grid->item(i,j)->text(),e)){failure(c,e);return;}};QObject::connect(&c,&EquationsController::commitRequested,page,commit);
 auto* row=new QHBoxLayout;auto* equations=new QPushButton("Linear Equations Solution");equations->setObjectName("equations-solve");auto* inverse=new QPushButton("Matrix Inversion");inverse->setObjectName("equations-invert");row->addWidget(equations);row->addWidget(inverse);layout->addLayout(row);
 QObject::connect(equations,&QPushButton::clicked,page,[&shell]{solve(shell,false);});QObject::connect(inverse,&QPushButton::clicked,page,[&shell]{solve(shell,true);});
 auto eligible=[&c,equations,inverse,init]{equations->setEnabled(c.document()&&!c.document()->matrixOnly());inverse->setEnabled(c.document());init->setEnabled(!c.readOnly());};QObject::connect(&c,&EquationsController::changed,page,eligible);eligible();render();polishUsableFonts(*page);return page;
}
const std::vector<equations::Step>& equationSteps(const EquationsController& controller)
{
    static const std::vector<equations::Step> empty;
    if (const auto* result = controller.result()) return result->trace;
    if (const auto* inverse = controller.inverse()) return inverse->trace;
    return empty;
}

bool equationTraceTruncated(const EquationsController& controller)
{
    if (const auto* result = controller.result()) return result->traceTruncated;
    return controller.inverse() && controller.inverse()->traceTruncated;
}

QString equationOperation(const equations::Step& step)
{
    const auto factor = QString::number(step.factor, 'g', 12);
    switch (step.operation) {
    case equations::Operation::Swap:
        return QStringLiteral("Swap rows %1 and %2").arg(step.row + 1).arg(step.otherRow + 1);
    case equations::Operation::Scale:
        return QStringLiteral("Multiply row %1 by %2").arg(step.row + 1).arg(factor);
    case equations::Operation::Eliminate:
        return QStringLiteral("Add (%1 × row %2) to row %3").arg(factor).arg(step.otherRow + 1).arg(step.row + 1);
    }
    return {};
}

QString equationStepLabel(const equations::Step& step, std::size_t index)
{
    return QStringLiteral("Step %1: %2").arg(index + 1).arg(equationOperation(step));
}

QStringList equationColumns(int width, bool matrixOnly, bool inverseTrace = false)
{
    QStringList headers;
    const int variables = inverseTrace ? width / 2 : width - (matrixOnly ? 0 : 1);
    for (int column = 0; column < variables; ++column)
        headers << (matrixOnly ? QStringLiteral("Column %1") : QStringLiteral("x%1")).arg(column + 1);
    if (inverseTrace) {
        for (int column = 0; column < variables; ++column)
            headers << QStringLiteral("Augmented %1").arg(column + 1);
    } else if (!matrixOnly) headers << QStringLiteral("RHS");
    return headers;
}

void showEquationMatrix(QTableWidget& table, const core::Matrix& matrix, const QStringList& headers,
                        const core::NumberFormatter& format)
{
    table.clearContents();
    table.verticalHeader()->show();
    table.setRowCount(static_cast<int>(matrix.rows()));
    table.setColumnCount(static_cast<int>(matrix.columns()));
    table.setHorizontalHeaderLabels(headers);
    for (int row = 0; row < table.rowCount(); ++row)
        for (int column = 0; column < table.columnCount(); ++column)
            putResultCell(table, row, column, QString::fromStdString(format.format(matrix(row,column))));
}

class EquationTraceModel final : public QAbstractTableModel {
public:
    EquationTraceModel(const EquationsController& controller, QObject* parent)
        : QAbstractTableModel(parent), controller_(controller) {}

    void show(std::optional<std::size_t> step)
    {
        beginResetModel();
        step_ = step;
        const auto& steps = equationSteps(controller_);
        if (steps.empty()) {
            rowsPerStep_ = stepCount_ = 0;
            headers_.clear();
        } else {
            rowsPerStep_ = static_cast<int>(steps.front().augmented.rows());
            stepCount_ = static_cast<int>(steps.size());
            const bool inverse = controller_.inverse() != nullptr;
            headers_ = equationColumns(static_cast<int>(steps.front().augmented.columns()), inverse, inverse);
            if (!step_) headers_.prepend(QStringLiteral("Row operation"));
            format_ = core::NumberFormatter(controller_.document()->format());
        }
        endResetModel();
    }

    int rowCount(const QModelIndex& parent = {}) const override
    {
        return parent.isValid() ? 0 : rowsPerStep_ * (step_ ? std::min(1,stepCount_) : stepCount_);
    }
    int columnCount(const QModelIndex& parent = {}) const override
    {
        return parent.isValid() ? 0 : static_cast<int>(headers_.size());
    }
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override
    {
        if (role != Qt::DisplayRole) return {};
        if (orientation == Qt::Horizontal) return headers_.value(section);
        return rowsPerStep_ ? QVariant(section % rowsPerStep_ + 1) : QVariant{};
    }
    QVariant data(const QModelIndex& index, int role) const override
    {
        if (!index.isValid() || !rowsPerStep_) return {};
        const bool operation = !step_ && index.column() == 0;
        if (role == Qt::TextAlignmentRole)
            return static_cast<int>((operation ? Qt::AlignLeft : Qt::AlignRight) | Qt::AlignVCenter);
        if (role != Qt::DisplayRole && role != Qt::ToolTipRole) return {};
        const auto& steps = equationSteps(controller_);
        const auto number = step_.value_or(static_cast<std::size_t>(index.row() / rowsPerStep_));
        if (number >= steps.size()) return {};
        const auto& step = steps[number];
        const int row = index.row() % rowsPerStep_;
        if (operation) return row == 0 ? equationStepLabel(step, number) : QString{};
        const int column = index.column() - (step_ ? 0 : 1);
        if (static_cast<std::size_t>(row) >= step.augmented.rows() || static_cast<std::size_t>(column) >= step.augmented.columns())
            return {};
        return QString::fromStdString(format_.format(step.augmented(row,column)));
    }

private:
    const EquationsController& controller_;
    std::optional<std::size_t> step_;
    QStringList headers_;
    core::NumberFormatter format_;
    int rowsPerStep_ = 0;
    int stepCount_ = 0;
};

QLabel* resultLabel(QBoxLayout& layout, const QString& name, bool bold = false)
{
    auto* label = new QLabel;
    label->setObjectName(name);
    label->setTextFormat(Qt::PlainText);
    label->setWordWrap(true);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    if (bold) {
        auto font = label->font();
        font.setBold(true);
        label->setFont(font);
    }
    layout.addWidget(label);
    return label;
}

QString determinantText(const equations::InverseResult& result, const core::NumberFormatter& format)
{
    if (result.determinant)
        return QStringLiteral("Determinant = %1").arg(QString::fromStdString(format.format(*result.determinant)));
    return QStringLiteral("Determinant outside the numeric range. Sign = %1. Log magnitude = %2.")
        .arg(result.determinantSign).arg(static_cast<double>(result.logAbsDeterminant), 0, 'g', 12);
}

QWidget* output(ApplicationShell& shell){
 auto& c=equationsController(shell);auto* page=new QWidget;auto* layout=new QVBoxLayout(page);auto* text=new QPlainTextEdit;text->setReadOnly(true);text->setObjectName("equations-result");layout->addWidget(text,1);
 auto* trace=new QPlainTextEdit;trace->setReadOnly(true);trace->setObjectName("equations-trace");layout->addWidget(trace,1);auto index=std::make_shared<std::size_t>(0);
 auto* row=new QHBoxLayout;auto* previous=new QPushButton("Previous Iteration");previous->setObjectName("equations-previous");auto* next=new QPushButton("Next Iteration");next->setObjectName("equations-next");auto* all=new QPushButton("All Iterations");all->setObjectName("equations-all");auto* print=new QPushButton("Print");print->setObjectName("equations-print");row->addWidget(previous);row->addWidget(next);row->addWidget(all);row->addWidget(print);layout->addLayout(row);
 auto matrix=[&c](const equations::Step& step,std::size_t i){QString s=QString("Step %1: ").arg(i+1);if(step.operation==equations::Operation::Swap)s+=QString("swap rows %1 and %2").arg(step.row+1).arg(step.otherRow+1);else if(step.operation==equations::Operation::Scale)s+=QString("row %1 *= %2").arg(step.row+1).arg(QString::number(step.factor,'g',12));else s+=QString("row %1 += %2 * row %3").arg(step.row+1).arg(QString::number(step.factor,'g',12)).arg(step.otherRow+1);s+="\n";core::NumberFormatter format(c.document()->format());for(std::size_t r=0;r<step.augmented.rows();++r){for(std::size_t col=0;col<step.augmented.columns();++col)s+=QString::fromStdString(format.format(step.augmented(r,col)))+"  ";s+="\n";}return s;};
 auto render=[&c,text,trace,index,previous,next,all,matrix]{text->setPlainText(c.outputText());const auto* steps=c.result()?&c.result()->trace:c.inverse()?&c.inverse()->trace:nullptr;const bool truncated=c.result()?c.result()->traceTruncated:c.inverse()?c.inverse()->traceTruncated:false;const bool available=steps&&!steps->empty();trace->setVisible(available);if(available){*index=std::min(*index,steps->size()-1);trace->setPlainText(matrix((*steps)[*index],*index)+(truncated?"\nTrace storage limit reached; solve completed.\n":""));}previous->setEnabled(available&&*index>0);next->setEnabled(available&&*index+1<steps->size());all->setEnabled(available);};
 QObject::connect(&c,&EquationsController::changed,page,[index,render]{*index=0;render();});QObject::connect(previous,&QPushButton::clicked,page,[index,render]{if(*index>0)--*index;render();});QObject::connect(next,&QPushButton::clicked,page,[index,render]{++*index;render();});QObject::connect(all,&QPushButton::clicked,page,[&c,trace,matrix]{const auto* steps=c.result()?&c.result()->trace:c.inverse()?&c.inverse()->trace:nullptr;if(!steps)return;QString s;for(std::size_t i=0;i<steps->size();++i)s+=matrix((*steps)[i],i)+"\n";if(c.result()?c.result()->traceTruncated:c.inverse()->traceTruncated)s+="Trace storage limit reached; solve completed.\n";trace->setPlainText(s);});
 QObject::connect(print,&QPushButton::clicked,page,[&c,&shell,trace]{auto outcome=requestLpPrint(&shell,[&c,trace](QPrinter& p){return printEquations(p,c,trace->isVisible()?trace->toPlainText():QString());});if(outcome==PrintOutcome::Failed)failure(c,"The print job could not be completed.");});
 auto* back=new QPushButton("View/Modify Input Data");layout->addWidget(back);QObject::connect(back,&QPushButton::clicked,page,[&shell]{shell.navigate("equations/input");});render();polishUsableFonts(*page);return page;
}
void numberFormat(ApplicationShell& shell){
 auto& c=equationsController(shell);QDialog dialog(&shell);dialog.setWindowTitle("Numeric Format");auto* layout=new QFormLayout(&dialog);auto* notation=new QComboBox;notation->addItems({"Decimal","Scientific"});auto* n=new QSpinBox;n->setRange(0,core::NumberFormatter::MaximumDigits);auto* d=new QSpinBox;d->setRange(0,core::NumberFormatter::MaximumDigits);const auto old=c.document()?c.document()->format():core::NumberFormat{};notation->setCurrentIndex(old.notation==core::Notation::Decimal?0:1);n->setValue(old.n);d->setValue(old.d);layout->addRow("Notation",notation);layout->addRow("N",n);layout->addRow("D",d);auto* buttons=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);layout->addRow(buttons);QObject::connect(buttons,&QDialogButtonBox::accepted,&dialog,&QDialog::accept);QObject::connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);if(dialog.exec()!=QDialog::Accepted)return;QString e;if(!c.setFormat({notation->currentIndex()==0?core::Notation::Decimal:core::Notation::Scientific,n->value(),d->value()},e))failure(c,e);
}
void menus(ApplicationShell& shell,QMenuBar& bar){auto& c=equationsController(shell);auto* file=bar.addMenu("File");file->addAction("New",&shell,[&shell,&c]{if(c.leaveInput()){c.beginNew();shell.navigate("equations/input");}});file->addAction("Open…",&shell,[&shell,&c]{if(c.requestOpen())shell.navigate("equations/input");});file->addAction("Save As…",&c,[&c]{c.requestSave();});file->addAction("Numeric Format…",&shell,[&shell]{numberFormat(shell);});auto* menu=bar.addMenu("Solve Problem");auto* equations=menu->addAction("Linear Equations Solution",&shell,[&shell]{solve(shell,false);});auto* inverse=menu->addAction("Matrix Inversion",&shell,[&shell]{solve(shell,true);});auto update=[&c,equations,inverse]{equations->setEnabled(c.document()&&!c.document()->matrixOnly());inverse->setEnabled(c.document());};QObject::connect(&c,&EquationsController::changed,menu,update);update();bar.addAction("MAIN Menu",&shell,[&shell]{shell.navigate("main");});}
}
ModuleDescriptor equationsModule(){ModuleDescriptor m{"equations","Linear Equations","equations/input",{}, {}};m.screens.append({"equations/input","LINEAR EQUATIONS",input,menus});m.screens.append({"equations/result","LINEAR EQUATIONS",output,menus});m.beforeLeave=[](ApplicationShell& shell,const QString&,const QString& to){return to.startsWith("equations/")?equationsController(shell).commitInput():equationsController(shell).leaveInput();};return m;}
bool printEquations(QPrinter& printer,const EquationsController& c,const QString& selectedTrace){
 if(!c.result()&&!c.inverse())return false;
 QPainter p;if(!p.begin(&printer))return false;
 const auto area=printer.pageLayout().paintRectPixels(printer.resolution());auto font=p.font();font.setPointSize(10);p.setFont(font);qreal y=area.top();
 const auto output=c.outputText()+(selectedTrace.isEmpty()?QString():QString("\nSELECTED ITERATIONS\n")+selectedTrace);
 for(const auto& paragraph:output.split('\n')){
  QTextLayout layout(paragraph.isEmpty()?QString(" "):paragraph,font,&printer);QTextOption option;option.setWrapMode(QTextOption::WrapAnywhere);layout.setTextOption(option);layout.beginLayout();
  while(true){auto line=layout.createLine();if(!line.isValid())break;line.setLineWidth(area.width());if(y+line.height()>area.bottom()){if(!printer.newPage())return false;y=area.top();}line.draw(&p,QPointF(area.left(),y));y+=line.height()+3;}
  layout.endLayout();
 }
 return p.end();
}
}
