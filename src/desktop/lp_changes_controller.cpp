#include "lp_changes_controller.hpp"
#include "lp_input_controller.hpp"
#include "ui_font.hpp"
#include <QMessageBox>
#include <cmath>
#include <stdexcept>
namespace tora::desktop {
LpChangesController::LpChangesController(ApplicationShell& shell):QObject(&shell),shell_(shell) {
    setObjectName(QStringLiteral("lp-changes-controller"));
    notice=[&shell](const QString& title,const QString& text){QMessageBox box(QMessageBox::Information,title,text,QMessageBox::Ok,&shell);polishUsableFonts(box);box.exec();};
    connect(&inputController(shell),&LpInputController::changed,this,[this]{baseline_.reset();clearResult();emit baselineChanged();});
}
void LpChangesController::clearResult(){fresh_.reset();revised_.reset();changed_.reset();beforeValues_.clear();beforeObjective_.reset();emit resultChanged();}
bool LpChangesController::request(LpChangeMethod method) {
    auto& input=inputController(shell_);
    if(!input.document() || !input.validateForSolve())return false;
    if(!shell_.navigate(QStringLiteral("lp/changes")))return false;
    baseline_=*input.document();method_=method;format_=baseline_->inputFormat;
    clearResult();emit baselineChanged();return true;
}
bool LpChangesController::analyze(const std::vector<double>& objective,const std::vector<double>& rhs,tora::core::NumberFormat format) {
    clearResult();if(!baseline_)return false;
    try {
        (void)tora::core::NumberFormatter(format);
        const auto& base=baseline_->problem;
        if(objective.size()!=base.variables.size() || rhs.size()!=base.constraints.size())throw std::invalid_argument("Enter one coefficient per variable and one RHS per constraint");
        for(auto x:objective)if(!std::isfinite(x))throw std::invalid_argument("Enter finite objective coefficients");
        for(auto x:rhs)if(!std::isfinite(x))throw std::invalid_argument("Enter finite RHS values");
        auto changed=base;changed.objective=objective;
        for(std::size_t i=0;i<rhs.size();++i)changed.constraints[i].rhs=rhs[i];
        if(method_==LpChangeMethod::RevisedDual) {
            if(objective!=base.objective)throw std::invalid_argument("Dual Revised changes RHS only; use Change Cj / RHS for objective changes");
            revised_=tora::lp::reoptimizeDualRevisedRhs(base,rhs);
            const auto before=tora::lp::solveDualRevisedModel(tora::lp::makeDualRevisedModel(base),revised_->canonical.trace.front().basis,0);
            beforeValues_=before.values;beforeObjective_=before.objective;
        }else {
            fresh_=tora::lp::analyzeChanges(base,objective,rhs);
            beforeValues_=fresh_->before.values;beforeObjective_=fresh_->before.objective;
        }
        changed_=std::move(changed);format_=format;emit resultChanged();
        return shell_.navigate(QStringLiteral("lp/changes-result"));
    }catch(const std::exception& e){clearResult();notice(QStringLiteral("LP changes"),QString::fromUtf8(e.what()));return false;}
}
bool LpChangesController::apply() {
    if(!hasResult() || !changed_)return false;
    const auto objective=changed_->objective;std::vector<double> rhs;
    for(const auto& row:changed_->constraints)rhs.push_back(row.rhs);
    if(!inputController(shell_).applyCoefficientChanges(objective,rhs))return false;
    return shell_.navigate(QStringLiteral("lp/input"));
}
LpChangesController& changesController(ApplicationShell& shell) {
    if(auto* c=shell.findChild<LpChangesController*>(QStringLiteral("lp-changes-controller")))return *c;
    return *new LpChangesController(shell);
}
}
