#pragma once
#include "application_shell.hpp"
#include "lp_input_controller.hpp"
#include "planning_document.hpp"
#include <tora/planning/planning.hpp>
#include <QObject>
#include <optional>
class QPrinter;
namespace tora::desktop {
class PlanningController final:public QObject {
 Q_OBJECT
public:
 explicit PlanningController(QObject* parent);
 void beginNew(planning::Method method=planning::Method::Cpm);
 bool setProblem(const QString& title,const planning::Problem& problem,QString& error);
 bool solve(QString& error);
 bool setFormat(core::NumberFormat format,QString& error);
 bool shiftActivity(std::size_t activity,double relativeShift,QString& error);
 void resetSchedule();
 const planning::Schedule* schedule()const{return schedule_?&*schedule_:nullptr;}
 core::NumberFormat format()const{return format_;}
 bool openFile(const QString& path,QString& error,bool readOnly=false);
 bool saveTo(const QString& path,QString& error);
 void rejectInput(){pendingValid_=false;}
 bool commitInput();bool requestSave();bool requestOpen();bool leaveInput();
 InputDialogs& dialogs(){return dialogs_;}
 const planning::Problem& problem()const{return problem_;}
 const planning::Result* result()const{return result_?&*result_:nullptr;}
 QString title()const{return title_;}bool dirty()const{return dirty_;}bool readOnly()const{return readOnly_;}
 QString sourcePath()const{return path_;}
 QString outputText()const;
signals:void gridEditRequested(int operation);void changed();void inputReset();void commitRequested();
private:
 bool reject(QString& error,const QString& message);
 PlanningDocument document_;core::NumberFormat format_;planning::Problem problem_;std::optional<planning::Result> result_;std::optional<planning::Schedule> schedule_;std::vector<double> scheduleDelays_;QString title_,path_;bool dirty_=false,readOnly_=false,pendingValid_=true;InputDialogs dialogs_;
};
PlanningController& planningController(ApplicationShell& shell);
ModuleDescriptor planningModule();
bool printPlanning(QPrinter& printer,const PlanningController& controller);
bool printPlanningSchedule(QPrinter& printer,const PlanningController& controller);
}
