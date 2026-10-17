#pragma once
#include "application_shell.hpp"
#include "networks_document.hpp"
#include "lp_input_controller.hpp"
#include <QObject>
#include <variant>
class QPrinter;class QPainter;
namespace tora::desktop {
enum class NetworkRouteMethod {ShortestRoutes,Dijkstra,Floyd,BellmanFord};
using NetworkResult=std::variant<networks::SpanningForest,networks::ShortestRoutes,networks::AllPairsRoutes,networks::MaximumFlow>;
class NetworksController final:public QObject {
 Q_OBJECT
public:
 explicit NetworksController(QObject* parent);
 void beginNew(NetworkMode mode=NetworkMode::MinimalTree,core::NumberFormat format={});
 bool initialize(NetworkMode mode,const QString& title,int nodes,QString& error);
 bool editCell(int row,int column,const QString& value,QString& error);
 bool setTitle(const QString& value,QString& error);bool setFormat(core::NumberFormat format,QString& error);bool setSymmetrical(bool value,QString& error);
 bool editNode(int position,bool insert,QString& error);bool undo(QString& error);
 bool selectLine(bool row,int position,bool before=false);bool copySelection();bool pasteSelection(QString& error);bool editSelection(bool insert,QString& error);
 bool openFile(const QString& path,QString& error,bool readOnly=false);bool saveTo(const QString& path,QString& error);
 bool solve(int source,int sink,QString& error,NetworkRouteMethod method=NetworkRouteMethod::ShortestRoutes);
 bool commitInput();bool requestOpen();bool requestSave();bool leaveInput();
 InputDialogs& dialogs(){return dialogs_;}
 const NetworksDocument* document()const{return document_?&*document_:nullptr;}
 const NetworkResult* result()const{return result_?&*result_:nullptr;}
 const networks::Graph& solvedGraph()const{return solvedGraph_;}
 bool dirty()const{return dirty_;}bool readOnly()const{return readOnly_;}bool canUndo()const{return undo_.has_value()&&!readOnly_;}
 NetworkMode mode()const{return mode_;}NetworkRouteMethod routeMethod()const{return routeMethod_;}const QString& sourcePath()const{return path_;}
 QString outputText()const;QStringList iterationText()const;
signals:
 void changed();void inputReset();void commitRequested();
private:
 bool reject(QString& error,const QString& message);void modified();
 NetworkRouteMethod routeMethod_=NetworkRouteMethod::ShortestRoutes;NetworkMode mode_=NetworkMode::MinimalTree;core::NumberFormat format_;
 std::optional<NetworksDocument> document_,undo_;std::optional<NetworkResult> result_;
 networks::Graph solvedGraph_;int source_=0,sink_=0;QString path_;InputDialogs dialogs_;
 bool dirty_=false,readOnly_=false,pendingValid_=true;
 bool selectedRow_=true,insertBefore_=false;int selectedNode_=-1;
 std::vector<std::string> clipboard_;
};
NetworksController& networksController(ApplicationShell& shell);
ModuleDescriptor networksModule();
void drawNetwork(QPainter& painter,const QRectF& rect,const NetworksController& controller);
bool printNetworks(QPrinter& printer,const NetworksController& controller);
}
