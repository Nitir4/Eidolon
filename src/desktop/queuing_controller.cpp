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
QueuingScenarioSummary QueuingController::scenarioSummary(std::size_t index) const
{
    QueuingScenarioSummary summary;
    if (!document_ || index >= results_.size()) return summary;
    const core::NumberFormatter formatter(document_->format());
    const auto n = [&formatter](double value) { return QString::fromStdString(formatter.format(value)); };
    const auto add = [&summary](const QString& label, const QString& value) { summary.metrics.append({label,value}); };
    const auto& result = results_[index];
    if (const auto& transient = transients_[index]) {
        const bool pureDeath = transient->kind == queuing::TransientKind::PureDeath;
        summary.profile = pureDeath ? "Pure death transient (t = 1)" : "Pure birth transient (t = 1)";
        add("Mean population", n(transient->mean));
        if (pureDeath) {
            const auto population = transient->initialPopulation;
            add("Initial population N", QString::number(population));
            add("Partial mean excluding state N", n(std::max(0., transient->mean - population * queuing::probability(*transient,population))));
            if (!document_->model(index).sourceLimit)
                summary.notes << "For an infinite source, the requested mathematical profile uses clone reference N=9999. Original TORA displays this label but its observed effective population is 500.";
            if (const auto& legacy = originalComparisons_[index]) {
                const auto cap = legacy->initialPopulation;
                const double last = queuing::probability(*legacy,cap);
                add("Effective initial population", QString::number(cap));
                add("Capped-profile true mean", n(legacy->mean));
                add("Partial mean excluding capped terminal state", n(std::max(0., legacy->mean - cap * last)) + " (before the original probability cutoff)");
                add(QStringLiteral("Capped terminal state %1").arg(cap), QStringLiteral("pn = %1, cumulative Pn = %2")
                    .arg(n(last), n(queuing::cumulativeProbability(*legacy,cap))));
                summary.notes << "Original TORA comparison (independent calculation). It uses an effective initial population capped at 500. The original routine limits its active states to min(K, N, 500)."
                    << "Original TORA's Ls omits the initial-state contribution and stops summing after the first probability below 0.00001 following the peak. It is not the full mean population shown above.";
            }
        }
        summary.notes << "The unit-time boundary profile uses a constant rate independent of c. Pure-birth probabilities ignore K and N."
            << "Stationary throughput and waiting-time metrics do not apply to this transient result.";
        return summary;
    }
    if (!result.metrics) {
        summary.profile = "No stationary distribution";
        summary.notes << "The offered arrival rate is at least the total service rate (lambda >= c*mu).";
        return summary;
    }
    summary.profile = "Stationary distribution";
    const auto& metrics = *result.metrics;
    const auto waitValue = [&metrics,&n](double value) {
        return metrics.absorbingNoService ? QStringLiteral("undefined (zero throughput)") : n(value);
    };
    add("Effective lambda", n(metrics.throughput));
    add("Raw rho/c (lambda/(c*mu))", metrics.offeredLoadPerServer ? n(*metrics.offeredLoadPerServer)
        : metrics.absorbingNoService ? "infinite (zero service)" : "exceeds display range");
    add("Actual server utilization", n(metrics.utilization));
    add("Ls", n(metrics.meanSystem));
    add("Lq", n(metrics.meanQueue));
    add("Ws", waitValue(metrics.meanSystemTime));
    add("Wq", waitValue(metrics.meanQueueTime));
    add("Probability empty", n(metrics.probabilityEmpty));
    add("Probability full", n(metrics.probabilityFull));
    add("Arrival blocking probability", metrics.arrivalBlockingUndefined
        ? QStringLiteral("undefined (no attempted arrivals)") : n(metrics.blockingProbability));
    add("Admitted arrival wait probability", waitValue(metrics.probabilityWait));
    add("Balance residual", QString::number(metrics.maximumRelativeBalanceResidual, 'g', 6));
    if (metrics.absorbingNoService)
        summary.notes << "Absorbing full state: no service completions; stationary waiting times are undefined.";
    if (metrics.zeroArrivalLimit) summary.notes << "Zero arrivals: Ws and Wq are limiting values.";
    return summary;
}

std::vector<QueuingStateProbability> QueuingController::stateProbabilities(std::size_t scenario, std::size_t first, std::size_t count) const
{
    std::vector<QueuingStateProbability> rows;
    if (scenario >= results_.size()) return rows;
    count = std::min<std::size_t>(count,1000);
    first = std::min<std::size_t>(first,1000000);
    rows.reserve(count);
    if (const auto& transient = transients_[scenario]) {
        for (std::size_t state = first; state < first + count; ++state) {
            if (transient->kind == queuing::TransientKind::PureDeath && state > transient->initialPopulation) break;
            rows.push_back({state,queuing::probability(*transient,state),queuing::cumulativeProbability(*transient,state)});
        }
        return rows;
    }
    const auto& result = results_[scenario];
    if (result.status != queuing::Status::Stationary) return rows;
    long double cumulative = 0;
    // The geometric tail includes the last explicit state.
    if (result.capacity || first <= result.probabilities.size()) {
        for (std::size_t state = 0; state < std::min(first,result.probabilities.size()); ++state)
            cumulative += result.probabilities[state];
    } else {
        cumulative = 1 - static_cast<long double>(result.tailMass) *
            std::exp(result.logTailRatio * static_cast<double>(first - (result.probabilities.size() - 1)));
    }
    for (std::size_t state = first; state < first + count; ++state) {
        if (result.capacity && state > *result.capacity) break;
        const double probability = queuing::probability(result,state);
        cumulative += probability;
        rows.push_back({state,probability,static_cast<double>(std::min(1.0L,cumulative))});
    }
    return rows;
}

