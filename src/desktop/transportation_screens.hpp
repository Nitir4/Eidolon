#pragma once

#include "application_shell.hpp"
#include "lp_input_controller.hpp"
#include <tora/compat/transportation_document.hpp>
#include <tora/transportation/north_west.hpp>
#include <tora/transportation/final_summary.hpp>
#include <QObject>

namespace tora::desktop {
class TransportationController final : public QObject {
    Q_OBJECT
public:
    using Axis = compat::TransportationDocument::Axis;
    explicit TransportationController(QObject* parent);
    void load(transportation::Problem problem);
    void loadExample();
    void beginNew(core::NumberFormat format = {});
    bool initialize(const QString& title, int sources, int destinations, QString& error);
    bool editCell(int row, int column, const QString& value, QString& error);
    bool setTitle(const QString& title, QString& error);
    bool openFile(const QString& filename, QString& error, bool readOnly = false);
    bool saveTo(const QString& filename, QString& error);
    bool requestOpen();
    bool requestSave();
    bool leaveInput();
    bool commitInput();
    bool selectLine(Axis axis, int position, bool before = false);
    void clearSelection();
    bool hasSelection() const;
    std::optional<Axis> selectionAxis() const { return selectionAxis_; }
    int selectionIndex() const { return selection_; }
    bool canCopy() const { return hasSelection(); }
    bool canPaste() const;
    bool canUndo() const { return undo_.has_value() && !readOnly_; }
    bool copySelection();
    bool pasteSelection(QString& error);
    bool insertSelection(QString& error);
    bool deleteSelection(QString& error);
    bool undo(QString& error);
    bool initializeMultiplier(transportation::MultiplierAnchor anchor, QString& error);
    transportation::MultiplierAnchor multiplierAnchor() const { return multiplierAnchor_; }
    bool multipliersRefreshed() const { return multipliersRefreshed_; }
    bool canInitializeMultiplier() const { return !trace_.empty() && !multipliersLocked_; }
    void lockMultipliers() { multipliersLocked_=true; }
    bool computeFinal(QString& error);
    const transportation::FinalSummary* finalOutput() const { return final_?&*final_:nullptr; }
    InputDialogs& dialogs() { return dialogs_; }
    const compat::TransportationDocument* document() const { return document_ ? &*document_ : nullptr; }
    bool dirty() const { return dirty_; }
    bool readOnly() const { return readOnly_; }
    const QString& sourcePath() const { return sourcePath_; }
    bool solve(QString& error, transportation::StartMethod method = transportation::StartMethod::NorthWest);
    transportation::StartMethod method() const { return method_; }
    bool loaded() const { return document_.has_value(); }
    const transportation::Problem& problem() const { return document_ ? document_->problem() : emptyProblem_; }
    const std::vector<transportation::Tableau>& trace() const { return trace_; }
signals:
    void changed();
    void inputReset();
    void cellChanged(int row, int column);
    void commitRequested();
    void selectionChanged();
    void multipliersChanged();
    void finalChanged();
private:
    bool reject(QString& error, const QString& message);
    void modified(bool resetInput = true);
    void resetEditState();
    void resetMultipliers();
    transportation::MultiplierAnchor multiplierAnchor_;
    std::optional<transportation::FinalSummary> final_;
    bool multipliersRefreshed_=false, multipliersLocked_=false;
    enum class GridOperation { Insert, Delete, Paste };
    bool changeSelection(QString& error, GridOperation operation);
    std::optional<Axis> selectionAxis_, clipboardAxis_;
    int selection_ = -1;
    bool insertBefore_ = false;
    std::vector<std::string> clipboard_;
    std::optional<compat::TransportationDocument> undo_;
    std::optional<compat::TransportationDocument> document_;
    InputDialogs dialogs_;
    QString sourcePath_, preferredSavePath_;
    bool dirty_ = false, readOnly_ = false, pendingCommitValid_ = true;
    transportation::StartMethod method_ = transportation::StartMethod::NorthWest;
    transportation::Problem emptyProblem_;
    core::NumberFormat newFormat_;
    std::vector<transportation::Tableau> trace_;
};

TransportationController& transportationController(ApplicationShell& shell);
ModuleDescriptor transportationModule();
} // namespace tora::desktop
