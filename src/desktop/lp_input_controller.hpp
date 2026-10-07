#pragma once
#include <tora/compat/lp_file.hpp>
#include <QAbstractTableModel>
#include <QObject>
#include <functional>
#include <optional>

namespace tora::desktop {
enum class SaveChoice { Yes, No, Cancel };
struct OpenSelection { QString path; bool readOnly = false; };
struct InputDialogs {
    std::function<void(const QString&, const QString&)> showError;
    std::function<SaveChoice()> askSave;
    std::function<bool(const QString&,const QString&)> askRetry;
    std::function<std::optional<OpenSelection>()> chooseOpen;
    std::function<std::optional<QString>(const QString&)> chooseSave;
};
class LpInputController;
class LpGridModel final : public QAbstractTableModel {
    Q_OBJECT
public:
    explicit LpGridModel(LpInputController& controller);
    int rowCount(const QModelIndex& parent = {}) const override;
    int columnCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    void refresh();
    void valuesChanged();
    void selectionChanged();
private:
    LpInputController& controller_;
};
class LpInputController final : public QObject {
    Q_OBJECT
public:
    explicit LpInputController(QWidget* owner = nullptr);
    InputDialogs& dialogs() { return dialogs_; }
    const tora::compat::LpDocument* document() const { return document_ ? &*document_ : nullptr; }
    LpGridModel* grid() { return &grid_; }
    void beginNew(tora::core::NumberFormat format = {});
    bool initialize(const QString& title, int variables, int constraints);
    bool setTitle(const QString& title);
    bool openFile(const QString& path, bool readOnly = false);
    bool saveTo(const QString& path);
    bool requestOpen();
    bool requestSave();
    bool leaveInput();
    bool validateForSolve();
    bool toggleObjective();
    void selectRow(int row);
    void selectColumn(int column);
    void clearSelection();
    bool copySelection();
    bool pasteSelection();
    bool deleteSelection();
    bool insertSelection(bool before = false);
    bool canCopy() const;
    bool canPaste() const;
    bool hasSelection() const { return selectionAxis_ != Axis::None; }
    bool editing() const { return editing_; }
    void setEditing(bool active) { editing_=active;emit selectionChanged(); }
    QString lastErrorTitle() const { return errorTitle_; }
    QString lastError() const { return error_; }
    QString captionPath() const { return captionPath_; }
signals:
    void changed();
    void selectionChanged();
    void commitRequested();
    void editRequested(int row,int column);
private:
    friend class LpGridModel;
    enum class Axis { None, Row, Column };
    bool fail(const QString& title, const QString& message);
    bool commitInput();
    bool replaceGrid(std::vector<std::string> cells);
    bool editCell(int row, int column, const QString& value);
    bool reshape(bool insert, bool before);
    void refresh();
    std::optional<tora::compat::LpDocument> document_;
    tora::core::NumberFormat newFormat_;
    std::vector<std::string> cells_;
    LpGridModel grid_;
    InputDialogs dialogs_;
    QString errorTitle_, error_, captionPath_;
    QString preferredSavePath_;
    Axis selectionAxis_ = Axis::None, clipboardAxis_ = Axis::None;
    int selection_ = -1;
    bool editing_ = false;
    bool pendingCommitValid_ = true;
    bool openingFromPicker_ = false, retryOpen_ = false;
    std::vector<std::string> clipboard_;
};
LpInputController& inputController(QWidget& shell);
QString legacyText(const std::string& bytes);
} // namespace tora::desktop
