#pragma once
#include "application_shell.hpp"
#include <tora/compat/lp_file.hpp>
#include <tora/lp/postoptimal.hpp>
#include <tora/lp/dual_revised_model.hpp>
#include <QObject>
#include <optional>
class QPrinter;
class QFont;
namespace tora::desktop {
enum class LpChangeMethod { Fresh, RevisedDual };
class LpChangesController final:public QObject {
    Q_OBJECT
public:
    explicit LpChangesController(ApplicationShell& shell);
    std::function<void(const QString&,const QString&)> notice;
    bool request(LpChangeMethod method);
    bool analyze(const std::vector<double>& objective,const std::vector<double>& rhs,tora::core::NumberFormat format);
    bool apply();
    void clearResult();
    const tora::compat::LpDocument* baseline() const {return baseline_?&*baseline_:nullptr;}
    LpChangeMethod method() const {return method_;}
    const std::optional<tora::lp::PostOptimalResult>& fresh() const {return fresh_;}
    const std::optional<tora::lp::RevisedDualLpResult>& revised() const {return revised_;}
    const std::optional<tora::lp::LpProblem>& changedProblem() const {return changed_;}
    tora::core::NumberFormat format() const {return format_;}
    const std::vector<double>& baselineValues() const {return beforeValues_;}
    std::optional<double> baselineObjective() const {return beforeObjective_;}
    bool hasResult() const {return fresh_.has_value() || revised_.has_value();}
signals:
    void baselineChanged();
    void resultChanged();
private:
    ApplicationShell& shell_;
    LpChangeMethod method_=LpChangeMethod::Fresh;
    tora::core::NumberFormat format_;
    std::optional<tora::compat::LpDocument> baseline_;
    std::optional<tora::lp::LpProblem> changed_;
    std::vector<double> beforeValues_;
    std::optional<double> beforeObjective_;
    std::optional<tora::lp::PostOptimalResult> fresh_;
    std::optional<tora::lp::RevisedDualLpResult> revised_;
};
LpChangesController& changesController(ApplicationShell& shell);
QWidget* changesScreen(ApplicationShell& shell);
QWidget* changesResultScreen(ApplicationShell& shell);
bool printChanges(QPrinter& printer,const LpChangesController& controller,const QFont& font,std::size_t traceStep=0);
}