QString QueuingController::outputText(int scenario, std::size_t first, std::size_t count) const
{
    if (!document_ || results_.empty()) return {};
    const core::NumberFormatter format(document_->format());
    const auto n = [&format](double value) { return QString::fromStdString(format.format(value)); };
    QString text = QString::fromStdString(document_->title()) + "\nQUEUING ANALYSIS\n";
    const auto appendSummary = [this,&text](std::size_t index) {
        text += QStringLiteral("\nScenario %1\n").arg(index + 1);
        for (int column = 0; column < 5; ++column) text += QString::fromStdString(document_->cell(index,column)) + "  ";
        text += "(lambda, mu, c, K, N)\n";
        const auto report = scenarioSummary(index);
        if (const auto& transient = transients_[index]) {
            text += report.profile.toUpper() + '\n';
            if (transient->kind == queuing::TransientKind::PureDeath) text += "REQUESTED MODEL TRANSIENT\n";
        } else if (!results_[index].metrics) text += "NO STATIONARY DISTRIBUTION: lambda >= c*mu\n";
        for (const auto& [label,value] : report.metrics) {
            if (label == "Effective initial population") text += "\nORIGINAL TORA COMPARISON (independent calculation)\n";
            text += label + " = " + value + '\n';
        }
        for (const auto& note : report.notes) text += note + '\n';
    };
    if (scenario < 0) {
        text += "COMPARATIVE ANALYSIS\n";
        for (std::size_t index = 0; index < results_.size(); ++index) appendSummary(index);
        return text;
    }
    const auto index = static_cast<std::size_t>(scenario);
    if (index >= results_.size()) return {};
    appendSummary(index);
    const auto& transient = transients_[index];
    if (!transient && results_[index].status != queuing::Status::Stationary) return text;
    if (transient && transient->kind == queuing::TransientKind::PureDeath)
        text += "\nREQUESTED MODEL STATE PROBABILITIES\n";
    text += "\nn     Probability pn     Cumulative Pn\n";
    for (const auto& row : stateProbabilities(index,first,count))
        text += QString::number(row.state) + "     " + n(row.probability) + "     " + n(row.cumulative) + '\n';
    if (transient) return text + "Poisson omitted-tail bound = " + QString::number(transient->omittedMass,'g',6) + '\n';
    return text + "Displayed state range is bounded; use First state / State count to view more.\n";
}
QueuingController& queuingController(ApplicationShell& shell){if(auto* c=shell.findChild<QueuingController*>("queuing-controller"))return *c;return *new QueuingController(&shell);}
bool QueuingController::setFormat(core::NumberFormat f,QString& e){e.clear();if(readOnly_)return reject(e,"Save a copy before editing this read-only system.");try{(void)core::NumberFormatter(f);format_=f;undo_.reset();if(document_){document_->setFormat(f);dirty_=true;emit changed();}return true;}catch(const std::exception& x){return reject(e,x.what());}}

bool QueuingController::selectScenario(int row,bool before){if(!document_||row<0||static_cast<std::size_t>(row)>=document_->count())return false;selection_=row;insertBefore_=before;emit changed();return true;}
bool QueuingController::copySelection(){if(!commitInput()||!hasSelection())return false;clipboard_=document_->scenario(static_cast<std::size_t>(selection_));emit changed();return true;}
bool QueuingController::editSelection(int operation,QString& e){e.clear();if(!commitInput())return reject(e,"Correct the active input before editing the grid.");if(readOnly_||!document_)return reject(e,"Open an editable system first.");try{if(operation==0){if(!undo_)return reject(e,"There is no grid operation to undo.");document_=std::move(undo_);undo_.reset();}else{if(!hasSelection())return reject(e,"Select a scenario heading first.");auto next=*document_;if(operation==1){if(!clipboard_)return reject(e,"Copy a scenario first.");next.pasteScenario(static_cast<std::size_t>(selection_),*clipboard_);}else if(operation==2)next.deleteScenario(static_cast<std::size_t>(selection_));else if(operation==3)next.insertScenario(static_cast<std::size_t>(selection_)+(insertBefore_?0:1));else return reject(e,"Unknown grid operation.");undo_=document_;document_=std::move(next);}selection_=-1;modified();emit inputReset();return true;}catch(const std::exception& x){return reject(e,x.what());}}
bool QueuingController::addScenario(QString& e){e.clear();if(!commitInput()||readOnly_||!document_)return reject(e,"Open an editable scenario first.");try{auto next=*document_;next.addScenario();undo_=document_;document_=std::move(next);selection_=-1;modified();emit inputReset();return true;}catch(const std::exception& x){return reject(e,x.what());}}
bool QueuingController::deleteScenario(int row,QString& e){if(!selectScenario(row))return reject(e,"Select a scenario heading first.");return editSelection(2,e);}

}
