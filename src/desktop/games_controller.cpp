#include "games_screens.hpp"
#include "model_json.hpp"
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QCheckBox>
#include <QGridLayout>
#include <cmath>
namespace tora::desktop {
GamesController::GamesController(QObject* parent):QObject(parent){setObjectName("games-controller");auto* owner=qobject_cast<QWidget*>(parent);dialogs_.showError=[owner](const QString& a,const QString& b){QMessageBox::warning(owner,a,b);};dialogs_.askSave=[owner]{auto a=QMessageBox::question(owner,"Save?","Do you wish to save data?",QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);return a==QMessageBox::Yes?SaveChoice::Yes:a==QMessageBox::No?SaveChoice::No:SaveChoice::Cancel;};dialogs_.chooseSave=[owner](const QString& current)->std::optional<QString>{auto p=QFileDialog::getSaveFileName(owner,"Save As",current,"Game TXT (*.txt);;Game JSON (*.json);;All Files (*)");return p.isEmpty()?std::nullopt:std::optional<QString>(p);};dialogs_.chooseOpen=[owner]()->std::optional<OpenSelection>{QFileDialog d(owner,"Open Game Problem");d.setFileMode(QFileDialog::ExistingFile);d.setNameFilter("Games JSON / TXT (*.json *.txt);;All Files (*)");d.setOption(QFileDialog::DontUseNativeDialog);auto* ro=new QCheckBox("Open as read-only",&d);if(auto* l=qobject_cast<QGridLayout*>(d.layout()))l->addWidget(ro,l->rowCount(),0,1,l->columnCount());if(d.exec()!=QDialog::Accepted||d.selectedFiles().isEmpty())return {};return OpenSelection{d.selectedFiles().first(),ro->isChecked()};};}
bool GamesController::reject(QString& e,const QString& m){e=m;pendingValid_=false;return false;}
void GamesController::modified(){dirty_=true;result_.reset();graph_.reset();emit changed();}
void GamesController::beginNew(core::NumberFormat f){(void)core::NumberFormatter(f);format_=f;document_.reset();undo_.reset();selection_=-1;clipboard_.clear();result_.reset();graph_.reset();dirty_=readOnly_=false;path_.clear();emit inputReset();emit changed();}
bool GamesController::initialize(const QString& title,int r,int c,QString& e){e.clear();if(readOnly_)return reject(e,"Save a copy before editing this read-only game.");try{auto next=GamesDocument::blank(r,c,title.toStdString(),format_);document_=std::move(next);undo_.reset();selection_=-1;modified();emit inputReset();return true;}catch(const std::exception& x){return reject(e,x.what());}}
bool GamesController::editCell(int r,int c,const QString& v,QString& e){e.clear();if(!document_)return reject(e,"Initialize a game first.");try{if(document_->cell(r,c)==v.toStdString())return true;if(readOnly_)return reject(e,"Save a copy before editing this read-only game.");document_->setCell(r,c,v.toStdString());undo_.reset();modified();return true;}catch(const std::exception& x){return reject(e,x.what());}}
bool GamesController::setTitle(const QString& title,QString& e){e.clear();if(!document_)return true;if(document_->title()==title.toStdString())return true;if(readOnly_)return reject(e,"Save a copy before editing this read-only game.");try{document_->setTitle(title.toStdString());undo_.reset();modified();return true;}catch(const std::exception& x){return reject(e,x.what());}}
bool GamesController::openFile(const QString& p,QString& e,bool ro){e.clear();QFile f(p);if(!f.open(QIODevice::ReadOnly))return reject(e,f.errorString());if(f.size()>4*1024*1024)return reject(e,"Game file exceeds the 4 MiB limit.");try{auto bytes=f.readAll();if(f.error()!=QFileDevice::NoError)return reject(e,f.errorString());auto next=bytes.trimmed().startsWith("{")?decodeGameJson(bytes):GamesDocument::decode(std::string_view(bytes.constData(),static_cast<std::size_t>(bytes.size())));document_=std::move(next);undo_.reset();selection_=-1;clipboard_.clear();format_=document_->format();path_=p;readOnly_=ro;dirty_=false;result_.reset();graph_.reset();emit inputReset();emit changed();return true;}catch(const std::exception& x){return reject(e,x.what());}}
bool GamesController::commitInput(){pendingValid_=true;emit commitRequested();return pendingValid_;}
bool GamesController::saveTo(const QString& p,QString& e){e.clear();if(!commitInput())return reject(e,"Correct the active input before saving.");if(!document_||p.isEmpty())return reject(e,"Initialize a game and choose a filename.");auto identity=[](const QString& s){QFileInfo f(s);return f.exists()?f.canonicalFilePath():f.absoluteFilePath();};if(readOnly_&&identity(p)==identity(path_))return reject(e,"The source is read-only. Save to a different file.");const auto bytes=p.endsWith(".json",Qt::CaseInsensitive)?encodeGameJson(*document_).toStdString():document_->encode();QSaveFile f(p);if(!f.open(QIODevice::WriteOnly)||f.write(bytes.data(),static_cast<qint64>(bytes.size()))!=static_cast<qint64>(bytes.size())||!f.commit())return reject(e,f.errorString());path_=p;dirty_=readOnly_=false;emit inputReset();emit changed();return true;}
bool GamesController::solve(QString& e,bool graphical){e.clear();if(!commitInput()||!document_)return reject(e,"Initialize and correct the game before solving.");try{if(graphical){auto next=games::solveGraphical(document_->payoff());graph_=std::move(next);result_=graph_->equilibrium;}else{auto next=games::solve(document_->payoff());result_=std::move(next);graph_.reset();}emit changed();return true;}catch(const std::exception& x){return reject(e,x.what());}}
bool GamesController::requestSave(){if(!commitInput()||!document_)return false;auto p=dialogs_.chooseSave(path_);if(!p)return false;QString e;if(saveTo(*p,e))return true;dialogs_.showError("Cannot Save Problem",e);return false;}
bool GamesController::leaveInput(){if(!commitInput())return false;if(!dirty_)return true;auto a=dialogs_.askSave();return a==SaveChoice::No||(a==SaveChoice::Yes&&requestSave());}
bool GamesController::requestOpen(){auto p=dialogs_.chooseOpen();if(!p||!leaveInput())return false;QString e;if(openFile(p->path,e,p->readOnly))return true;dialogs_.showError("Cannot Open Problem",e);return false;}
QString GamesController::outputText() const
{
    if (!document_ || !result_) return {};
    const core::NumberFormatter formatter(document_->format());
    const auto number = [&formatter](double value) { return QString::fromStdString(formatter.formatBound(value)); };
    QString text = QString::fromStdString(document_->title()) + "\nValue of game = " + number(result_->value) + '\n';
    const auto appendStrategies = [&text,&number](const auto& names, const auto& probabilities) {
        for (std::size_t i = 0; i < probabilities.size(); ++i)
            text += QString::fromStdString(names[i]) + " = " + number(probabilities[i]) + '\n';
    };
    text += "Player A strategies\n";
    appendStrategies(document_->rowNames(),result_->rowStrategy);
    text += "Player B strategies\n";
    appendStrategies(document_->columnNames(),result_->columnStrategy);
    if (graph_) {
        for (const auto& line : graph_->lines) {
            text += QString::fromStdString(document_->columnNames()[line.strategy]) + ": ";
            const double slope = line.atOne - line.atZero;
            if (std::isfinite(slope)) {
                text += number(line.atZero) + (slope < 0 ? " - " : " + ") + number(std::abs(slope)) + " p\n";
            } else {
                // Finite endpoints still describe the line when their difference exceeds binary64.
                text += QStringLiteral("(1 - p) × (%1) + p × (%2)\n").arg(number(line.atZero),number(line.atOne));
            }
        }
    } else {
        const auto lp = games::rowPlayerLp(document_->payoff());
        text += "\nMaximize v\n";
        for (const auto& row : lp.constraints) {
            for (std::size_t column = 0; column < row.coefficients.size(); ++column)
                text += number(row.coefficients[column]) + QString(" %1 ").arg(column == 0 ? "v" : QString("x%1").arg(column));
            text += (row.relation == lp::Relation::Equal ? "= " : "<= ") + number(row.rhs) + '\n';
        }
        text += "v unrestricted; x >= 0\n";
    }
    return text;
}
GamesController& gamesController(ApplicationShell& shell){if(auto* c=shell.findChild<GamesController*>("games-controller"))return *c;return *new GamesController(&shell);}
bool GamesController::setFormat(core::NumberFormat f,QString& e){e.clear();try{(void)core::NumberFormatter(f);if(f.n==format_.n&&f.d==format_.d&&f.notation==format_.notation)return true;if(readOnly_)return reject(e,"Save a copy before editing this read-only game.");format_=f;undo_.reset();if(document_){document_->setFormat(f);modified();}return true;}catch(const std::exception& x){return reject(e,x.what());}}
bool GamesController::selectLine(bool row,int at,bool before){if(!commitInput()||!document_||at<0||static_cast<std::size_t>(at)>=(row?document_->payoff().rows():document_->payoff().columns()))return false;selection_=at;selectionRow_=row;insertBefore_=before;emit changed();return true;}
bool GamesController::copySelection(){if(!commitInput()||!document_||!hasSelection())return false;clipboard_=document_->line(selectionRow_,selection_);clipboardRow_=selectionRow_;emit changed();return true;}
bool GamesController::editSelection(int operation,QString& e){e.clear();if(!commitInput())return reject(e,"Correct the active input before editing the grid.");if(readOnly_||!document_)return reject(e,"Open an editable game first.");try{if(operation==0){if(!undo_)return reject(e,"There is no grid operation to undo.");document_=std::move(undo_);undo_.reset();}else{if(!hasSelection())return reject(e,"Select a player strategy heading first.");auto next=*document_;if(operation==1){if(!canPaste())return reject(e,"Copy a matching strategy line first.");next.paste(selectionRow_,selection_,clipboard_);}else if(operation==2)next.erase(selectionRow_,selection_);else if(operation==3)next.insert(selectionRow_,selection_+(insertBefore_?0:1));else return reject(e,"Unknown grid operation.");undo_=document_;document_=std::move(next);}selection_=-1;modified();emit inputReset();return true;}catch(const std::exception& x){return reject(e,x.what());}}

}
