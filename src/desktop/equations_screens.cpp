#include "equations_screens.hpp"
#include "ui_font.hpp"
#include "lp_printing.hpp"
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
