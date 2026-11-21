#include "history_view.hpp"
#include "lp_printing.hpp"
#include <QComboBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QSaveFile>
#include <QMessageBox>
#include <QFontDatabase>
#include <QPrinter>
#include <QTextDocument>
namespace tora::desktop {
HistoryView::HistoryView(const QString& prefix,QWidget* parent):QWidget(parent) {
    auto* layout=new QVBoxLayout(this);selector_=new QComboBox;
    selector_->setObjectName(prefix+"-step-selector");selector_->setAccessibleName("Recorded calculation step");
    selector_->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);selector_->setMinimumContentsLength(20);
    layout->addWidget(selector_);text_=new QPlainTextEdit;text_->setReadOnly(true);
    text_->setLineWrapMode(QPlainTextEdit::NoWrap);text_->setObjectName(prefix+"-step-text");
    text_->setAccessibleName("Recorded calculation details");
    auto fixed=QFontDatabase::systemFont(QFontDatabase::FixedFont);fixed.setPointSize(12);text_->setFont(fixed);layout->addWidget(text_,1);
    auto* controls=new QHBoxLayout;layout->addLayout(controls);
    const auto button=[&](const QString& label,const QString& suffix){auto* b=new QPushButton(label);b->setObjectName(prefix+suffix);controls->addWidget(b);return b;};
    previous_=button("Previous","-step-previous");next_=button("Next","-step-next");auto* all=button("All steps","-step-all");
    auto* save=button("Export history","-step-export");auto* print=button("Print history","-step-print");
    connect(selector_,&QComboBox::currentIndexChanged,this,[this](int i){showStep(i);});
    connect(previous_,&QPushButton::clicked,this,[this]{selector_->setCurrentIndex(selector_->currentIndex()-1);});
    connect(next_,&QPushButton::clicked,this,[this]{selector_->setCurrentIndex(selector_->currentIndex()+1);});
    connect(all,&QPushButton::clicked,this,[this]{text_->setPlainText(completeText());});
    connect(save,&QPushButton::clicked,this,[this]{const auto path=QFileDialog::getSaveFileName(this,"Export complete recorded history",{},"Text (*.txt)");if(!path.isEmpty()&&!exportTo(path))QMessageBox::warning(this,"History","Could not export the complete history.");});
    connect(print,&QPushButton::clicked,this,[this]{if(requestLpPrint(this,[this](QPrinter& p){return printTo(p);})==PrintOutcome::Failed)QMessageBox::warning(this,"History","Could not print the complete history.");});
    showStep(-1);
}
void HistoryView::setHistory(const QStringList& titles,std::function<QString(int)> render) {
    selector_->clear();render_=std::move(render);selector_->addItems(titles);showStep(selector_->currentIndex());
}
void HistoryView::showStep(int index) {
    previous_->setEnabled(index>0);next_->setEnabled(index>=0&&index+1<selector_->count());
    text_->setPlainText(render_&&index>=0?render_(index):"No recorded calculation. Solve the model first.");
}
QString HistoryView::completeText()const {
    QStringList steps;for(int i=0;render_&&i<selector_->count();++i)steps<<render_(i);
    return steps.join("\n\n------------------------------------------------------------\n\n");
}
bool HistoryView::exportTo(const QString& path)const {
    if(selector_->count()==0)return false;
    QSaveFile file(path);const auto bytes=completeText().toUtf8();
    return file.open(QIODevice::WriteOnly)&&file.write(bytes)==bytes.size()&&file.commit();
}
bool HistoryView::printTo(QPrinter& printer)const {
    if(selector_->count()==0)return false;
    QTextDocument document;document.setDefaultFont(text_->font());document.setPlainText(completeText());document.print(&printer);
    return printer.printerState()!=QPrinter::Error&&printer.printerState()!=QPrinter::Aborted;
}
}
