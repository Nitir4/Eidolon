#include "lp_input_controller.hpp"
#include "ui_font.hpp"
#include <QApplication>
#include <QCheckBox>
#include <QColor>
#include <QFileDialog>
#include <QGridLayout>
#include <QMessageBox>
#include <QRegularExpression>
#include <QWidget>
#include <QFont>
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace tora::desktop {
namespace {
using tora::compat::ToraLpCodec;
using tora::compat::LpFileStore;
constexpr char16_t cp1252[] = {0x20ac,0x81,0x201a,0x192,0x201e,0x2026,0x2020,0x2021,
    0x2c6,0x2030,0x160,0x2039,0x152,0x8d,0x17d,0x8f,0x90,0x2018,0x2019,0x201c,
    0x201d,0x2022,0x2013,0x2014,0x2dc,0x2122,0x161,0x203a,0x153,0x9d,0x17e,0x178};
std::string encodedText(const QString& value) {
    std::string result;
    for (const auto ch : value) {
        const auto u=ch.unicode();
        if (u<0x80 || (u>=0xa0 && u<=0xff)) result+=static_cast<char>(u);
        else {
            const auto* at=std::find(std::begin(cp1252),std::end(cp1252),u);
            if(at==std::end(cp1252)) throw std::invalid_argument("Text cannot be represented in the reference Windows-1252 file profile");
            result+=static_cast<char>(0x80+(at-std::begin(cp1252)));
        }
    }
    return result;
}
std::filesystem::path nativePath(const QString& path) {
    const auto bytes=path.toUtf8();
#if defined(__cpp_char8_t)
    return std::filesystem::path(std::u8string(reinterpret_cast<const char8_t*>(bytes.constData()),static_cast<std::size_t>(bytes.size())));
#else
    return std::filesystem::u8path(bytes.toStdString());
#endif
}
QString displayPath(const std::filesystem::path& path) { const auto bytes=path.u8string();return QString::fromUtf8(reinterpret_cast<const char*>(bytes.data()),static_cast<qsizetype>(bytes.size())); }
QMessageBox::StandardButton message(QWidget* owner,QMessageBox::Icon icon,const QString& title,
        const QString& text,QMessageBox::StandardButtons buttons) {
    QMessageBox box(icon,title,text,buttons,owner);polishUsableFonts(box);
    return static_cast<QMessageBox::StandardButton>(box.exec());
}
double enteredNumber(const QString& text) {
    // The reference accepts nonnumeric entry as zero (VB Val). Preserve a
    // finite numeric prefix, including exponent notation, before formatting.
    static const QRegularExpression prefix(QStringLiteral(R"(^\s*([+-]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][+-]?\d+)?))"));
    const auto match=prefix.match(text);
    if(!match.hasMatch()) return 0;
    bool ok=false; const auto value=match.captured(1).toDouble(&ok);
    if(!ok || !std::isfinite(value)) throw std::invalid_argument("Numeric entry exceeds the supported finite range");
    return value;
}
}
QString legacyText(const std::string& bytes) {
    QString result;
    for(const unsigned char ch:bytes) result+=QChar(ch>=0x80 && ch<0xa0?cp1252[ch-0x80]:ch);
    return result;
}
LpInputController& inputController(QWidget& shell) {
    auto* controller=shell.findChild<LpInputController*>(QStringLiteral("lp-input-controller"),Qt::FindDirectChildrenOnly);
    if(!controller) {
        controller=new LpInputController(&shell);controller->setObjectName(QStringLiteral("lp-input-controller"));
        QObject::connect(controller,&LpInputController::changed,&shell,[controller,&shell] {
            shell.setWindowTitle(QStringLiteral("TORA Clone")+(controller->captionPath().isEmpty()?QString():QStringLiteral("  ")+controller->captionPath()));
        });
    }
    return *controller;
}
LpGridModel::LpGridModel(LpInputController& c) : QAbstractTableModel(&c),controller_(c) {}
int LpGridModel::rowCount(const QModelIndex& parent) const { return parent.isValid()||!controller_.document()?0:static_cast<int>(controller_.document()->problem.constraints.size())+5; }
int LpGridModel::columnCount(const QModelIndex& parent) const { return parent.isValid()||!controller_.document()?0:static_cast<int>(controller_.document()->problem.variables.size())+3; }
QVariant LpGridModel::headerData(int section,Qt::Orientation orientation,int role) const {
    if(role!=Qt::DisplayRole || orientation!=Qt::Horizontal || !controller_.document()) return {};
    const int n=columnCount()-3;
    if(section==0) return QString();
    if(section<=n) return QStringLiteral("x%1").arg(section);
    return section==n+1?QStringLiteral("Enter <, >, or ="):QStringLiteral("R.H.S.");
}
QVariant LpGridModel::data(const QModelIndex& index,int role) const {
    if(!index.isValid() || index.row()>=rowCount() || index.column()>=columnCount()) return {};
    const auto& model=controller_.document()->problem;
    const int m=static_cast<int>(model.constraints.size()),n=static_cast<int>(model.variables.size());
    const int r=index.row(),c=index.column();
    if(role==Qt::BackgroundRole) {
        const bool selected=(controller_.selectionAxis_==LpInputController::Axis::Row && r==controller_.selection_) ||
            (controller_.selectionAxis_==LpInputController::Axis::Column && c==controller_.selection_);
        return QColor(selected?"#cc0000":(c==0?"#00ffff":"#0000ff"));
    }
    if(role==Qt::ForegroundRole) return QColor(c==0?"#0000ff":"#ffffff");
    if(role==Qt::FontRole && c>0) {auto font=usableUiFont(QApplication::font());font.setBold(true);return font;}
    if(role==Qt::TextAlignmentRole) return static_cast<int>((c==0?Qt::AlignLeft:Qt::AlignRight)|Qt::AlignVCenter);
    if(role!=Qt::DisplayRole && role!=Qt::EditRole && role!=Qt::ToolTipRole) return {};
    if(c==0) {
        if(r==0) return QStringLiteral("Var. Name");
        if(r==1) return model.sense==tora::lp::ObjectiveSense::Maximize?QStringLiteral("Maximize"):QStringLiteral("Minimize");
        if(r<m+2) return QStringLiteral("Constr %1").arg(r-1);
        return QStringList{QStringLiteral("Lower Bound"),QStringLiteral("Upper Bound"),QStringLiteral("Unrestr'd (y/n)?")}.at(r-m-2);
    }
    const auto value=legacyText(controller_.cells_.at(static_cast<std::size_t>(r*(n+2)+c-1)));
    return c==n+1 && r>=2 && r<m+2?value.trimmed():value;
}
Qt::ItemFlags LpGridModel::flags(const QModelIndex& index) const {
    if(!index.isValid()) return Qt::NoItemFlags;
    auto result=Qt::ItemIsEnabled|Qt::ItemIsSelectable;
    const int n=columnCount()-3,m=rowCount()-5,r=index.row(),c=index.column();
    if(c>0 && (c<=n || (r>=2 && r<m+2))) result|=Qt::ItemIsEditable;
    return result;
}
bool LpGridModel::setData(const QModelIndex& index,const QVariant& value,int role) {
    return role==Qt::EditRole && (flags(index)&Qt::ItemIsEditable) && controller_.editCell(index.row(),index.column(),value.toString());
}
void LpGridModel::refresh() { beginResetModel();endResetModel(); }
void LpGridModel::valuesChanged() { if(rowCount()) emit dataChanged(index(0,0),index(rowCount()-1,columnCount()-1)); }
void LpGridModel::selectionChanged() { if(rowCount()) emit dataChanged(index(0,0),index(rowCount()-1,columnCount()-1),{Qt::BackgroundRole}); }

LpInputController::LpInputController(QWidget* owner):QObject(owner),grid_(*this) {
    dialogs_.showError=[owner](const QString& title,const QString& body){message(owner,QMessageBox::Critical,title,body,QMessageBox::Ok);};
    dialogs_.askRetry=[owner](const QString& title,const QString& body){return message(owner,QMessageBox::Warning,title,body,QMessageBox::Yes|QMessageBox::No)==QMessageBox::Yes;};
    dialogs_.askSave=[owner]{const auto answer=message(owner,QMessageBox::Question,QStringLiteral("Save?"),QStringLiteral("Do you wish to save data?"),QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
        return answer==QMessageBox::Yes?SaveChoice::Yes:(answer==QMessageBox::No?SaveChoice::No:SaveChoice::Cancel);};
    dialogs_.chooseOpen=[owner]()->std::optional<OpenSelection>{
        QFileDialog picker(owner,QStringLiteral("Open"));picker.setFileMode(QFileDialog::ExistingFile);
        picker.setNameFilter(QStringLiteral("Text Files (*.txt);;All Files (*)"));
        picker.setOption(QFileDialog::DontUseNativeDialog);
        auto* readOnly=new QCheckBox(QStringLiteral("Open as read-only"),&picker);
        if(auto* layout=qobject_cast<QGridLayout*>(picker.layout())) layout->addWidget(readOnly,layout->rowCount(),0,1,layout->columnCount());
        polishUsableFonts(picker);
        if(picker.exec()!=QDialog::Accepted || picker.selectedFiles().isEmpty()) return {};
        return OpenSelection{picker.selectedFiles().first(),readOnly->isChecked()};
    };
    dialogs_.chooseSave=[owner](const QString& current)->std::optional<QString>{
        QFileDialog picker(owner,QStringLiteral("Save As"),current);picker.setAcceptMode(QFileDialog::AcceptSave);
        picker.setNameFilter(QStringLiteral("Text Files (*.txt);;All Files (*)"));picker.setDefaultSuffix(QStringLiteral("txt"));
        polishUsableFonts(picker);
        if(picker.exec()!=QDialog::Accepted || picker.selectedFiles().isEmpty()) return {};
        return picker.selectedFiles().first();
    };
}
bool LpInputController::fail(const QString& title,const QString& message) {pendingCommitValid_=false;errorTitle_=title;error_=message;dialogs_.showError(title,message);return false;}
bool LpInputController::commitInput() {pendingCommitValid_=true;emit commitRequested();return pendingCommitValid_;}
void LpInputController::refresh() {cells_=document_?ToraLpCodec::gridText(*document_):std::vector<std::string>{};clearSelection();grid_.refresh();emit changed();}
void LpInputController::beginNew(tora::core::NumberFormat format) {newFormat_=format;document_.reset();clipboard_.clear();clipboardAxis_=Axis::None;refresh();}
bool LpInputController::initialize(const QString& title,int variables,int constraints) {
    const auto dimensions=tora::lp::LpValidator::dimensions(variables<0?0:static_cast<std::size_t>(variables),constraints<0?0:static_cast<std::size_t>(constraints));
    if(!dimensions.valid()) return fail(QString::fromStdString(dimensions.issues.front().title),QString::fromStdString(dimensions.issues.front().message));
    try {
        auto problem=tora::lp::LpProblem::blank(static_cast<std::size_t>(variables),static_cast<std::size_t>(constraints));problem.title=encodedText(title);
        if(document_) {auto next=*document_;next.problem=std::move(problem);const auto blank=ToraLpCodec::create(next.problem,next.inputFormat);ToraLpCodec::setGridText(next,ToraLpCodec::gridText(blank));document_=std::move(next);}
        else document_=ToraLpCodec::create(std::move(problem),newFormat_);
        clipboard_.clear();clipboardAxis_=Axis::None;refresh();return true;
    }catch(const std::exception& e){return fail(QStringLiteral("Input Data Error"),QString::fromUtf8(e.what()));}
}
bool LpInputController::setTitle(const QString& title) {
    if(!document_) return true;
    try {auto next=*document_;next.problem.title=encodedText(title);if(next.problem.title==document_->problem.title)return true;
        (void)ToraLpCodec::encode(next);document_=std::move(next);emit changed();return true;}
    catch(const std::exception& e){return fail(QStringLiteral("Input Data Error"),QString::fromUtf8(e.what()));}
}
bool LpInputController::openFile(const QString& path,bool readOnly) {
    try {auto next=LpFileStore::open(nativePath(path),readOnly);document_=std::move(next);captionPath_=path;preferredSavePath_=path;clipboard_.clear();clipboardAxis_=Axis::None;refresh();return true;}
    catch(const tora::compat::FileError& e){
        if(openingFromPicker_ && e.code()==tora::compat::FileErrorCode::Marker) {
            errorTitle_=QString::fromStdString(e.title());error_=QString::fromUtf8(e.what());
            retryOpen_=dialogs_.askRetry(errorTitle_,error_);return false;
        }
        return fail(QString::fromStdString(e.title()),QString::fromUtf8(e.what()));
    }
    catch(const std::exception& e){return fail(QStringLiteral("LP File Error"),QString::fromUtf8(e.what()));}
}
bool LpInputController::saveTo(const QString& path) {
    if(!document_) return false;
    try {LpFileStore::saveAs(*document_,nativePath(path));preferredSavePath_=path;emit changed();return true;}
    catch(const std::exception& e){return fail(QStringLiteral("LP File Error"),QString::fromUtf8(e.what()));}
}
bool LpInputController::requestOpen() {
    do {
        retryOpen_=false;const auto selected=dialogs_.chooseOpen();if(!selected)return false;
        openingFromPicker_=true;const bool opened=openFile(selected->path,selected->readOnly);openingFromPicker_=false;
        if(opened)return true;
    }while(retryOpen_);
    return false;
}
bool LpInputController::requestSave() {if(!commitInput() || !document_)return false;const auto path=dialogs_.chooseSave(preferredSavePath_.isEmpty()?displayPath(document_->sourcePath()):preferredSavePath_);return path && saveTo(*path);}
bool LpInputController::leaveInput() {if(!commitInput())return false;if(!document_)return true;const auto choice=dialogs_.askSave();return choice==SaveChoice::No || (choice==SaveChoice::Yes && requestSave());}
bool LpInputController::validateForSolve() {
    if(!commitInput())return false;
    if(!document_)return false;
    for(std::size_t i=0;i<document_->problem.constraints.size();++i) {
        const auto& coefficients=document_->problem.constraints[i].coefficients;
        if(std::all_of(coefficients.begin(),coefficients.end(),[](double x){return x==0;}))
            return fail(QStringLiteral("Input Data Error"),QStringLiteral("All Left-hand side coefficients of Constraint %1 are\nzero.  Reenter or delete constraint").arg(i+1));
    }
    return true;
}
bool LpInputController::replaceGrid(std::vector<std::string> cells) {
    try {auto next=*document_;ToraLpCodec::setGridText(next,cells);document_=std::move(next);cells_=std::move(cells);clearSelection();grid_.valuesChanged();emit changed();return true;}
    catch(const std::exception& e){return fail(QStringLiteral("Input Data Error"),QString::fromUtf8(e.what()));}
}
bool LpInputController::editCell(int row,int column,const QString& value) {
    if(!document_)return false;
    const int n=static_cast<int>(document_->problem.variables.size()),m=static_cast<int>(document_->problem.constraints.size()),j=column-1;
    auto cells=cells_;const auto index=[n](int r,int c){return static_cast<std::size_t>(r*(n+2)+c);};
    try {
        std::string text;
        if(row==0) text=encodedText(value);
        else if(row==m+4) {
            const auto flag=value.trimmed().toLower();
            if(flag!=QStringLiteral("n") && flag!=QStringLiteral("y")) return false;
            text=flag.toStdString();cells[index(m+2,j)]=flag==QStringLiteral("y")?"-infinity":tora::core::NumberFormatter(document_->inputFormat).format(0);
            cells[index(m+3,j)]="infinity";
        } else if(column==n+1) {
            const auto relation=value.trimmed();
            if(relation==QStringLiteral("<") || relation==QStringLiteral("<="))text="<=";
            else if(relation==QStringLiteral(">") || relation==QStringLiteral(">="))text=">=";
            else if(relation==QStringLiteral("="))text="=";
            else return false;
        } else if(row==m+3 && value.trimmed().compare(QStringLiteral("infinity"),Qt::CaseInsensitive)==0)text="infinity";
        else if(row==m+2 && document_->problem.variables[static_cast<std::size_t>(j)].unrestricted && value.trimmed()==QStringLiteral("-infinity"))text="-infinity";
        else text=tora::core::NumberFormatter(document_->inputFormat).format(enteredNumber(value));
        cells[index(row,j)]=std::move(text);return replaceGrid(std::move(cells));
    }catch(const std::exception& e){return fail(QStringLiteral("Input Data Error"),QString::fromUtf8(e.what()));}
}
bool LpInputController::toggleObjective() {if(!document_)return false;document_->problem.sense=document_->problem.sense==tora::lp::ObjectiveSense::Maximize?tora::lp::ObjectiveSense::Minimize:tora::lp::ObjectiveSense::Maximize;refresh();return true;}
void LpInputController::selectRow(int row) {selectionAxis_=Axis::Row;selection_=row;grid_.selectionChanged();emit selectionChanged();}
void LpInputController::selectColumn(int column) {selectionAxis_=Axis::Column;selection_=column;grid_.selectionChanged();emit selectionChanged();}
void LpInputController::clearSelection() {selectionAxis_=Axis::None;selection_=-1;grid_.selectionChanged();emit selectionChanged();}
bool LpInputController::canCopy() const {
    if(!document_)return false;
    return (selectionAxis_==Axis::Row && selection_>=0 && selection_<grid_.rowCount()) ||
        (selectionAxis_==Axis::Column && selection_>0 && selection_<=grid_.columnCount()-3);
}
bool LpInputController::canPaste() const {
    return canCopy() && clipboardAxis_==selectionAxis_ && clipboard_.size()==static_cast<std::size_t>(selectionAxis_==Axis::Row?grid_.columnCount()-1:grid_.rowCount());
}
bool LpInputController::copySelection() {
    if(!canCopy())return false;
    clipboard_.clear();clipboardAxis_=selectionAxis_;
    const int stride=grid_.columnCount()-1;
    if(selectionAxis_==Axis::Row) for(int j=0;j<stride;++j)clipboard_.push_back(cells_[static_cast<std::size_t>(selection_*stride+j)]);
    else for(int i=0;i<grid_.rowCount();++i)clipboard_.push_back(cells_[static_cast<std::size_t>(i*stride+selection_-1)]);
    emit selectionChanged();return true;
}
bool LpInputController::pasteSelection() {
    if(!canPaste())return false;
    auto cells=cells_;const int stride=grid_.columnCount()-1;
    if(selectionAxis_==Axis::Row) for(int j=0;j<stride;++j)cells[static_cast<std::size_t>(selection_*stride+j)]=clipboard_[static_cast<std::size_t>(j)];
    else for(int i=0;i<grid_.rowCount();++i)cells[static_cast<std::size_t>(i*stride+selection_-1)]=clipboard_[static_cast<std::size_t>(i)];
    const auto axis=selectionAxis_;const int selected=selection_;
    if(!replaceGrid(std::move(cells)))return false;
    if(axis==Axis::Row)selectRow(selected);else selectColumn(selected);
    return true;
}
bool LpInputController::deleteSelection() {return reshape(false,true);}
bool LpInputController::insertSelection(bool before) {return reshape(true,before);}
bool LpInputController::reshape(bool insert,bool before) {
    if(!canCopy())return false;
    const int n=grid_.columnCount()-3,m=grid_.rowCount()-5;
    const bool column=selectionAxis_==Axis::Column;
    if(!column && (selection_<2 || selection_>=m+2))return false;
    const int position=column?selection_-1:selection_;
    const int count=column?n:m;
    // Observed Delete leaves a two-variable grid unchanged, but deletes from
    // three variables to two. Dimension entry can still create one variable.
    if(!insert && count<=(column?2:1)){clearSelection();emit editRequested(0,1);return false;}
    if(insert && count==100)return fail(QStringLiteral("Problem size"),QStringLiteral("Student version limits maximum size to 100"));
    try {
        auto next=*document_;auto& p=next.problem;
        const int at=position+(insert && !before?1:0);
        if(column) {
            if(insert) {p.variables.insert(p.variables.begin()+at,tora::lp::Variable{});p.objective.insert(p.objective.begin()+at,0);for(auto& r:p.constraints)r.coefficients.insert(r.coefficients.begin()+at,0);}
            else {p.variables.erase(p.variables.begin()+at);p.objective.erase(p.objective.begin()+at);for(auto& r:p.constraints)r.coefficients.erase(r.coefficients.begin()+at);}
        }else {
            if(insert) p.constraints.insert(p.constraints.begin()+at-2,tora::lp::Constraint{std::vector<double>(static_cast<std::size_t>(n)),tora::lp::Relation::LessEqual,0});
            else p.constraints.erase(p.constraints.begin()+at-2);
        }
        auto cells=ToraLpCodec::gridText(ToraLpCodec::create(p,next.inputFormat));
        const int nn=static_cast<int>(p.variables.size()),mm=static_cast<int>(p.constraints.size());
        for(int r=0;r<mm+5;++r)for(int c=0;c<nn+2;++c) {
            if(column) {
                if(insert && c==at)continue;
                const int oldc=c+(insert?(c>at?-1:0):(c>=at?1:0));
                cells[static_cast<std::size_t>(r*(nn+2)+c)]=cells_[static_cast<std::size_t>(r*(n+2)+oldc)];
            }else {
                if(insert && r==at)continue;
                const int oldr=r+(insert?(r>at?-1:0):(r>=at?1:0));
                cells[static_cast<std::size_t>(r*(nn+2)+c)]=cells_[static_cast<std::size_t>(oldr*(n+2)+c)];
            }
        }
        if(insert && !column)cells[static_cast<std::size_t>(at*(nn+2)+nn)]="<=";
        ToraLpCodec::setGridText(next,cells);document_=std::move(next);refresh();
        emit editRequested(insert && !column?at:0,insert && column?at+1:1);return true;
    }catch(const std::exception& e){return fail(QStringLiteral("Input Data Error"),QString::fromUtf8(e.what()));}
}
} // namespace tora::desktop
