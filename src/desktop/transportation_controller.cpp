#include "transportation_screens.hpp"
#include "model_json.hpp"
#include "ui_font.hpp"
#include <QCheckBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QMessageBox>
#include <QSaveFile>
#include <exception>

namespace tora::desktop {
namespace {
QString pathIdentity(const QString& path) {
    const QFileInfo info(path);
    return info.exists() ? info.canonicalFilePath() : info.absoluteFilePath();
}
}
TransportationController::TransportationController(QObject* parent) : QObject(parent) {
    setObjectName(QStringLiteral("transportation-controller"));
    auto* owner = qobject_cast<QWidget*>(parent);
    dialogs_.showError = [owner](const QString& title, const QString& body) { QMessageBox::warning(owner, title, body); };
    dialogs_.askSave = [owner] {
        const auto answer = QMessageBox::question(owner, QStringLiteral("Save?"), QStringLiteral("Do you wish to save data?"),
                                                  QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        return answer == QMessageBox::Yes ? SaveChoice::Yes : answer == QMessageBox::No ? SaveChoice::No : SaveChoice::Cancel;
    };
    dialogs_.chooseOpen = [owner]() -> std::optional<OpenSelection> {
        QFileDialog picker(owner, QStringLiteral("Open Transportation Problem"));
        picker.setFileMode(QFileDialog::ExistingFile); picker.setNameFilter(QStringLiteral("Transportation TXT (*.txt);;Transportation JSON (*.json);;All Files (*)"));
        picker.setOption(QFileDialog::DontUseNativeDialog);
        auto* readOnly = new QCheckBox(QStringLiteral("Open as read-only"), &picker);
        if (auto* layout = qobject_cast<QGridLayout*>(picker.layout())) layout->addWidget(readOnly, layout->rowCount(), 0, 1, layout->columnCount());
        polishUsableFonts(picker);
        if (picker.exec() != QDialog::Accepted || picker.selectedFiles().isEmpty()) return {};
        return OpenSelection{picker.selectedFiles().first(), readOnly->isChecked()};
    };
    dialogs_.chooseSave = [owner](const QString& current) -> std::optional<QString> {
        QFileDialog picker(owner, QStringLiteral("Save As"), current); picker.setAcceptMode(QFileDialog::AcceptSave);
        picker.setNameFilter(QStringLiteral("Transportation TXT (*.txt);;Transportation JSON (*.json);;All Files (*)")); picker.setDefaultSuffix(QStringLiteral("txt"));
        polishUsableFonts(picker);
        if (picker.exec() != QDialog::Accepted || picker.selectedFiles().isEmpty()) return {};
        return picker.selectedFiles().first();
    };
}
bool TransportationController::reject(QString& error, const QString& message) {
    error = message; pendingCommitValid_ = false; return false;
}
void TransportationController::modified(bool resetInput) {
    final_.reset();emit finalChanged();
    dirty_ = true; trace_.clear(); method_ = transportation::StartMethod::NorthWest;
    resetMultipliers();
    if (resetInput) emit inputReset();
    emit changed();
}
void TransportationController::resetEditState() {
    final_.reset();emit finalChanged();
    resetMultipliers();
    selectionAxis_.reset(); selection_ = -1; insertBefore_ = false;
    clipboardAxis_.reset(); clipboard_.clear(); undo_.reset(); emit selectionChanged();
}
void TransportationController::resetMultipliers() {
    multiplierAnchor_={};multipliersRefreshed_=false;multipliersLocked_=false;
}
bool TransportationController::initializeMultiplier(transportation::MultiplierAnchor anchor, QString& error) {
    error.clear();
    if(!canInitializeMultiplier())return reject(error,QStringLiteral("Initialize a multiplier before advancing the first tableau."));
    try {
        auto candidate=trace_;
        for(auto& tableau:candidate)tableau=transportation::initializeMultipliers(problem(),tableau,anchor);
        trace_=std::move(candidate);multiplierAnchor_=anchor;multipliersRefreshed_=true;
        emit multipliersChanged();return true;
    } catch(const std::exception& e) {return reject(error,QString::fromUtf8(e.what()));}
}
bool TransportationController::computeFinal(QString& error) {
    error.clear();
    if(!commitInput())return reject(error,QStringLiteral("Correct the active Transportation input before solving."));
    if(!document_)return reject(error,QStringLiteral("Open or initialize a Transportation problem first."));
    try {
        auto candidate=transportation::finalSummary(problem());
        final_=std::move(candidate);emit finalChanged();return true;
    }catch(const std::exception& e){return reject(error,QString::fromUtf8(e.what()));}
}
bool TransportationController::hasSelection() const {
    if (!document_ || !selectionAxis_ || selection_ < 0) return false;
    return static_cast<std::size_t>(selection_) < (*selectionAxis_ == Axis::Source ? problem().supply.size() : problem().demand.size());
}
void TransportationController::clearSelection() {
    selectionAxis_.reset(); selection_ = -1; insertBefore_ = false; emit selectionChanged();
}
bool TransportationController::selectLine(Axis axis, int position, bool before) {
    if (!commitInput()) return false;
    selectionAxis_ = axis; selection_ = position; insertBefore_ = before;
    if (!hasSelection()) { clearSelection(); return false; }
    emit selectionChanged(); return true;
}
bool TransportationController::canPaste() const {
    return hasSelection() && !readOnly_ && clipboardAxis_ == selectionAxis_
        && clipboard_.size() == (*selectionAxis_ == Axis::Source ? problem().demand.size() + 2 : problem().supply.size() + 2);
}
bool TransportationController::copySelection() {
    if (!commitInput() || !hasSelection()) return false;
    clipboard_ = document_->line(*selectionAxis_, static_cast<std::size_t>(selection_));
    clipboardAxis_ = selectionAxis_; emit selectionChanged(); return true;
}
bool TransportationController::changeSelection(QString& error, GridOperation operation) {
    error.clear();
    if (!commitInput()) return reject(error, QStringLiteral("Correct the active Transportation input before editing the grid."));
    if (readOnly_) return reject(error, QStringLiteral("This Transportation problem is read-only. Save a copy before editing."));
    if (!hasSelection()) return reject(error, QStringLiteral("Select a source or destination heading first."));
    if (operation == GridOperation::Delete &&
        (*selectionAxis_ == Axis::Source ? problem().supply.size() : problem().demand.size()) <= 2)
        return reject(error, QStringLiteral("EditGrid keeps at least two sources and two destinations."));
    if (operation == GridOperation::Paste && !canPaste())
        return reject(error, QStringLiteral("Copy a matching source or destination line for this grid first."));
    try {
        auto next = *document_; const auto at = static_cast<std::size_t>(selection_);
        if (operation == GridOperation::Insert) next.insert(*selectionAxis_, at + (insertBefore_ ? 0 : 1));
        else if (operation == GridOperation::Delete) next.erase(*selectionAxis_, at);
        else next.paste(*selectionAxis_, at, clipboard_);
        undo_ = document_; document_ = std::move(next); clearSelection(); modified(); return true;
    } catch (const std::exception& e) { return reject(error, QString::fromUtf8(e.what())); }
}
bool TransportationController::pasteSelection(QString& error) { return changeSelection(error, GridOperation::Paste); }
bool TransportationController::insertSelection(QString& error) { return changeSelection(error, GridOperation::Insert); }
bool TransportationController::deleteSelection(QString& error) { return changeSelection(error, GridOperation::Delete); }
bool TransportationController::undo(QString& error) {
    error.clear();
    if (!commitInput()) return reject(error, QStringLiteral("Correct the active Transportation input before undo."));
    if (!canUndo()) return reject(error, QStringLiteral("There is no Transportation grid operation to undo."));
    document_ = std::move(undo_); undo_.reset(); clearSelection(); modified(); return true;
}
void TransportationController::load(transportation::Problem problem) {
    auto next = compat::TransportationDocument::create(std::move(problem));
    document_ = std::move(next); newFormat_=document_->problem().format; resetEditState(); trace_.clear(); method_ = transportation::StartMethod::NorthWest;
    dirty_ = false; readOnly_ = false; sourcePath_.clear(); preferredSavePath_.clear(); emit inputReset(); emit changed();
}
void TransportationController::loadExample() {
    auto p = transportation::Problem::blank(2,3);
    p.title = "Transportation example"; p.costs = {{2,3,1},{5,4,8}}; p.supply = {20,30}; p.demand = {10,25,15}; load(std::move(p));
}
void TransportationController::beginNew(core::NumberFormat format) {
    (void)core::NumberFormatter(format);newFormat_=format;
    document_.reset(); resetEditState(); trace_.clear(); dirty_ = false; readOnly_ = false;
    sourcePath_.clear(); preferredSavePath_.clear(); method_ = transportation::StartMethod::NorthWest; emit inputReset(); emit changed();
}
bool TransportationController::initialize(const QString& title, int sources, int destinations, QString& error) {
    error.clear();
    if (readOnly_) return reject(error, QStringLiteral("This Transportation problem is read-only. Save a copy before editing."));
    try {
        auto next = compat::TransportationDocument::blank(sources > 0 ? static_cast<std::size_t>(sources) : 0,
                                                         destinations > 0 ? static_cast<std::size_t>(destinations) : 0, title.toStdString(),newFormat_);
        document_ = std::move(next); resetEditState(); modified(); return true;
    } catch (const std::exception& e) { return reject(error, QString::fromUtf8(e.what())); }
}
bool TransportationController::editCell(int row, int col, const QString& value, QString& error) {
    error.clear();
    if (!document_) return reject(error, QStringLiteral("Initialize a Transportation problem first."));
    if (readOnly_) return reject(error, QStringLiteral("This Transportation problem is read-only. Save a copy before editing."));
    try {
        const auto r = static_cast<std::size_t>(row), c = static_cast<std::size_t>(col);
        if (document_->cell(r, c) == value.toStdString()) return true;
        document_->setCell(r, c, value.toStdString()); undo_.reset(); modified(false); emit cellChanged(row, col); return true;
    } catch (const std::exception& e) { return reject(error, QString::fromUtf8(e.what())); }
}
bool TransportationController::setTitle(const QString& title, QString& error) {
    error.clear(); if (!document_) return true;
    if (document_->problem().title == title.toStdString()) return true;
    if (readOnly_) return reject(error, QStringLiteral("This Transportation problem is read-only. Save a copy before editing."));
    try { document_->setTitle(title.toStdString()); undo_.reset(); modified(); return true; }
    catch (const std::exception& e) { return reject(error, QString::fromUtf8(e.what())); }
}
bool TransportationController::openFile(const QString& filename, QString& error, bool readOnly) {
    error.clear(); QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) return reject(error, file.errorString());
    if (file.size() > 4*1024*1024) return reject(error, QStringLiteral("Transportation file exceeds the 4 MiB limit."));
    try {
        const auto bytes = file.readAll();
        if (file.error() != QFileDevice::NoError) return reject(error, file.errorString());
        auto next = bytes.trimmed().startsWith("{")?decodeTransportationJson(bytes):compat::TransportationDocument::decode(std::string_view(bytes.constData(), static_cast<std::size_t>(bytes.size())));
        document_ = std::move(next); newFormat_=document_->problem().format; resetEditState(); trace_.clear(); method_ = transportation::StartMethod::NorthWest;
        sourcePath_ = filename; preferredSavePath_ = filename; dirty_ = false; readOnly_ = readOnly; emit inputReset(); emit changed(); return true;
    } catch (const std::exception& e) { return reject(error, QString::fromUtf8(e.what())); }
}
bool TransportationController::commitInput() {
    pendingCommitValid_ = true; emit commitRequested(); return pendingCommitValid_;
}
bool TransportationController::saveTo(const QString& filename, QString& error) {
    error.clear();
    if (!commitInput()) return reject(error, QStringLiteral("Correct the active Transportation input before saving."));
    if (!document_) return reject(error, QStringLiteral("Initialize a Transportation problem first."));
    if (filename.isEmpty()) return reject(error, QStringLiteral("Choose a Transportation filename."));
    if (readOnly_ && pathIdentity(filename) == pathIdentity(sourcePath_)) return reject(error, QStringLiteral("The source was opened read-only. Save to a different file."));
    try {
        const auto bytes = filename.endsWith(".json",Qt::CaseInsensitive)?encodeTransportationJson(*document_).toStdString():document_->encode(); QSaveFile file(filename);
        if (!file.open(QIODevice::WriteOnly)) return reject(error, file.errorString());
        if (file.write(bytes.data(), static_cast<qint64>(bytes.size())) != static_cast<qint64>(bytes.size())) return reject(error, file.errorString());
        if (!file.commit()) return reject(error, file.errorString());
        sourcePath_ = filename; preferredSavePath_ = filename; dirty_ = false; readOnly_ = false; clearSelection(); emit inputReset(); emit changed(); return true;
    } catch (const std::exception& e) { return reject(error, QString::fromUtf8(e.what())); }
}
bool TransportationController::requestSave() {
    if (!commitInput() || !document_) return false;
    const auto path = dialogs_.chooseSave(preferredSavePath_); if (!path) return false;
    QString error; if (saveTo(*path, error)) return true;
    dialogs_.showError(QStringLiteral("Cannot Save Problem"), error); return false;
}
bool TransportationController::leaveInput() {
    if (!commitInput()) return false;
    if (!dirty_) return true;
    const auto answer = dialogs_.askSave();
    return answer == SaveChoice::No || (answer == SaveChoice::Yes && requestSave());
}
bool TransportationController::requestOpen() {
    if (!commitInput()) return false;
    const auto selected = dialogs_.chooseOpen(); if (!selected || !leaveInput()) return false;
    QString error; if (openFile(selected->path, error, selected->readOnly)) return true;
    dialogs_.showError(QStringLiteral("Cannot Open Problem"), error); return false;
}
bool TransportationController::solve(QString& error, transportation::StartMethod method) {
    error.clear();
    if (!commitInput()) return reject(error, QStringLiteral("Correct the active Transportation input before solving."));
    if (!document_) return reject(error, QStringLiteral("Open or initialize a Transportation problem first."));
    try {
        const auto& p = problem(); auto result = transportation::solve(p, method);
        const auto cellsPerTable = (3*p.supply.size()+4)*(p.demand.size()+4);
        if (result.size() > 250000/cellsPerTable) return reject(error, QStringLiteral("This Transportation preview limits the displayed trace to 250,000 cells. The problem requires a larger trace."));
        trace_ = std::move(result); method_ = method; resetMultipliers(); emit changed(); return true;
    } catch (const std::exception& e) { return reject(error, QString::fromUtf8(e.what())); }
}
TransportationController& transportationController(ApplicationShell& shell) {
    if (auto* existing = shell.findChild<TransportationController*>(QStringLiteral("transportation-controller"))) return *existing;
    return *new TransportationController(&shell);
}
}
