#pragma once
#include "application_shell.hpp"
#include "equations_document.hpp"
#include "lp_input_controller.hpp"
#include <tora/equations/equations.hpp>
#include <QObject>
#include <optional>
class QPainter;class QPrinter;
namespace tora::desktop {
class EquationsController final:public QObject {
 Q_OBJECT
public:
 explicit EquationsController(QObject* parent);
 void beginNew(core::NumberFormat format={});
 bool initialize(const QString& title,int variables,QString& error,bool matrixOnly=false);
 bool editCell(int row,int column,const QString& value,QString& error);
 bool setTitle(const QString& title,QString& error);
 bool setFormat(core::NumberFormat format,QString& error);
 bool openFile(const QString& path,QString& error,bool readOnly=false);
 bool saveTo(const QString& path,QString& error);
 bool solve(QString& error,bool inverse=false);
 bool commitInput();bool requestSave();bool requestOpen();bool leaveInput();
 InputDialogs& dialogs(){return dialogs_;}
 const EquationsDocument* document()const{return document_?&*document_:nullptr;}
 const equations::Result* result()const{return result_?&*result_:nullptr;}
 const equations::InverseResult* inverse()const{return inverse_?&*inverse_:nullptr;}
 // Cached summary for unique equation systems; selected trace remains [A | b].
 const equations::InverseResult* solutionInverse()const{return solutionInverse_?&*solutionInverse_:nullptr;}
 const QString& solutionInverseError() const {return solutionInverseError_;}
 bool dirty()const{return dirty_;}bool readOnly()const{return readOnly_;}
 const QString& sourcePath()const{return path_;}
 QString outputText()const;
signals:
 void changed();void inputReset();void commitRequested();
private:
 bool reject(QString& error,const QString& message);void modified();
 std::optional<EquationsDocument> document_;std::optional<equations::Result> result_;std::optional<equations::InverseResult> inverse_,solutionInverse_;QString solutionInverseError_;
 InputDialogs dialogs_;QString path_;core::NumberFormat format_;bool dirty_=false,readOnly_=false,pendingValid_=true;
};
EquationsController& equationsController(ApplicationShell& shell);
ModuleDescriptor equationsModule();
QString equationsTraceText(const EquationsController& controller,std::optional<std::size_t> selected);
bool printEquations(QPrinter& printer,const EquationsController& controller,const QString& selectedTrace={});
}
