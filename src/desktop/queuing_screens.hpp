#pragma once
#include "application_shell.hpp"
#include "queuing_document.hpp"
#include <tora/queuing/transient.hpp>
#include "lp_input_controller.hpp"
#include <QObject>
#include <optional>
class QPrinter;
namespace tora::desktop {
class QueuingController final:public QObject {
 Q_OBJECT
public:
 explicit QueuingController(QObject* parent);
 void beginNew(core::NumberFormat format={});
 bool initialize(const QString& title,QString& error);
 bool editCell(int row,int column,const QString& value,QString& error);
 bool setTitle(const QString& title,QString& error);
 bool setFormat(core::NumberFormat format,QString& error);
 bool addScenario(QString& error);bool deleteScenario(int row,QString& error);
 bool selectScenario(int row,bool before=false);bool copySelection();
 bool editSelection(int operation,QString& error); // 0 undo, 1 paste, 2 delete, 3 insert
 bool hasSelection()const{return document_&&selection_>=0&&static_cast<std::size_t>(selection_)<document_->count();}
 bool canUndo()const{return undo_.has_value()&&!readOnly_;}
 bool canPaste()const{return hasSelection()&&clipboard_.has_value()&&!readOnly_;}
 bool openFile(const QString& path,QString& error,bool readOnly=false);
 bool saveTo(const QString& path,QString& error);
 bool solve(QString& error);
 bool commitInput();bool requestSave();bool requestOpen();bool leaveInput();
 InputDialogs& dialogs(){return dialogs_;}
 const QueuingDocument* document()const{return document_?&*document_:nullptr;}
 const std::vector<queuing::Result>& results()const{return results_;}
 const std::vector<std::optional<queuing::TransientResult>>& transients()const{return transients_;}
 const std::vector<std::optional<queuing::TransientResult>>& originalComparisons()const{return originalComparisons_;}
 bool dirty()const{return dirty_;}bool readOnly()const{return readOnly_;}
 const QString& sourcePath()const{return path_;}
 QString outputText(int scenario=0,std::size_t firstState=0,std::size_t stateCount=100)const;
signals:
 void changed();void inputReset();void commitRequested();
private:
 bool reject(QString& error,const QString& message);void modified();
 std::optional<QueuingDocument> document_;std::vector<queuing::Result> results_;
 std::vector<std::optional<queuing::TransientResult>> transients_,originalComparisons_;
 std::optional<QueuingDocument> undo_;std::optional<QueuingDocument::Scenario> clipboard_;
 int selection_=-1;bool insertBefore_=false;
 InputDialogs dialogs_;QString path_;core::NumberFormat format_;bool dirty_=false,readOnly_=false,pendingValid_=true;
};
QueuingController& queuingController(ApplicationShell& shell);
ModuleDescriptor queuingModule();
bool printQueuing(QPrinter& printer,const QueuingController& controller,int scenario=0,std::size_t firstState=0,std::size_t stateCount=100);
}
