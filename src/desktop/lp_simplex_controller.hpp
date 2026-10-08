#pragma once
#include "application_shell.hpp"
#include <tora/lp/all_slack.hpp>
#include <tora/lp/m_method.hpp>
#include <tora/lp/two_phase.hpp>
#include <tora/lp/bounded.hpp>
#include <QObject>
#include <QTimer>
#include <functional>
#include <memory>

class QPrinter;
namespace tora::desktop {
class LpSimplexController final:public QObject {
    Q_OBJECT
public:
    explicit LpSimplexController(ApplicationShell& shell);
    std::function<void(const QString&,const QString&)> notice;
    std::function<bool(const QString&)> redirect; // true = OK, false = Cancel; each method preserves its captured handoff
    bool requestAllSlack();
    bool requestMMethod();
    bool requestTwoPhase();
    bool requestBounded();
    bool canEnterM() const {return mPending_;}
    bool setPenalty(const QString& text);
    bool canStart() const {return requested_;}
    bool start(tora::core::NumberFormat format);
    const tora::lp::PrimalSimplexSession* session() const {return session_.get();}
    tora::core::NumberFormat format() const {return format_;}
    bool busy() const {return busy_;}
    bool canNext() const;
    void selectCell(int row,int column);
    void next();
    void all();
signals:
    void changed();
private:
    void runOne();
    void terminal();
    ApplicationShell& shell_;
    std::unique_ptr<tora::lp::PrimalSimplexSession> session_;
    tora::core::NumberFormat format_;
    bool requested_=false,busy_=false,mPending_=false;
    tora::lp::PrimalMethod requestedMethod_=tora::lp::PrimalMethod::AllSlack;
    double penalty_=100;
    std::size_t automaticSteps_=0;
    QTimer timer_;
};
LpSimplexController& simplexController(ApplicationShell& shell);
QWidget* simplexScreen(ApplicationShell& shell);
bool printSimplexTrace(QPrinter& printer,const LpSimplexController& controller,const QFont& font);
} // namespace tora::desktop
