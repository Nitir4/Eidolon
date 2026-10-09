#pragma once
#include "application_shell.hpp"
#include <tora/lp/final_solution.hpp>
#include <QObject>
#include <memory>

class QPrinter;
namespace tora::desktop {
class LpFinalController final:public QObject {
    Q_OBJECT
public:
    explicit LpFinalController(ApplicationShell& shell);
    std::function<void(const QString&,const QString&)> notice;
    bool request();
    bool canStart() const {return requested_;}
    bool start(tora::core::NumberFormat format);
    const tora::lp::FinalSolutionSession* session() const {return session_.get();}
    tora::core::NumberFormat format() const {return format_;}
signals:
    void changed();
private:
    ApplicationShell& shell_;
    bool requested_=false;
    tora::core::NumberFormat format_;
    std::unique_ptr<tora::lp::FinalSolutionSession> session_;
};
LpFinalController& finalController(ApplicationShell& shell);
QWidget* finalScreen(ApplicationShell& shell);
bool printSummary(QPrinter& printer,const LpFinalController& controller,const QFont& font);
} // namespace tora::desktop
