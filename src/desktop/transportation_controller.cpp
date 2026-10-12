#include "transportation_screens.hpp"
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
        picker.setFileMode(QFileDialog::ExistingFile); picker.setNameFilter(QStringLiteral("Text Files (*.txt);;All Files (*)"));
        picker.setOption(QFileDialog::DontUseNativeDialog);
        auto* readOnly = new QCheckBox(QStringLiteral("Open as read-only"), &picker);
        if (auto* layout = qobject_cast<QGridLayout*>(picker.layout())) layout->addWidget(readOnly, layout->rowCount(), 0, 1, layout->columnCount());
        polishUsableFonts(picker);
        if (picker.exec() != QDialog::Accepted || picker.selectedFiles().isEmpty()) return {};
        return OpenSelection{picker.selectedFiles().first(), readOnly->isChecked()};
    };
    dialogs_.chooseSave = [owner](const QString& current) -> std::optional<QString> {
        QFileDialog picker(owner, QStringLiteral("Save As"), current); picker.setAcceptMode(QFileDialog::AcceptSave);
        picker.setNameFilter(QStringLiteral("Text Files (*.txt);;All Files (*)")); picker.setDefaultSuffix(QStringLiteral("txt"));
        polishUsableFonts(picker);
        if (picker.exec() != QDialog::Accepted || picker.selectedFiles().isEmpty()) return {};
        return picker.selectedFiles().first();
    };
}
bool TransportationController::reject(QString& error, const QString& message) {
    error = message; pendingCommitValid_ = false; return false;
}
void TransportationController::modified(bool resetInput) {
    dirty_ = true; trace_.clear(); method_ = transportation::StartMethod::NorthWest;
    if (resetInput) emit inputReset();
    emit changed();
}
void TransportationController::load(transportation::Problem problem) {
    auto next = compat::TransportationDocument::create(std::move(problem));
    document_ = std::move(next); trace_.clear(); method_ = transportation::StartMethod::NorthWest;
    dirty_ = false; readOnly_ = false; sourcePath_.clear(); preferredSavePath_.clear(); emit inputReset(); emit changed();
}
void TransportationController::loadExample() {
    auto p = transportation::Problem::blank(2,3);
    p.title = "Transportation example"; p.costs = {{2,3,1},{5,4,8}}; p.supply = {20,30}; p.demand = {10,25,15}; load(std::move(p));
}
void TransportationController::beginNew() {
    document_.reset(); trace_.clear(); dirty_ = false; readOnly_ = false;
    sourcePath_.clear(); preferredSavePath_.clear(); method_ = transportation::StartMethod::NorthWest; emit inputReset(); emit changed();
}
bool TransportationController::initialize(const QString& title, int sources, int destinations, QString& error) {
    error.clear();
    if (readOnly_) return reject(error, QStringLiteral("This Transportation problem is read-only. Save a copy before editing."));
    try {
        auto next = compat::TransportationDocument::blank(sources > 0 ? static_cast<std::size_t>(sources) : 0,
                                                         destinations > 0 ? static_cast<std::size_t>(destinations) : 0, title.toStdString());
        document_ = std::move(next); modified(); return true;
    } catch (const std::exception& e) { return reject(error, QString::fromUtf8(e.what())); }
}
bool TransportationController::editCell(int row, int col, const QString& value, QString& error) {
    error.clear();
    if (!document_) return reject(error, QStringLiteral("Initialize a Transportation problem first."));
    if (readOnly_) return reject(error, QStringLiteral("This Transportation problem is read-only. Save a copy before editing."));
    try {
        const auto r = static_cast<std::size_t>(row), c = static_cast<std::size_t>(col);
        if (document_->cell(r, c) == value.toStdString()) return true;
        document_->setCell(r, c, value.toStdString()); modified(false); emit cellChanged(row, col); return true;
    } catch (const std::exception& e) { return reject(error, QString::fromUtf8(e.what())); }
}
bool TransportationController::setTitle(const QString& title, QString& error) {
    error.clear(); if (!document_) return true;
    if (document_->problem().title == title.toStdString()) return true;
    if (readOnly_) return reject(error, QStringLiteral("This Transportation problem is read-only. Save a copy before editing."));
    try { document_->setTitle(title.toStdString()); modified(); return true; }
    catch (const std::exception& e) { return reject(error, QString::fromUtf8(e.what())); }
}
bool TransportationController::openFile(const QString& filename, QString& error, bool readOnly) {
    error.clear(); QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) return reject(error, file.errorString());
    if (file.size() > 4*1024*1024) return reject(error, QStringLiteral("Transportation file exceeds the 4 MiB limit."));
    try {
        const auto bytes = file.readAll();
        if (file.error() != QFileDevice::NoError) return reject(error, file.errorString());
        auto next = compat::TransportationDocument::decode(std::string_view(bytes.constData(), static_cast<std::size_t>(bytes.size())));
        document_ = std::move(next); trace_.clear(); method_ = transportation::StartMethod::NorthWest;
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
        const auto bytes = document_->encode(); QSaveFile file(filename);
        if (!file.open(QIODevice::WriteOnly)) return reject(error, file.errorString());
        if (file.write(bytes.data(), static_cast<qint64>(bytes.size())) != static_cast<qint64>(bytes.size())) return reject(error, file.errorString());
        if (!file.commit()) return reject(error, file.errorString());
        sourcePath_ = filename; preferredSavePath_ = filename; dirty_ = false; readOnly_ = false; emit inputReset(); emit changed(); return true;
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
        trace_ = std::move(result); method_ = method; emit changed(); return true;
    } catch (const std::exception& e) { return reject(error, QString::fromUtf8(e.what())); }
}
TransportationController& transportationController(ApplicationShell& shell) {
    if (auto* existing = shell.findChild<TransportationController*>(QStringLiteral("transportation-controller"))) return *existing;
    return *new TransportationController(&shell);
}
}
