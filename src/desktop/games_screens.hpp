#pragma once
#include "application_shell.hpp"
#include "games_document.hpp"
#include "lp_input_controller.hpp"
#include <tora/games/graphical.hpp>
#include <QObject>
#include <optional>
class QPainter;class QPrinter;
namespace tora::desktop {
class GamesController final:public QObject {
 Q_OBJECT
public:
 explicit GamesController(QObject* parent);
 void beginNew(core::NumberFormat format={});
 bool initialize(const QString& title,int rows,int columns,QString& error);
 bool editCell(int row,int column,const QString& value,QString& error);
 bool setTitle(const QString& title,QString& error);
 bool setFormat(core::NumberFormat format,QString& error);
 bool selectLine(bool row,int position,bool before=false);bool copySelection();
 bool editSelection(int operation,QString& error); // 0 undo, 1 paste, 2 delete, 3 insert at selected heading
 bool hasSelection()const{return selection_>=0;}bool canUndo()const{return undo_.has_value()&&!readOnly_;}
 bool canPaste()const{return hasSelection()&&selectionRow_==clipboardRow_&&!clipboard_.empty()&&!readOnly_;}
 bool openFile(const QString& path,QString& error,bool readOnly=false);
 bool saveTo(const QString& path,QString& error);
 bool solve(QString& error,bool graphical=false);
 bool commitInput();bool requestSave();bool requestOpen();bool leaveInput();
 InputDialogs& dialogs(){return dialogs_;}
 const GamesDocument* document()const{return document_?&*document_:nullptr;}
 const games::Result* result()const{return result_?&*result_:nullptr;}
 const games::GraphicalResult* graph()const{return graph_?&*graph_:nullptr;}
 bool dirty()const{return dirty_;}bool readOnly()const{return readOnly_;}
 const QString& sourcePath()const{return path_;}
 QString outputText()const;
signals:
 void changed();void inputReset();void commitRequested();
private:
 bool reject(QString& error,const QString& message);void modified();
 std::optional<GamesDocument> document_;std::optional<games::Result> result_;std::optional<games::GraphicalResult> graph_;
 std::optional<GamesDocument> undo_;int selection_=-1;bool selectionRow_=true,clipboardRow_=true,insertBefore_=false;std::vector<std::string> clipboard_;
 InputDialogs dialogs_;QString path_;core::NumberFormat format_;bool dirty_=false,readOnly_=false,pendingValid_=true;
};
GamesController& gamesController(ApplicationShell& shell);
ModuleDescriptor gamesModule();
void drawGamesGraph(QPainter& painter,const QRectF& rect,const games::GraphicalResult& graph,double zoom=1,const QString& strategy=QStringLiteral("A1"));
bool printGames(QPrinter& printer,const GamesController& controller);
}
