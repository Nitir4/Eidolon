#include "equations_screens.hpp"
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QCheckBox>
#include <QGridLayout>
namespace tora::desktop {
EquationsController::EquationsController(QObject* parent):QObject(parent){setObjectName("equations-controller");auto* owner=qobject_cast<QWidget*>(parent);dialogs_.showError=[owner](const QString& a,const QString& b){QMessageBox::warning(owner,a,b);};dialogs_.askSave=[owner]{auto a=QMessageBox::question(owner,"Save?","Do you wish to save data?",QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);return a==QMessageBox::Yes?SaveChoice::Yes:a==QMessageBox::No?SaveChoice::No:SaveChoice::Cancel;};dialogs_.chooseSave=[owner](const QString& current)->std::optional<QString>{auto p=QFileDialog::getSaveFileName(owner,"Save As",current,"Text Files (*.txt);;All Files (*)");return p.isEmpty()?std::nullopt:std::optional<QString>(p);};dialogs_.chooseOpen=[owner]()->std::optional<OpenSelection>{QFileDialog d(owner,"Open System Problem");d.setFileMode(QFileDialog::ExistingFile);d.setOption(QFileDialog::DontUseNativeDialog);auto* ro=new QCheckBox("Open as read-only",&d);if(auto* l=qobject_cast<QGridLayout*>(d.layout()))l->addWidget(ro,l->rowCount(),0,1,l->columnCount());if(d.exec()!=QDialog::Accepted||d.selectedFiles().isEmpty())return {};return OpenSelection{d.selectedFiles().first(),ro->isChecked()};};}
bool EquationsController::reject(QString& e,const QString& m){e=m;pendingValid_=false;return false;}
void EquationsController::modified(){dirty_=true;result_.reset();inverse_.reset();solutionInverse_.reset();solutionInverseError_.clear();emit changed();}
void EquationsController::beginNew(core::NumberFormat f){(void)core::NumberFormatter(f);format_=f;document_.reset();result_.reset();inverse_.reset();solutionInverse_.reset();solutionInverseError_.clear();dirty_=readOnly_=false;path_.clear();emit inputReset();emit changed();}
bool EquationsController::initialize(const QString& title,int r,QString& e,bool matrixOnly){e.clear();if(readOnly_)return reject(e,"Save a copy before editing this read-only system.");try{auto next=EquationsDocument::blank(r,title.toStdString(),format_,matrixOnly);document_=std::move(next);modified();emit inputReset();return true;}catch(const std::exception& x){return reject(e,x.what());}}
bool EquationsController::editCell(int r,int c,const QString& v,QString& e){e.clear();if(!document_)return reject(e,"Initialize a system first.");if(readOnly_)return reject(e,"Save a copy before editing this read-only system.");try{if(document_->cell(r,c)==v.toStdString())return true;document_->setCell(r,c,v.toStdString());modified();return true;}catch(const std::exception& x){return reject(e,x.what());}}
bool EquationsController::setTitle(const QString& title,QString& e){e.clear();if(!document_)return true;if(document_->title()==title.toStdString())return true;if(readOnly_)return reject(e,"Save a copy before editing this read-only system.");try{document_->setTitle(title.toStdString());modified();return true;}catch(const std::exception& x){return reject(e,x.what());}}
bool EquationsController::openFile(const QString& p,QString& e,bool ro){e.clear();QFile f(p);if(!f.open(QIODevice::ReadOnly))return reject(e,f.errorString());if(f.size()>4*1024*1024)return reject(e,"System file exceeds the 4 MiB limit.");try{auto bytes=f.readAll();if(f.error()!=QFileDevice::NoError)return reject(e,f.errorString());auto next=EquationsDocument::decode(std::string_view(bytes.constData(),static_cast<std::size_t>(bytes.size())));document_=std::move(next);format_=document_->format();path_=p;readOnly_=ro;dirty_=false;result_.reset();inverse_.reset();solutionInverse_.reset();solutionInverseError_.clear();emit inputReset();emit changed();return true;}catch(const std::exception& x){return reject(e,x.what());}}
bool EquationsController::commitInput(){pendingValid_=true;emit commitRequested();return pendingValid_;}
bool EquationsController::saveTo(const QString& p,QString& e){e.clear();if(!commitInput())return reject(e,"Correct the active input before saving.");if(!document_||p.isEmpty())return reject(e,"Initialize a system and choose a filename.");auto identity=[](const QString& s){QFileInfo f(s);return f.exists()?f.canonicalFilePath():f.absoluteFilePath();};if(readOnly_&&identity(p)==identity(path_))return reject(e,"The source is read-only. Save to a different file.");const auto bytes=document_->encode();QSaveFile f(p);if(!f.open(QIODevice::WriteOnly)||f.write(bytes.data(),static_cast<qint64>(bytes.size()))!=static_cast<qint64>(bytes.size())||!f.commit())return reject(e,f.errorString());path_=p;dirty_=readOnly_=false;emit inputReset();emit changed();return true;}
bool EquationsController::solve(QString& e,bool inverse){
 e.clear();if(!commitInput()||!document_)return reject(e,"Initialize and correct the system before solving.");
 if(document_->matrixOnly()&&!inverse)return reject(e,"This matrix file has no RHS. Use Matrix Inversion.");
 try{
  equations::Options options;options.recordTrace=true;options.maximumTraceSteps=200;
  const auto coefficients=document_->coefficients();
  if(inverse){
   auto next=equations::invert(coefficients,options);inverse_=std::move(next);result_.reset();solutionInverse_.reset();solutionInverseError_.clear();
  }else{
   auto next=equations::solve(coefficients,document_->rhs(),options);
   std::optional<equations::InverseResult> associated;QString inverseError;
   if(next.status==equations::Status::Unique && coefficients.rows()==coefficients.columns()){
    // Original square-system Solution includes its inverse and determinant.
    // Keep only the equation trace; the separate inversion mode records [A | I].
    options.recordTrace=false;
    try{associated=equations::invert(coefficients,options);}
    catch(const std::exception& x){inverseError=QString::fromUtf8(x.what());}
   }
   result_=std::move(next);inverse_.reset();solutionInverse_=std::move(associated);solutionInverseError_=std::move(inverseError);
  }
  emit changed();return true;
 }catch(const std::exception& x){return reject(e,x.what());}
}
bool EquationsController::requestSave(){if(!commitInput()||!document_)return false;auto p=dialogs_.chooseSave(path_);if(!p)return false;QString e;if(saveTo(*p,e))return true;dialogs_.showError("Cannot Save Problem",e);return false;}
bool EquationsController::leaveInput(){if(!commitInput())return false;if(!dirty_)return true;auto a=dialogs_.askSave();return a==SaveChoice::No||(a==SaveChoice::Yes&&requestSave());}
bool EquationsController::requestOpen(){auto p=dialogs_.chooseOpen();if(!p||!leaveInput())return false;QString e;if(openFile(p->path,e,p->readOnly))return true;dialogs_.showError("Cannot Open Problem",e);return false;}
QString EquationsController::outputText()const{
 if(!document_||(!result_&&!inverse_))return {};
 core::NumberFormatter f(document_->format());auto n=[&](double v){return QString::fromStdString(f.format(v));};
 QString text=QString::fromStdString(document_->title())+(document_->matrixOnly()?"\nORIGINAL MATRIX\n":"\nORIGINAL EQUATIONS\n");
 const auto a=document_->coefficients();const auto b=document_->rhs();
 for(std::size_t i=0;i<a.rows();++i){for(std::size_t j=0;j<a.columns();++j)text+=n(a(i,j))+QString(" x%1 ").arg(j+1);if(!document_->matrixOnly())text+="= "+n(b[i]);text+="\n";}
 const auto inverseText=[&](const equations::InverseResult& inverse,const QString& heading){
  QString summary="\n"+heading+"\nDeterminant = "+(inverse.determinant?n(*inverse.determinant):QString("sign %1; log(abs(det)) = %2 (outside binary64 range)").arg(inverse.determinantSign).arg(static_cast<double>(inverse.logAbsDeterminant),0,'g',12))+"\n";
  if(!inverse.invertible)return summary+"NO INVERSE: numerically singular matrix\n";
  for(std::size_t i=0;i<a.rows();++i){for(std::size_t j=0;j<a.columns();++j)summary+=n(inverse.inverse(i,j))+"  ";summary+="\n";}
  return summary+"Maximum identity residual = "+QString::number(inverse.maximumIdentityResidual,'g',6)+"\n";
 };
 if(inverse_)return text+inverseText(*inverse_,"MATRIX INVERSION");
 text+=QString("\nCoefficient rank = %1; augmented rank = %2\n").arg(result_->rank).arg(result_->augmentedRank);
 if(result_->status==equations::Status::Inconsistent){text+="INCONSISTENT: no solution\nWitness y: ";for(auto v:result_->inconsistencyWitness)text+=n(v)+" ";return text+"\ny^T b = "+QString::number(result_->witnessRhs,'g',8)+"; max |y^T A| = "+QString::number(result_->witnessCoefficientResidual,'g',8)+"\n";}
 text+=result_->status==equations::Status::Unique?"UNIQUE SOLUTION\n":"INFINITELY MANY SOLUTIONS: particular solution (free variables zero)\n";
 for(std::size_t j=0;j<result_->solution.size();++j)text+=QString("x%1 = %2\n").arg(j+1).arg(n(result_->solution[j]));
 if(!result_->nullspace.empty()){text+="x = particular solution + sum(t_i z_i)\n";for(std::size_t i=0;i<result_->nullspace.size();++i){text+=QString("z%1 = (").arg(i+1);for(auto v:result_->nullspace[i])text+=n(v)+" ";text+=")\n";}}
 if(solutionInverse_)text+=inverseText(*solutionInverse_,"ASSOCIATED MATRIX INVERSE");
 else if(!solutionInverseError_.isEmpty())text+="Associated matrix inverse unavailable: "+solutionInverseError_+"\n";
 return text+"Relative residual = "+QString::number(result_->relativeResidual,'g',6)+"\n";
}
EquationsController& equationsController(ApplicationShell& shell){if(auto* c=shell.findChild<EquationsController*>("equations-controller"))return *c;return *new EquationsController(&shell);}
bool EquationsController::setFormat(core::NumberFormat f,QString& e){e.clear();if(readOnly_)return reject(e,"Save a copy before editing this read-only system.");try{(void)core::NumberFormatter(f);format_=f;if(document_){document_->setFormat(f);dirty_=true;emit changed();}return true;}catch(const std::exception& x){return reject(e,x.what());}}

}
