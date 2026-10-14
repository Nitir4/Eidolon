#pragma once
#include "application_shell.hpp"
#include "lp_input_controller.hpp"
#include <tora/assignment/assignment.hpp>
#include <tora/core/number_formatter.hpp>
#include <QAbstractTableModel>
class QPrinter;
class QFont;
namespace tora::desktop {
struct AssignmentData {
    assignment::Problem model;
    QString title;
    QStringList workers,jobs;
    core::NumberFormat format;
};
QByteArray encodeAssignment(const AssignmentData& data);
AssignmentData decodeAssignment(const QByteArray& bytes);
class AssignmentController;
class AssignmentGrid final:public QAbstractTableModel {
    Q_OBJECT
public:
    explicit AssignmentGrid(AssignmentController& c);
    int rowCount(const QModelIndex& p={})const override;
    int columnCount(const QModelIndex& p={})const override;
    QVariant data(const QModelIndex&,int role=Qt::DisplayRole)const override;
    QVariant headerData(int,Qt::Orientation,int role=Qt::DisplayRole)const override;
    Qt::ItemFlags flags(const QModelIndex&)const override;
    bool setData(const QModelIndex&,const QVariant&,int role=Qt::EditRole)override;
    void refresh();
private:AssignmentController& c_;
};
class AssignmentController final:public QObject {
    Q_OBJECT
public:
    explicit AssignmentController(ApplicationShell& shell);
    const std::optional<AssignmentData>& data()const{return data_;}
    const std::optional<assignment::Result>& result()const{return result_;}
    AssignmentGrid* grid(){return &grid_;}
    InputDialogs& dialogs(){return dialogs_;}
    bool initialize(const QString& title,int workers,int jobs);
    bool loadExample(bool maximize=false);
    bool setCost(int row,int column,const QString& value);
    bool setMetadata(const QString& title,assignment::Sense sense,core::NumberFormat format);
    bool openFile(const QString& path,bool readOnly=false);
    bool importTransportation(const QString& path,bool negatedProfit=false);
    bool saveTo(const QString& path);
    bool requestOpen();bool requestSave();bool allowLeave();bool solve();bool commitInput();
    bool setName(bool worker,int index,const QString& name);
    bool dirty()const{return dirty_;}bool readOnly()const{return readOnly_;}
    const QString& path()const{return path_;}
    void fail(const QString& message);
signals:void changed();void resultChanged();void commitRequested();void modelReplaced();
private:
    void install(AssignmentData data,const QString& path={},bool readOnly=false,bool dirty=false);
    void modified();
    ApplicationShell& shell_;AssignmentGrid grid_;InputDialogs dialogs_;
    std::optional<AssignmentData> data_;std::optional<assignment::Result> result_;
    QString path_;bool dirty_=false,readOnly_=false,pendingValid_=true;
};
AssignmentController& assignmentController(ApplicationShell& shell);
ModuleDescriptor assignmentModule();
bool printAssignment(QPrinter& printer,const AssignmentController& controller,const QFont& font);
}
