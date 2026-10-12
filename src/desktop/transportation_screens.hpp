#pragma once

#include "application_shell.hpp"
#include "lp_input_controller.hpp"
#include <tora/compat/transportation_document.hpp>
#include <tora/transportation/north_west.hpp>
#include <QObject>

namespace tora::desktop {
class TransportationController final : public QObject {
    Q_OBJECT
public:
    explicit TransportationController(QObject* parent);
    void load(transportation::Problem problem);
    void loadExample();
    void beginNew();
    bool initialize(const QString& title, int sources, int destinations, QString& error);
    bool editCell(int row, int column, const QString& value, QString& error);
    bool setTitle(const QString& title, QString& error);
    bool openFile(const QString& filename, QString& error, bool readOnly = false);
    bool saveTo(const QString& filename, QString& error);
    bool requestOpen();
    bool requestSave();
    bool leaveInput();
    bool commitInput();
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
private:
    bool reject(QString& error, const QString& message);
    void modified(bool resetInput = true);
    std::optional<compat::TransportationDocument> document_;
    InputDialogs dialogs_;
    QString sourcePath_, preferredSavePath_;
    bool dirty_ = false, readOnly_ = false, pendingCommitValid_ = true;
    transportation::StartMethod method_ = transportation::StartMethod::NorthWest;
    transportation::Problem emptyProblem_;
    std::vector<transportation::Tableau> trace_;
};

TransportationController& transportationController(ApplicationShell& shell);
ModuleDescriptor transportationModule();
} // namespace tora::desktop
