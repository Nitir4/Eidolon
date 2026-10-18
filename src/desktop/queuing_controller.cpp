#include "queuing_screens.hpp"
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QCheckBox>
#include <QGridLayout>
#include <algorithm>
#include <cmath>
namespace tora::desktop {
QueuingController::QueuingController(QObject* parent):QObject(parent){setObjectName("queuing-controller");auto* owner=qobject_cast<QWidget*>(parent);dialogs_.showError=[owner](const QString& a,const QString& b){QMessageBox::warning(owner,a,b);};dialogs_.askSave=[owner]{auto a=QMessageBox::question(owner,"Save?","Do you wish to save data?",QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);return a==QMessageBox::Yes?SaveChoice::Yes:a==QMessageBox::No?SaveChoice::No:SaveChoice::Cancel;};dialogs_.chooseSave=[owner](const QString& current)->std::optional<QString>{auto p=QFileDialog::getSaveFileName(owner,"Save As",current,"Text Files (*.txt);;All Files (*)");return p.isEmpty()?std::nullopt:std::optional<QString>(p);};dialogs_.chooseOpen=[owner]()->std::optional<OpenSelection>{QFileDialog d(owner,"Open System Problem");d.setFileMode(QFileDialog::ExistingFile);d.setOption(QFileDialog::DontUseNativeDialog);auto* ro=new QCheckBox("Open as read-only",&d);if(auto* l=qobject_cast<QGridLayout*>(d.layout()))l->addWidget(ro,l->rowCount(),0,1,l->columnCount());if(d.exec()!=QDialog::Accepted||d.selectedFiles().isEmpty())return {};return OpenSelection{d.selectedFiles().first(),ro->isChecked()};};}
bool QueuingController::reject(QString& e,const QString& m){e=m;pendingValid_=false;return false;}
void QueuingController::modified(){dirty_=true;results_.clear();transients_.clear();originalComparisons_.clear();emit changed();}
void QueuingController::beginNew(core::NumberFormat f){(void)core::NumberFormatter(f);format_=f;document_.reset();undo_.reset();clipboard_.reset();selection_=-1;results_.clear();transients_.clear();originalComparisons_.clear();dirty_=readOnly_=false;path_.clear();emit inputReset();emit changed();}
bool QueuingController::initialize(const QString& title,QString& e){e.clear();if(readOnly_)return reject(e,"Save a copy before editing this read-only system.");try{auto next=QueuingDocument::blank(title.toStdString(),format_);document_=std::move(next);undo_.reset();selection_=-1;modified();emit inputReset();return true;}catch(const std::exception& x){return reject(e,x.what());}}
bool QueuingController::editCell(int r,int c,const QString& v,QString& e){e.clear();if(!document_)return reject(e,"Initialize a system first.");if(readOnly_)return reject(e,"Save a copy before editing this read-only system.");try{if(document_->cell(r,c)==v.toStdString())return true;document_->setCell(r,c,v.toStdString());undo_.reset();modified();return true;}catch(const std::exception& x){return reject(e,x.what());}}
bool QueuingController::setTitle(const QString& title,QString& e){e.clear();if(!document_)return true;if(document_->title()==title.toStdString())return true;if(readOnly_)return reject(e,"Save a copy before editing this read-only system.");try{document_->setTitle(title.toStdString());undo_.reset();modified();return true;}catch(const std::exception& x){return reject(e,x.what());}}
bool QueuingController::openFile(const QString& p,QString& e,bool ro){e.clear();QFile f(p);if(!f.open(QIODevice::ReadOnly))return reject(e,f.errorString());if(f.size()>4*1024*1024)return reject(e,"System file exceeds the 4 MiB limit.");try{auto bytes=f.readAll();if(f.error()!=QFileDevice::NoError)return reject(e,f.errorString());auto next=QueuingDocument::decode(std::string_view(bytes.constData(),static_cast<std::size_t>(bytes.size())));document_=std::move(next);undo_.reset();clipboard_.reset();selection_=-1;format_=document_->format();path_=p;readOnly_=ro;dirty_=false;results_.clear();transients_.clear();originalComparisons_.clear();emit inputReset();emit changed();return true;}catch(const std::exception& x){return reject(e,x.what());}}
bool QueuingController::commitInput(){pendingValid_=true;emit commitRequested();return pendingValid_;}
bool QueuingController::saveTo(const QString& p,QString& e){e.clear();if(!commitInput())return reject(e,"Correct the active input before saving.");if(!document_||p.isEmpty())return reject(e,"Initialize a system and choose a filename.");auto identity=[](const QString& s){QFileInfo f(s);return f.exists()?f.canonicalFilePath():f.absoluteFilePath();};if(readOnly_&&identity(p)==identity(path_))return reject(e,"The source is read-only. Save to a different file.");const auto bytes=document_->encode();QSaveFile f(p);if(!f.open(QIODevice::WriteOnly)||f.write(bytes.data(),static_cast<qint64>(bytes.size()))!=static_cast<qint64>(bytes.size())||!f.commit())return reject(e,f.errorString());path_=p;dirty_=readOnly_=false;emit inputReset();emit changed();return true;}
bool QueuingController::solve(QString& e){
 e.clear();if(!commitInput()||!document_)return reject(e,"Initialize and correct the scenarios before solving.");
 std::vector<queuing::Result> next;std::vector<std::optional<queuing::TransientResult>> transients,originalComparisons;
 for(std::size_t i=0;i<document_->count();++i){try{
  const auto model=document_->model(i);next.push_back(queuing::solve(model));
  if(model.arrivalRate==0){
   const auto initial=model.sourceLimit.value_or(9999);
   transients.push_back(queuing::pureDeath(model.serviceRate,initial));
   // Observed original terminal support is capped at 500 even when its N label
   // says 501/1000/9999/infinity. Keep the requested mathematical model intact.
   originalComparisons.push_back(queuing::pureDeath(model.serviceRate,std::min<std::size_t>(initial,500)));
  }else{
   if(model.serviceRate==0)transients.push_back(queuing::pureBirth(model.arrivalRate));
   else transients.push_back({});
   originalComparisons.push_back({});
  }
 }catch(const std::exception& x){return reject(e,QString("Scenario %1: %2").arg(i+1).arg(x.what()));}}
 results_=std::move(next);transients_=std::move(transients);originalComparisons_=std::move(originalComparisons);emit changed();return true;
}
bool QueuingController::requestSave(){if(!commitInput()||!document_)return false;auto p=dialogs_.chooseSave(path_);if(!p)return false;QString e;if(saveTo(*p,e))return true;dialogs_.showError("Cannot Save Problem",e);return false;}
bool QueuingController::leaveInput(){if(!commitInput())return false;if(!dirty_)return true;auto a=dialogs_.askSave();return a==SaveChoice::No||(a==SaveChoice::Yes&&requestSave());}
bool QueuingController::requestOpen(){auto p=dialogs_.chooseOpen();if(!p||!leaveInput())return false;QString e;if(openFile(p->path,e,p->readOnly))return true;dialogs_.showError("Cannot Open Problem",e);return false;}
QString QueuingController::outputText(int scenario,std::size_t first,std::size_t count)const{
 if(!document_||results_.empty())return {};
 core::NumberFormatter formatter(document_->format());auto n=[&](double v){return QString::fromStdString(formatter.format(v));};QString text=QString::fromStdString(document_->title())+"\nQUEUING ANALYSIS\n";
 auto summary=[&](std::size_t i){const auto& r=results_[i];QString s=QString("\nScenario %1\n").arg(i+1);for(int col=0;col<5;++col)s+=QString::fromStdString(document_->cell(i,col))+"  ";s+="(lambda, mu, c, K, N)\n";
 if(i<transients_.size()&&transients_[i]){
  const auto& t=*transients_[i];
  s+=t.kind==queuing::TransientKind::PureBirth?"PURE BIRTH TRANSIENT (t = 1)\n":"PURE DEATH TRANSIENT (t = 1)\nREQUESTED MODEL TRANSIENT\n";
  s+="Mean population = "+n(t.mean)+"\n";
  if(t.kind==queuing::TransientKind::PureDeath){
   const auto model=document_->model(i);
   s+=QString("Initial population N = %1\n").arg(t.initialPopulation);
   const auto partial=t.mean-t.initialPopulation*queuing::probability(t,t.initialPopulation);
   s+="Partial mean excluding state N = "+n(std::max(0.,partial))+"\n";
   if(!model.sourceLimit)s+="For an infinite source, the requested mathematical profile uses clone reference N=9999. Original TORA displays this label but its observed effective population is 500.\n";
   if(i<originalComparisons_.size()&&originalComparisons_[i]){
    const auto& legacy=*originalComparisons_[i];
    const auto legacyPartial=std::max(0.,legacy.mean-legacy.initialPopulation*queuing::probability(legacy,legacy.initialPopulation));
    s+="\nORIGINAL TORA COMPARISON (independent calculation)\n";
    s+=QString("Effective initial population = %1\n").arg(legacy.initialPopulation);
    s+="Compatibility profile uses the observed maximum population of 500; it does not change the requested model.\n";
    s+="Capped-profile true mean = "+n(legacy.mean)+"\n";
    s+="Partial mean excluding capped terminal state = "+n(legacyPartial)+"\n";
    s+=QString("Capped terminal state %1: pn = %2, cumulative Pn = %3\n").arg(legacy.initialPopulation).arg(n(queuing::probability(legacy,legacy.initialPopulation)),n(queuing::cumulativeProbability(legacy,legacy.initialPopulation)));
    if(legacy.initialPopulation==500&&model.servers==1&&(model.serviceRate==1||model.serviceRate==3)){
     const auto observed=model.serviceRate==1?QString("315.05973"):QString("472.10614");
     s+="Original displayed Ls = "+observed+" (captured profile; a small legacy numerical discrepancy remains).\n";
    }
    if(t.initialPopulation==3)s+="Captured original N=3 Ls omits the terminal-state contribution.\n";
   }
  }
  s+="Unit-time original boundary profile uses a constant rate independent of c; pure-birth probabilities ignore K/N.\nStationary throughput and waiting-time metrics do not apply to this transient result.\n";return s;
 }
 if(r.status==queuing::Status::Unstable)return s+"NO STATIONARY DISTRIBUTION: lambda >= c*mu\n";
 const auto& v=*r.metrics;s+="Effective lambda = "+n(v.throughput)+"\nRaw rho/c (lambda/(c*mu)) = "+(v.offeredLoadPerServer?n(*v.offeredLoadPerServer):(v.absorbingNoService?"infinite (zero service)":"exceeds display range"))+"\nActual server utilization = "+n(v.utilization)+"\nLs = "+n(v.meanSystem)+"\nLq = "+n(v.meanQueue)+"\nWs = "+(v.absorbingNoService?QString("undefined (zero throughput)"):n(v.meanSystemTime))+"\nWq = "+(v.absorbingNoService?QString("undefined (zero throughput)"):n(v.meanQueueTime))+"\nProbability empty = "+n(v.probabilityEmpty)+"\nProbability full = "+n(v.probabilityFull)+"\nArrival blocking probability = "+(v.arrivalBlockingUndefined?QString("undefined (no attempted arrivals)"):n(v.blockingProbability))+"\nAdmitted arrival wait probability = "+(v.absorbingNoService?QString("undefined (zero throughput)"):n(v.probabilityWait))+"\nBalance residual = "+QString::number(v.maximumRelativeBalanceResidual,'g',6)+"\n";if(v.absorbingNoService)s+="Absorbing full state: no service completions; stationary waiting times are undefined.\n";if(v.zeroArrivalLimit)s+="Zero arrivals: Ws and Wq are limiting values.\n";return s;};
 if(scenario<0){text+="COMPARATIVE ANALYSIS\n";for(std::size_t i=0;i<results_.size();++i)text+=summary(i);return text;}
 if(static_cast<std::size_t>(scenario)>=results_.size())return {};
 const auto selected=static_cast<std::size_t>(scenario);const auto& r=results_[selected];text+=summary(selected);
 if(selected<transients_.size()&&transients_[selected]){const auto& t=*transients_[selected];count=std::min<std::size_t>(count,1000);first=std::min<std::size_t>(first,1000000);if(t.kind==queuing::TransientKind::PureDeath)text+="\nREQUESTED MODEL STATE PROBABILITIES\n";text+="\nn     Probability pn     Cumulative Pn\n";for(std::size_t offset=0;offset<count;++offset){const auto state=first+offset;if(t.kind==queuing::TransientKind::PureDeath&&state>t.initialPopulation)break;text+=QString::number(state)+"     "+n(queuing::probability(t,state))+"     "+n(queuing::cumulativeProbability(t,state))+"\n";}return text+"Poisson omitted-tail bound = "+QString::number(t.omittedMass,'g',6)+"\n";}
 if(r.status!=queuing::Status::Stationary)return text;
 count=std::min<std::size_t>(count,1000);first=std::min<std::size_t>(first,1000000);text+="\nn     Probability pn     Cumulative Pn\n";
 long double cumulative=0;
 // Sum finite prefix or use analytical infinite geometric tail.
 if(r.capacity){for(std::size_t i=0;i<std::min(first,r.probabilities.size());++i)cumulative+=r.probabilities[i];}
 else if(first<=r.probabilities.size()){for(std::size_t i=0;i<first;++i)cumulative+=r.probabilities[i];}
 else cumulative=1-static_cast<long double>(r.tailMass)*std::exp(r.logTailRatio*static_cast<double>(first-(r.probabilities.size()-1)));
 for(std::size_t offset=0;offset<count;++offset){const auto state=first+offset;if(r.capacity&&state>*r.capacity)break;const auto p=queuing::probability(r,state);cumulative+=p;text+=QString::number(state)+"     "+n(p)+"     "+n(static_cast<double>(std::min(1.0L,cumulative)))+"\n";}
 text+="Displayed state range is bounded; use First state / State count to view more.\n";return text;
}
QueuingController& queuingController(ApplicationShell& shell){if(auto* c=shell.findChild<QueuingController*>("queuing-controller"))return *c;return *new QueuingController(&shell);}
bool QueuingController::setFormat(core::NumberFormat f,QString& e){e.clear();if(readOnly_)return reject(e,"Save a copy before editing this read-only system.");try{(void)core::NumberFormatter(f);format_=f;undo_.reset();if(document_){document_->setFormat(f);dirty_=true;emit changed();}return true;}catch(const std::exception& x){return reject(e,x.what());}}

bool QueuingController::selectScenario(int row,bool before){if(!document_||row<0||static_cast<std::size_t>(row)>=document_->count())return false;selection_=row;insertBefore_=before;emit changed();return true;}
bool QueuingController::copySelection(){if(!commitInput()||!hasSelection())return false;clipboard_=document_->scenario(static_cast<std::size_t>(selection_));emit changed();return true;}
bool QueuingController::editSelection(int operation,QString& e){e.clear();if(!commitInput())return reject(e,"Correct the active input before editing the grid.");if(readOnly_||!document_)return reject(e,"Open an editable system first.");try{if(operation==0){if(!undo_)return reject(e,"There is no grid operation to undo.");document_=std::move(undo_);undo_.reset();}else{if(!hasSelection())return reject(e,"Select a scenario heading first.");auto next=*document_;if(operation==1){if(!clipboard_)return reject(e,"Copy a scenario first.");next.pasteScenario(static_cast<std::size_t>(selection_),*clipboard_);}else if(operation==2)next.deleteScenario(static_cast<std::size_t>(selection_));else if(operation==3)next.insertScenario(static_cast<std::size_t>(selection_)+(insertBefore_?0:1));else return reject(e,"Unknown grid operation.");undo_=document_;document_=std::move(next);}selection_=-1;modified();emit inputReset();return true;}catch(const std::exception& x){return reject(e,x.what());}}
bool QueuingController::addScenario(QString& e){e.clear();if(!commitInput()||readOnly_||!document_)return reject(e,"Open an editable scenario first.");try{auto next=*document_;next.addScenario();undo_=document_;document_=std::move(next);selection_=-1;modified();emit inputReset();return true;}catch(const std::exception& x){return reject(e,x.what());}}
bool QueuingController::deleteScenario(int row,QString& e){if(!selectScenario(row))return reject(e,"Select a scenario heading first.");return editSelection(2,e);}

}
