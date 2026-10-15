#pragma once
#include "application_shell.hpp"
#include "lp_input_controller.hpp"
#include <tora/compat/integer_document.hpp>
#include <tora/integer/integer.hpp>
class QPrinter;
class QFont;
namespace tora::desktop {
class IntegerController final:public QObject {
 Q_OBJECT
public:
 explicit IntegerController(ApplicationShell& shell);
 LpInputController& input(){return input_;}
 const LpInputController& input()const{return input_;}
 InputDialogs& dialogs(){return input_.dialogs();}
 const std::vector<bool>& flags()const{return document_.integral;}
 const std::optional<integer::Result>& result()const{return result_;}
 bool initialize(const QString& title,int variables,int constraints);
 bool loadExample();bool setFlag(int index,bool value);bool setFormat(core::NumberFormat format);
 bool openFile(const QString& path,bool readOnly=false);bool saveTo(const QString& path);
 void selectRow(int row);void selectColumn(int column);void clearSelection();
 bool copySelection();bool pasteSelection();bool deleteSelection();bool insertSelection(bool before=false);bool undo();
 bool hasUndo()const{return undo_.has_value();}
 bool requestOpen();bool requestSave();bool allowLeave();bool solve(std::size_t limit=4096);
 bool startGuided(std::size_t limit=4096);bool nextGuided(std::optional<std::size_t> pendingIndex={});bool branchGuided(std::size_t variable);bool finishGuided();void setBoundPruning(bool enabled);
 const std::optional<integer::Session>& guided()const{return session_;}
 bool dirty()const;bool readOnly()const{return readOnly_;}QString path()const{return path_;}
 void fail(const QString& message);
 std::string encode()const;
signals:void changed();void resultChanged();void modelReplaced();
private:
 bool commit();void install(compat::IntegerDocument document,QString path={},bool readOnly=false,bool saved=false);
 compat::IntegerDocument snapshot()const;
 std::optional<compat::IntegerDocument> undo_;int selectedColumn_=-1;std::optional<bool> clipboardFlag_;
 ApplicationShell& shell_;LpInputController input_;compat::IntegerDocument document_;
 std::optional<integer::Session> session_;std::optional<integer::Result> result_;QString path_;std::string checkpoint_;bool readOnly_=false,installing_=false;
};
IntegerController& integerController(ApplicationShell& shell);
ModuleDescriptor integerModule();
bool printInteger(QPrinter& printer,const IntegerController& controller,const QFont& font);
}
