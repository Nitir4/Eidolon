#include "transportation_screens.hpp"
#include "keyboard_navigation.hpp"
#include "transportation_printing.hpp"
#include "screen_widgets.hpp"
#include "ui_colors.hpp"
#include "ui_font.hpp"
#include <tora/compat/transportation_reader.hpp>
#include <tora/core/number_formatter.hpp>

#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QIntValidator>
#include <QPointer>
#include <QPersistentModelIndex>
#include <QKeyEvent>
#include <QScopedValueRollback>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QResizeEvent>
#include <QStyle>
#include <QTableWidget>
#include <QTabWidget>
#include <QStyledItemDelegate>
#include <QSignalBlocker>
#include <QTimer>
#include <QComboBox>
#include <QRadioButton>
#include <QVBoxLayout>
#include <algorithm>
#include <exception>
#include <memory>
#include <set>

namespace tora::desktop {
namespace {
QString text(const std::string& value) { return QString::fromUtf8(value.data(), static_cast<qsizetype>(value.size())); }
QString id(const char* suffix) { return QStringLiteral("transportation/") + QString::fromLatin1(suffix); }

class TransportationTable final : public QTableWidget {
public:
    explicit TransportationTable(QWidget* parent):QTableWidget(parent) {}
    void fitColumns(int firstDestination, int destinations) {
        firstDestination_=firstDestination; destinations_=destinations; fitColumns();
    }
protected:
    void resizeEvent(QResizeEvent* event) override {
        QTableWidget::resizeEvent(event); fitColumns();
    }
private:
    void fitColumns() {
        if (fitting_ || !columnCount()) return;
        fitting_=true;
        const int padding=2*fontMetrics().averageCharWidth()+2*style()->pixelMetric(QStyle::PM_FocusFrameHMargin);
        std::vector<int> widths(static_cast<std::size_t>(columnCount()),padding);
        // Qt's content-size hint omits some spanned cells. Measure all text,
        // including u multipliers, before distributing space to destinations.
        for (int row=0; row<rowCount(); ++row) for (int col=0; col<columnCount(); ++col) {
            const auto* cell=item(row,col); if (!cell) continue;
            const QFontMetrics metrics(cell->data(Qt::FontRole).isValid()?cell->font():font());
            widths[static_cast<std::size_t>(col)]=std::max(widths[static_cast<std::size_t>(col)],metrics.horizontalAdvance(cell->text())+padding);
        }
        int total=0;for (int width:widths) total+=width;
        int extra=std::max(0,viewport()->width()-total);
        // A model reload can resize the viewport synchronously while the old
        // destination count is still cached. Use only columns that exist now.
        const int destinations=std::max(0,std::min(destinations_,columnCount()-firstDestination_));
        for (int j=0; j<destinations; ++j) {
            const int share=extra/(destinations-j);
            widths.at(static_cast<std::size_t>(firstDestination_+j))+=share; extra-=share;
        }
        for (int col=0; col<columnCount(); ++col) setColumnWidth(col,widths[static_cast<std::size_t>(col)]);
        fitting_=false;
    }
    int firstDestination_=0, destinations_=0;
    bool fitting_=false;
};

QPushButton* addButton(QHBoxLayout& row, const QString& label, const char* name,
                      QWidget* context, std::function<void()> action)
{
    auto* button = new QPushButton(label);
    button->setObjectName(QString::fromLatin1(name));
    row.addWidget(button);
    QObject::connect(button, &QPushButton::clicked, context, std::move(action));
    return button;
}
void footer(QVBoxLayout& layout, ApplicationShell& shell, bool viewInput)
{
    auto* row = new QHBoxLayout;
    row->addStretch();
    if (viewInput) addButton(*row, QStringLiteral("View Input Data"), "transport-view-input", &shell,
                            [&shell] { shell.navigate(id("input")); });
    addButton(*row, QStringLiteral("MAIN Menu"), "transport-main", &shell,
              [&shell] { shell.navigate(QStringLiteral("main")); });
    addButton(*row, QStringLiteral("Exit Eidolon"), "transport-exit", &shell, [&shell] { shell.close(); });
    row->addStretch();
    layout.addLayout(row);
}
void openProblem(ApplicationShell& shell)
{
    if (transportationController(shell).requestOpen()) shell.navigate(id("input"));
}
void newProblem(ApplicationShell& shell,core::NumberFormat format = {})
{
    auto& c = transportationController(shell);
    try {(void)core::NumberFormatter(format);}
    catch(const std::exception& e){c.dialogs().showError(QStringLiteral("Number Format"),QString::fromUtf8(e.what()));return;}
    if (!c.leaveInput()) return;
    c.beginNew(format); shell.navigate(id("input"));
}
void solveProblem(ApplicationShell& shell, transportation::StartMethod method = transportation::StartMethod::NorthWest)
{
    // Direct menu actions must honor Save/Cancel before computing a trace or
    // displaying a terminal notice, just like the SOLVE Menu button route.
    if (shell.currentScreen()==id("input") && !shell.navigate(id("solve-menu"))) return;
    QString error;
    if (!transportationController(shell).solve(error,method)) {
        QMessageBox::warning(&shell, QStringLiteral("Cannot Solve Problem"), error);
        return;
    }
    if (!shell.navigate(id(method==transportation::StartMethod::Vogel?"vogel":method==transportation::StartMethod::LeastCost?"leastcost":"northwest"))) return;
    if(transportationController(shell).trace().size()==1)
        QMessageBox::information(&shell,QStringLiteral("Optimal Tableau"),QStringLiteral("Iteration 1 is optimal"));
}
void finalProblem(ApplicationShell& shell) {
    if(shell.currentScreen()==id("input") && !shell.navigate(id("solve-menu")))return;
    auto& controller=transportationController(shell);QString error;
    if(!controller.computeFinal(error)) {
        controller.dialogs().showError(QStringLiteral("Cannot Solve Problem"),error);return;
    }
    shell.navigate(id("final"));
}
TransportationTable* table(QWidget* parent, const char* name)
{
    auto* grid = new TransportationTable(parent);
    grid->setObjectName(QString::fromLatin1(name));
    grid->setFont(readableUiFont(parent->font()));
    grid->setEditTriggers(QAbstractItemView::NoEditTriggers);
    grid->horizontalHeader()->hide();
    grid->verticalHeader()->hide();
    grid->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    grid->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    grid->horizontalHeader()->setStretchLastSection(false);
    grid->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    grid->setStyleSheet(QStringLiteral("QTableWidget { background: #ffffff; color: #172b4d; gridline-color: #879bb5; selection-background-color: #225ea8; selection-color: white; }"));
    return grid;
}
QTableWidgetItem* put(QTableWidget& grid, int row, int col, const QString& value, bool header = false)
{
    auto* item = new QTableWidgetItem(value);
    item->setTextAlignment(Qt::AlignCenter);
    item->setForeground(QColor(colors::text));
    item->setBackground(QColor(header ? colors::header : colors::surface));
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    grid.setItem(row, col, item);
    return item;
}
class TransportationInputDelegate final : public SpreadsheetDelegate {
public:
    TransportationInputDelegate(TransportationController& controller, QTableWidget* grid)
        : SpreadsheetDelegate(grid), controller_(controller), grid_(grid) {}
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        auto* editor = SpreadsheetDelegate::createEditor(parent, option, index);
        editor_ = editor; index_ = index; return editor;
    }
    void destroyEditor(QWidget* editor, const QModelIndex& index) const override {
        if (editor_ == editor) editor_.clear();
        SpreadsheetDelegate::destroyEditor(editor, index);
    }
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override {
        if (committing_) return;
        const QScopedValueRollback<bool> committing(committing_, true);
        auto* line = qobject_cast<QLineEdit*>(editor); if (!line) return;
        valid_ = writeCell(model, index, line->text());
    }
    bool eventFilter(QObject* object, QEvent* event) override {
        if (committing_) return true;
        return SpreadsheetDelegate::eventFilter(object, event);
    }
    bool writeCell(QAbstractItemModel* model, const QModelIndex& index, const QVariant& value) const override {
        QString error;
        if (!controller_.editCell(index.row()-1, index.column()-1, value.toString(), error)) {
            controller_.dialogs().showError(QStringLiteral("Input Data Error"), error);
            return false;
        }
        const QSignalBlocker block(grid_);
        return model->setData(index, text(controller_.document()->cell(static_cast<std::size_t>(index.row()-1),
                                                                      static_cast<std::size_t>(index.column()-1))), Qt::EditRole);
    }
    bool commitEditor(QWidget* editor, const QModelIndex& index) const override {
        setModelData(editor, grid_->model(), index);
        return valid_;
    }
    bool hasEditor() const { return !editor_.isNull(); }
    bool commitActiveEditor() {
        if (!editor_) return true;
        auto* editor = editor_.data(); setModelData(editor, grid_->model(), index_);
        if (!valid_) return false;
        emit closeEditor(editor, QAbstractItemDelegate::NoHint); editor_.clear(); return true;
    }
private:
    TransportationController& controller_;
    QTableWidget* grid_;
    mutable QPointer<QWidget> editor_;
    mutable QPersistentModelIndex index_;
    mutable bool valid_ = true, committing_ = false;
};
QWidget* inputMode(ApplicationShell& shell)
{
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);
    auto* panel = new QWidget;
    auto* choices = new QVBoxLayout(panel);
    auto* title = new QLabel(QStringLiteral("TRANSPORTATION MODEL"));
    title->setAlignment(Qt::AlignCenter);
    choices->addWidget(title);
    auto* description = new QLabel(QStringLiteral("Create or edit a Transportation problem, or load the example.\nNorth-west / Least-cost / Vogel starts · automatic iterations"));
    description->setAlignment(Qt::AlignCenter);
    choices->addWidget(description);
    auto* decimal=new QRadioButton(QStringLiteral("Decimal notation"));decimal->setObjectName(QStringLiteral("transport-decimal"));decimal->setChecked(true);
    auto* scientific=new QRadioButton(QStringLiteral("Scientific notation"));scientific->setObjectName(QStringLiteral("transport-scientific"));
    choices->addWidget(decimal);choices->addWidget(scientific);
    auto* formatFields=new QFormLayout;
    auto* n=new QLineEdit(QStringLiteral("5"));n->setObjectName(QStringLiteral("transport-format-n"));n->setValidator(new QIntValidator(0,core::NumberFormatter::MaximumDigits,n));
    auto* d=new QLineEdit(QStringLiteral("2"));d->setObjectName(QStringLiteral("transport-format-d"));d->setValidator(new QIntValidator(0,core::NumberFormatter::MaximumDigits,d));
    n->setToolTip(QStringLiteral("Decimal field width, or scientific mantissa decimal places."));
    d->setToolTip(QStringLiteral("Decimal places, or scientific exponent width."));
    formatFields->addRow(QStringLiteral("N:"),n);formatFields->addRow(QStringLiteral("D:"),d);choices->addLayout(formatFields);
    button(QStringLiteral("Enter New Problem"), QStringLiteral("transport-new"), *choices, [&shell,n,d,scientific] {
        bool validN=false,validD=false;const auto nn=n->text().toInt(&validN),dd=d->text().toInt(&validD);
        if(!validN || !validD){transportationController(shell).dialogs().showError(QStringLiteral("Number Format"),QStringLiteral("Enter whole numbers for N and D."));return;}
        newProblem(shell,{scientific->isChecked()?core::Notation::Scientific:core::Notation::Decimal,nn,dd});
    });
    button(QStringLiteral("Enter/Edit Existing Problem"), QStringLiteral("transport-open"), *choices, [&shell] { openProblem(shell); });
    button(QStringLiteral("Load Example"), QStringLiteral("transport-example"), *choices, [&shell] {
        if (!transportationController(shell).leaveInput()) return;
        transportationController(shell).loadExample();
        shell.navigate(id("input"));
    });
    centerPanel(*layout, panel);
    footer(*layout, shell, false);
    return page;
}
QWidget* input(ApplicationShell& shell)
{
    auto& controller = transportationController(shell);
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);
    auto* form = new QFormLayout;
    auto* title = new QLineEdit; title->setObjectName(QStringLiteral("transport-title"));
    form->addRow(QStringLiteral("Problem Title:"), title);
    auto* dimensions = new QHBoxLayout;
    auto* sources = new QLineEdit; sources->setObjectName(QStringLiteral("transport-sources"));
    auto* destinations = new QLineEdit; destinations->setObjectName(QStringLiteral("transport-destinations"));
    for (auto* field : {sources, destinations}) {
        field->setValidator(new QIntValidator(1, 100, field)); field->setMaximumWidth(field->fontMetrics().horizontalAdvance(QStringLiteral("000000")));
    }
    dimensions->addWidget(new QLabel(QStringLiteral("No. of Sources:"))); dimensions->addWidget(sources);
    dimensions->addWidget(new QLabel(QStringLiteral("No. of Dest'ns:"))); dimensions->addWidget(destinations);
    auto* initialize = new QPushButton(QStringLiteral("Initialize Grid")); initialize->setObjectName(QStringLiteral("transport-initialize"));
    dimensions->addWidget(initialize); dimensions->addStretch();
    layout->addLayout(form); layout->addLayout(dimensions);
    auto* status = new QLabel; status->setObjectName(QStringLiteral("transport-input-status")); status->setWordWrap(true); layout->addWidget(status);
    auto* grid = table(page, "transport-input-grid");
    auto* delegate = new TransportationInputDelegate(controller, grid); grid->setItemDelegate(delegate);
    grid->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed | QAbstractItemView::AnyKeyPressed);
    layout->addWidget(grid, 1);
    auto* row = new QHBoxLayout;
    addButton(*row, QStringLiteral("New Problem"), "transport-input-new", page, [&shell] { newProblem(shell); });
    addButton(*row, QStringLiteral("Open Problem"), "transport-input-open", page, [&shell] { openProblem(shell); });
    auto* save = addButton(*row, QStringLiteral("Save As"), "transport-input-save", page, [&controller] { controller.requestSave(); });
    row->addStretch();
    auto* solve = addButton(*row, QStringLiteral("SOLVE Menu"), "transport-solve-menu", page, [&shell] { shell.navigate(id("solve-menu")); });
    layout->addLayout(row);
    const auto refresh = [&controller, title, sources, destinations, initialize, status, grid, solve, save] {
        const QSignalBlocker block(grid);
        grid->clear(); solve->setEnabled(controller.loaded()); save->setEnabled(controller.loaded());
        title->setReadOnly(controller.readOnly()); sources->setReadOnly(controller.readOnly()); destinations->setReadOnly(controller.readOnly());
        initialize->setEnabled(!controller.readOnly());
        if (!controller.loaded()) {
            title->clear(); sources->clear(); destinations->clear();
            status->setText(QStringLiteral("Enter source and destination counts, then press Return or Initialize Grid."));
            grid->setRowCount(0); grid->setColumnCount(0); return;
        }
        const auto& p = controller.problem(); const auto& doc = *controller.document();
        const int m = static_cast<int>(p.supply.size()), n = static_cast<int>(p.demand.size());
        title->setText(text(p.title)); sources->setText(QString::number(m)); destinations->setText(QString::number(n));
        status->setText((controller.readOnly() ? QStringLiteral("Read-only") : controller.dirty() ? QStringLiteral("Unsaved changes") : QStringLiteral("Input data"))
                       + (controller.sourcePath().isEmpty() ? QString() : QStringLiteral(" · ") + controller.sourcePath()));
        grid->setRowCount(m+3); grid->setColumnCount(n+3);
        put(*grid, 1, 1, QStringLiteral("S/D Name"), true); put(*grid, 0, n+2, QStringLiteral("Supply"), true);
        for (int j = 0; j < n; ++j) put(*grid, 0, j+2, QStringLiteral("D%1").arg(j+1), true);
        for (int i = 0; i < m; ++i) put(*grid, i+2, 0, QStringLiteral("S%1").arg(i+1), true);
        put(*grid, m+2, 0, QStringLiteral("Demand"), true);
        for (int r = 0; r < m+2; ++r) for (int c = 0; c < n+2; ++c) {
            const bool alias = (r == 0 && c >= 1 && c <= n) || (c == 0 && r >= 1 && r <= m);
            const bool cost = r >= 1 && r <= m && c >= 1 && c <= n;
            const bool quantity = (r >= 1 && r <= m && c == n+1) || (r == m+1 && c >= 1 && c <= n);
            if (!(alias || cost || quantity)) continue;
            auto* item = put(*grid, r+1, c+1, text(doc.cell(static_cast<std::size_t>(r), static_cast<std::size_t>(c))), quantity);
            if (!controller.readOnly()) item->setFlags(item->flags() | Qt::ItemIsEditable);
        }
        grid->fitColumns(2, n);
    };
    // Cell commits update only their item. Rebuilding the whole grid here would
    // destroy Qt's next editor after Return/Tab and lose keyboard continuity.
    QObject::connect(&controller, &TransportationController::inputReset, page, refresh);
    QObject::connect(&controller, &TransportationController::cellChanged, page, [&controller, grid, status](int r, int c) {
        const QSignalBlocker block(grid);
        if (auto* item = grid->item(r+1, c+1)) item->setText(text(controller.document()->cell(static_cast<std::size_t>(r), static_cast<std::size_t>(c))));
        status->setText(QStringLiteral("Unsaved changes") + (controller.sourcePath().isEmpty() ? QString() : QStringLiteral(" · ") + controller.sourcePath()));
    });
    const auto updateTitle = [&controller, title] {
        QString error;
        if (!controller.setTitle(title->text(), error)) {
            controller.dialogs().showError(QStringLiteral("Input Data Error"), error);
            if (controller.loaded()) title->setText(text(controller.problem().title));
        }
    };
    QObject::connect(title, &QLineEdit::editingFinished, page, updateTitle);
    QObject::connect(&controller, &TransportationController::commitRequested, page, [delegate, updateTitle] {
        if (delegate->commitActiveEditor()) updateTitle();
    });
    const auto initializeGrid = [&controller, title, sources, destinations] {
        bool validSources = false, validDestinations = false;
        const auto m = sources->text().toInt(&validSources), n = destinations->text().toInt(&validDestinations);
        QString error;
        if (!controller.commitInput()) return;
        if (!controller.initialize(title->text(), validSources ? m : 0, validDestinations ? n : 0, error))
            controller.dialogs().showError(QStringLiteral("Problem Size"), error);
    };
    QObject::connect(initialize, &QPushButton::clicked, page, initializeGrid);
    QObject::connect(sources, &QLineEdit::returnPressed, page, initializeGrid);
    QObject::connect(destinations, &QLineEdit::returnPressed, page, initializeGrid);
    const auto selectHeading = [&controller](int r, int c, bool before) {
        const auto m = static_cast<int>(controller.problem().supply.size());
        const auto n = static_cast<int>(controller.problem().demand.size());
        if (c == 0 && r >= 2 && r < m + 2) controller.selectLine(TransportationController::Axis::Source, r - 2, before);
        else if (r == 0 && c >= 2 && c < n + 2) controller.selectLine(TransportationController::Axis::Destination, c - 2, before);
        else controller.clearSelection();
    };
    QObject::connect(grid, &QTableWidget::cellClicked, page, [selectHeading](int r, int c) { selectHeading(r, c, false); });
    QObject::connect(grid, &QTableWidget::cellDoubleClicked, page, [selectHeading](int r, int c) { selectHeading(r, c, true); });
    QObject::connect(&controller, &TransportationController::selectionChanged, grid, [&controller, grid] {
        const QSignalBlocker block(grid);
        if (!controller.hasSelection()) grid->clearSelection();
        else if (*controller.selectionAxis() == TransportationController::Axis::Source) grid->selectRow(controller.selectionIndex() + 2);
        else grid->selectColumn(controller.selectionIndex() + 2);
    });
    auto* editHint = new QLabel(QStringLiteral("EditGrid: click a source/destination heading, then choose Copy, Paste, Delete or Insert.\nSingle click inserts after the heading; double click inserts before it."));
    editHint->setWordWrap(true); layout->insertWidget(3, editHint);
    refresh(); footer(*layout, shell, false); return page;
}
QWidget* solveMenu(ApplicationShell& shell)
{
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);
    auto* panel = new QWidget;
    auto* choices = new QVBoxLayout(panel);
    auto* heading = new QLabel(QStringLiteral("Solve Problem\nIterations"));
    heading->setAlignment(Qt::AlignCenter); choices->addWidget(heading);
    button(QStringLiteral("North-west corner start"), QStringLiteral("transport-northwest-start"), *choices,
           [&shell] { solveProblem(shell); });
    button(QStringLiteral("Least-cost start"), QStringLiteral("transport-leastcost-start"), *choices,
           [&shell] { solveProblem(shell,transportation::StartMethod::LeastCost); });
    button(QStringLiteral("Vogel start"), QStringLiteral("transport-vogel-start"), *choices,
           [&shell] { solveProblem(shell,transportation::StartMethod::Vogel); });
    button(QStringLiteral("Final solution"),QStringLiteral("transport-final-start"),*choices,[&shell]{finalProblem(shell);});
    centerPanel(*layout, panel);
    footer(*layout, shell, true);
    return page;
}
QWidget* finalScreen(ApplicationShell& shell) {
    auto& controller=transportationController(shell);auto* page=new QWidget;auto* layout=new QVBoxLayout(page);
    auto* heading=new QLabel(QStringLiteral("TRANSPORTATION MODEL OUTPUT SUMMARY"));heading->setAlignment(Qt::AlignCenter);layout->addWidget(heading);
    const auto label=[layout](const char* name) {auto* value=new QLabel;value->setObjectName(QString::fromLatin1(name));value->setWordWrap(true);layout->addWidget(value);return value;};
    auto* title=label("transport-final-title"),*iteration=label("transport-final-iteration"),*objective=label("transport-final-objective");
    auto* controls=new QHBoxLayout;
    for(const auto& command:{std::pair<const char*,const char*>{"Next Iteration","transport-final-next"},{"All Iterations","transport-final-all"}}) {
        auto* b=new QPushButton(QString::fromLatin1(command.first));b->setObjectName(QString::fromLatin1(command.second));b->setEnabled(false);controls->addWidget(b);
    }
    auto* print=new QPushButton(QStringLiteral("Write to Printer"));print->setObjectName(QStringLiteral("transport-final-print"));controls->addWidget(print);
    QObject::connect(print,&QPushButton::clicked,page,[&controller,&shell] {
        if(requestTransportationFinalPrint(&shell,controller)==PrintOutcome::Failed)
            controller.dialogs().showError(QStringLiteral("Printer Error"),QStringLiteral("The Transportation print job could not be completed."));
    });
    controls->addStretch();layout->addLayout(controls);
    auto* grid=table(page,"transport-final-grid");layout->addWidget(grid,1);
    const auto render=[&controller,title,iteration,objective,grid,print] {
        const auto* output=controller.finalOutput();grid->clear();grid->setRowCount(0);grid->setColumnCount(5);
        print->setEnabled(output!=nullptr);
        title->setText(output?QStringLiteral("Title: ")+text(output->title):QStringLiteral("Open or initialize input, then choose Final solution from the SOLVE Menu."));
        iteration->setText(output?QStringLiteral("Final Iteration No.: ")+text(output->iteration):QString());
        objective->setText(output?(output->maximize?QStringLiteral("Objective Value (maximum profit) ="):QStringLiteral("Objective Value (minimum cost) ="))+text(output->objective):QString());
        if(!output)return;
        grid->setRowCount(static_cast<int>(output->rows.size())+1);
        const QStringList headers{QStringLiteral("From"),QStringLiteral("To"),QStringLiteral("Amt Shipped"),QStringLiteral("Obj Coeff"),QStringLiteral("Obj Contrib")};
        for(int col=0;col<5;++col)put(*grid,0,col,headers[col],true);
        for(std::size_t row=0;row<output->rows.size();++row)for(int col=0;col<5;++col)
            put(*grid,static_cast<int>(row)+1,col,text(output->rows[row][static_cast<std::size_t>(col)]));
        grid->fitColumns(0,5);
    };
    QObject::connect(&controller,&TransportationController::finalChanged,page,render);render();footer(*layout,shell,true);return page;
}
QWidget* iterations(ApplicationShell& shell)
{
    auto& controller = transportationController(shell);
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);
    auto* heading = new QLabel;
    heading->setObjectName(QStringLiteral("transport-method-heading"));
    heading->setWordWrap(true); layout->addWidget(heading);
    auto* title = new QLabel;
    title->setWordWrap(true); layout->addWidget(title);
    auto* balance = new QLabel(text(controller.balancing()));
    balance->setObjectName(QStringLiteral("transport-balancing"));balance->setWordWrap(true);layout->addWidget(balance);
    auto* tabs=new QTabWidget;tabs->setObjectName(QStringLiteral("transport-phases"));layout->addWidget(tabs,1);
    auto* modi=new QWidget;auto* modiLayout=new QVBoxLayout(modi);tabs->addTab(modi,QStringLiteral("MODI iterations"));
    auto* construction=new QWidget;auto* constructionLayout=new QVBoxLayout(construction);
    tabs->addTab(construction,QStringLiteral("Initial allocation steps"));
    auto* allocationControls=new QHBoxLayout;
    auto* previousAllocation=new QPushButton(QStringLiteral("Previous allocation"));previousAllocation->setObjectName(QStringLiteral("transport-allocation-previous"));
    auto* nextAllocation=new QPushButton(QStringLiteral("Next allocation"));nextAllocation->setObjectName(QStringLiteral("transport-allocation-next"));
    auto* allAllocations=new QPushButton(QStringLiteral("All allocations"));allAllocations->setObjectName(QStringLiteral("transport-allocation-all"));
    for(auto* b:{previousAllocation,nextAllocation,allAllocations})allocationControls->addWidget(b);
    allocationControls->addStretch();constructionLayout->addLayout(allocationControls);
    auto* allocationExplanation=new QLabel;allocationExplanation->setObjectName(QStringLiteral("transport-allocation-explanation"));
    allocationExplanation->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);allocationExplanation->setWordWrap(true);
    allocationExplanation->setAlignment(Qt::AlignLeft|Qt::AlignTop);constructionLayout->addWidget(allocationExplanation);
    auto* allocationGrid=table(construction,"transport-allocation-grid");constructionLayout->addWidget(allocationGrid,1);
    auto allocationShown=std::make_shared<std::size_t>(1);
    const auto renderAllocation=[&controller,allocationGrid,allocationExplanation,allocationShown,previousAllocation,nextAllocation,allAllocations,balance] {
        balance->setText(text(controller.balancing()));balance->setVisible(!controller.balancing().empty());
        const auto& p=controller.solutionProblem();const auto& steps=controller.allocations();
        const auto count=std::min(*allocationShown,steps.size());
        const int m=static_cast<int>(p.supply.size()),n=static_cast<int>(p.demand.size());
        allocationGrid->clear();allocationGrid->setRowCount(m+3);allocationGrid->setColumnCount(n+3);
        previousAllocation->setEnabled(count>1);nextAllocation->setEnabled(count<steps.size());allAllocations->setEnabled(count<steps.size());
        if(!count){allocationExplanation->clear();return;}
        const auto& step=steps[count-1];
        QString details=QStringLiteral("Allocation %1 of %2: S%3 → D%4; min(%5, %6) = %7. Unit cost/profit = %8; partial objective = %9.\n%10")
            .arg(count).arg(steps.size()).arg(step.cell.source+1).arg(step.cell.destination+1)
            .arg(text(transportation::quantity(step.remainingSupply[step.cell.source]+step.amount)),
                 text(transportation::quantity(step.remainingDemand[step.cell.destination]+step.amount)),
                 text(transportation::quantity(step.amount)),text(transportation::quantity(p.costs[step.cell.source][step.cell.destination])),
                 text(transportation::quantity(step.objective)),text(step.explanation));
        if(p.maximize && controller.method()==transportation::StartMethod::Vogel)
            details+=QStringLiteral(" Penalties use negated profits to select the greatest opportunity loss.");
        if(count==steps.size())details+=QStringLiteral(" Initial basis complete; continue in MODI iterations.");
        allocationExplanation->setText(details);
        put(*allocationGrid,0,0,QStringLiteral("Basic shipments"),true);
        put(*allocationGrid,0,n+1,QStringLiteral("Remaining supply"),true);
        put(*allocationGrid,0,n+2,QStringLiteral("Row penalty"),true);
        for(int j=0;j<n;++j)put(*allocationGrid,0,j+1,QStringLiteral("D%1 %2").arg(j+1).arg(text(p.destinationNames[std::size_t(j)])),true);
        for(int i=0;i<m;++i) {
            put(*allocationGrid,i+1,0,QStringLiteral("S%1 %2").arg(i+1).arg(text(p.sourceNames[std::size_t(i)])),true);
            put(*allocationGrid,i+1,n+1,text(transportation::quantity(step.remainingSupply[std::size_t(i)])));
            if(!step.sourcePenalties.empty() && step.sourcePenalties[std::size_t(i)])
                put(*allocationGrid,i+1,n+2,text(transportation::quantity(*step.sourcePenalties[std::size_t(i)])));
        }
        put(*allocationGrid,m+1,0,QStringLiteral("Remaining demand"),true);
        put(*allocationGrid,m+2,0,QStringLiteral("Column penalty"),true);
        for(int j=0;j<n;++j) {
            put(*allocationGrid,m+1,j+1,text(transportation::quantity(step.remainingDemand[std::size_t(j)])));
            if(!step.destinationPenalties.empty() && step.destinationPenalties[std::size_t(j)])
                put(*allocationGrid,m+2,j+1,text(transportation::quantity(*step.destinationPenalties[std::size_t(j)])));
        }
        for(std::size_t k=0;k<count;++k) {
            const auto& allocation=steps[k];
            auto* item=put(*allocationGrid,int(allocation.cell.source)+1,int(allocation.cell.destination)+1,
                allocation.amount==0?QStringLiteral("0 (basic)"):text(transportation::quantity(allocation.amount)));
            item->setToolTip(QStringLiteral("Unit cost/profit: ")+text(transportation::quantity(p.costs[allocation.cell.source][allocation.cell.destination])));
            if(k+1==count)item->setBackground(QColor(colors::entering));
        }
        allocationGrid->fitColumns(1,n);
    };
    QObject::connect(previousAllocation,&QPushButton::clicked,page,[allocationShown,renderAllocation]{if(*allocationShown>1)--*allocationShown;renderAllocation();});
    QObject::connect(nextAllocation,&QPushButton::clicked,page,[&controller,allocationShown,renderAllocation]{if(*allocationShown<controller.allocations().size())++*allocationShown;renderAllocation();});
    QObject::connect(allAllocations,&QPushButton::clicked,page,[&controller,allocationShown,renderAllocation]{*allocationShown=controller.allocations().size();renderAllocation();});
    QObject::connect(&controller,&TransportationController::changed,page,[allocationShown,renderAllocation]{*allocationShown=1;renderAllocation();});
    renderAllocation();
    auto* controls = new QHBoxLayout;
    auto* next = new QPushButton(QStringLiteral("Next Iteration"));
    next->setObjectName(QStringLiteral("transport-next")); controls->addWidget(next);
    auto* all = new QPushButton(QStringLiteral("All Iterations"));
    all->setObjectName(QStringLiteral("transport-all")); controls->addWidget(all);
    auto* print=new QPushButton(QStringLiteral("Write to Printer"));print->setObjectName(QStringLiteral("transport-print"));controls->addWidget(print);
    controls->addStretch();
    controls->addWidget(new QLabel(QStringLiteral("Initialize u or v:")));
    auto* anchor=new QComboBox;anchor->setObjectName(QStringLiteral("transport-multiplier-anchor"));controls->addWidget(anchor);
    modiLayout->addLayout(controls);
    auto* grid = table(page, "transport-tableau"); modiLayout->addWidget(grid, 1);
    auto* hint=new QLabel(QStringLiteral("Click shipment cells to check the improvement loop. Next or All can be used without selecting cells."));
    hint->setWordWrap(true);modiLayout->insertWidget(1,hint);
    auto* iterationExplanation=new QLabel;iterationExplanation->setObjectName(QStringLiteral("transport-iteration-explanation"));
    iterationExplanation->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);iterationExplanation->setWordWrap(true);
    iterationExplanation->setAlignment(Qt::AlignLeft|Qt::AlignTop);modiLayout->insertWidget(2,iterationExplanation);
    auto shown = std::make_shared<std::size_t>(1);
    auto attempt=std::make_shared<std::set<std::pair<std::size_t,std::size_t>>>();
    const auto mark=[](QTableWidgetItem* item,transportation::LoopRole role) {
        if(role==transportation::LoopRole::Entering)item->setBackground(QColor(colors::entering));
        else if(role==transportation::LoopRole::Leaving)item->setBackground(QColor(colors::leaving));
        else if(role==transportation::LoopRole::Other)item->setBackground(QColor(colors::header));
    };
    const auto render = [&controller, grid, next, all, shown, title, heading, anchor, attempt, mark, print, iterationExplanation] {
        heading->setText(controller.method()==transportation::StartMethod::Vogel?
                         QStringLiteral("TRANSPORTATION TABLEAU - (Vogel's Method)"):
                         controller.method()==transportation::StartMethod::LeastCost?
                         QStringLiteral("TRANSPORTATION TABLEAU - (Least Cost Method)"):
                         QStringLiteral("TRANSPORTATION TABLEAU - (North-West Corner Method)"));
        grid->clearSpans(); grid->clear();
        const auto& history=controller.trace();
        const auto& p=controller.solutionProblem();
        title->setText(QStringLiteral("Title: %1 (%2)").arg(text(p.title),p.maximize?QStringLiteral("maximum profit"):QStringLiteral("minimum cost")));
        const int m=static_cast<int>(p.supply.size()), n=static_cast<int>(p.demand.size()), height=3*m+4;
        {
            const QSignalBlocker blocker(anchor);anchor->clear();
            for(int i=0;i<m;++i)anchor->addItem(QStringLiteral("u%1=0").arg(i+1));
            for(int j=0;j<n;++j)anchor->addItem(QStringLiteral("v%1=0").arg(j+1));
            const auto selected=controller.multiplierAnchor();
            anchor->setCurrentIndex(static_cast<int>(selected.index)+(selected.axis==transportation::MultiplierAxis::Destination?m:0));
            anchor->setEnabled(controller.canInitializeMultiplier());
        }
        const auto count=std::min(*shown,history.size());
        QString explanation;
        if(count) {
            const auto& current=history[count-1];std::size_t basic=0,positive=0;
            for(const auto& row:current.shipments)for(const auto& value:row)if(value){++basic;if(*value>0)++positive;}
            explanation=QStringLiteral("%1 basis: %2 positive shipments, %3 basic cells (m+n−1 = %4).")
                .arg(positive<basic?QStringLiteral("Degenerate"):QStringLiteral("Nondegenerate")).arg(positive).arg(basic).arg(m+n-1);
            if(positive<basic)explanation+=QStringLiteral(" Zero basic cells keep the basis connected and carry no physical flow.");
            if(current.nextPivot) {
                const auto& pivot=*current.nextPivot;const auto theta=*current.shipments[pivot.leaving.source][pivot.leaving.destination];
                explanation+=QStringLiteral(" Next pivot: S%1D%2 enters, S%3D%4 leaves; θ = %5%6.")
                    .arg(pivot.entering.source+1).arg(pivot.entering.destination+1).arg(pivot.leaving.source+1).arg(pivot.leaving.destination+1)
                    .arg(text(transportation::quantity(theta)),theta==0?QStringLiteral(" (degenerate pivot)"):QString());
                explanation+=QStringLiteral(" Loop:");
                for(std::size_t k=0;k<pivot.loop.size();++k)explanation+=QStringLiteral(" %1S%2D%3")
                    .arg(k%2?QStringLiteral("−"):QStringLiteral("+")).arg(pivot.loop[k].source+1).arg(pivot.loop[k].destination+1);
            } else explanation+=QStringLiteral(" Optimal: no improving nonbasic reduced cost remains.");
        }
        iterationExplanation->setText(explanation);
        print->setEnabled(count>0);
        grid->setRowCount(static_cast<int>(count)*height); grid->setColumnCount(n+4);
        next->setEnabled(count<history.size()); all->setEnabled(count<history.size());
        for (std::size_t k=0; k<count; ++k) {
            const int base=static_cast<int>(k)*height;
            const auto d=transportation::display(p,history[k],k+1,k==0 && controller.multipliersRefreshed());
            std::size_t positive=0,basic=0;
            for(const auto& row:history[k].shipments)for(const auto& value:row)if(value){++basic;if(*value>0)++positive;}
            auto* iterationItem=put(*grid, base, 0, QStringLiteral("Iter %1").arg(k+1), true);
            iterationItem->setToolTip(QStringLiteral("%1 basic cells; %2 positive shipments. %3")
                .arg(basic).arg(positive).arg(positive<basic?QStringLiteral("Degenerate basis: zero basic cells maintain m+n−1 independent cells."):QStringLiteral("Nondegenerate basis.")));
            put(*grid, base, 1, QStringLiteral("ObjVal ="), true);
            put(*grid, base, 2, text(d.objective), true);
            put(*grid, base+1, 1, QStringLiteral("Name"), true);
            put(*grid, base+1, n+3, QStringLiteral("Supply"), true);
            for (int j=0; j<n; ++j) {
                put(*grid, base, j+3, QStringLiteral("D%1").arg(j+1), true);
                put(*grid, base+1, j+3, text(p.destinationNames[static_cast<std::size_t>(j)]), true);
                auto* multiplier=put(*grid, base+2, j+3, text(d.v[static_cast<std::size_t>(j)]), true);
                if(controller.multiplierAnchor().axis==transportation::MultiplierAxis::Destination && controller.multiplierAnchor().index==static_cast<std::size_t>(j))
                    multiplier->setForeground(QColor(Qt::darkRed));
                put(*grid, base+height-1, j+3, text(d.demand[static_cast<std::size_t>(j)]), true);
            }
            put(*grid, base+height-1, 1, QStringLiteral("Demand"), true);
            for (int i=0; i<m; ++i) {
                const int row=base+3+3*i; const auto r=static_cast<std::size_t>(i);
                put(*grid, row, 0, QStringLiteral("S%1").arg(i+1), true); grid->setSpan(row,0,3,1);
                put(*grid, row, 1, text(p.sourceNames[r])); grid->setSpan(row,1,3,1);
                auto* multiplier=put(*grid, row, 2, text(d.u[r]), true); grid->setSpan(row,2,3,1);
                if(controller.multiplierAnchor().axis==transportation::MultiplierAxis::Source && controller.multiplierAnchor().index==r)
                    multiplier->setForeground(QColor(Qt::darkRed));
                put(*grid, row, n+3, text(d.supply[r]), true); grid->setSpan(row,n+3,3,1);
                for (int j=0; j<n; ++j) {
                    const auto c=static_cast<std::size_t>(j);
                    put(*grid,row,j+3,text(d.costs[r][c]));
                    auto* shipment=put(*grid,row+1,j+3,text(d.shipments[r][c]));
                    QFont large=grid->font(); large.setPointSizeF(large.pointSizeF()*1.2); large.setBold(true); shipment->setFont(large);
                    put(*grid,row+2,j+3,text(d.reducedCosts[r][c]));
                    if(k+1<count || attempt->count({r,c}))mark(shipment,transportation::loopRole(history[k],{r,c}));
                }
            }
        }
        grid->fitColumns(3,n);
    };
    QObject::connect(&controller, &TransportationController::changed, page, [shown,attempt,render] { *shown=1;attempt->clear();render(); });
    QObject::connect(&controller, &TransportationController::multipliersChanged, page, render);
    QObject::connect(anchor,qOverload<int>(&QComboBox::activated),page,[&controller,render](int index) {
        const auto m=static_cast<int>(controller.solutionProblem().supply.size());QString error;
        const transportation::MultiplierAnchor selected{index<m?transportation::MultiplierAxis::Source:transportation::MultiplierAxis::Destination,
            static_cast<std::size_t>(index<m?index:index-m)};
        if(!controller.initializeMultiplier(selected,error)) {
            controller.dialogs().showError(QStringLiteral("Initialize u or v"),error);render();
        }
    });
    QObject::connect(grid,&QTableWidget::cellClicked,page,[&controller,grid,shown,attempt,mark](int row,int col) {
        const auto count=std::min(*shown,controller.trace().size());if(!count)return;
        const auto& p=controller.solutionProblem();const int m=static_cast<int>(p.supply.size()),n=static_cast<int>(p.demand.size());
        const int local=row-static_cast<int>(count-1)*(3*m+4);
        if(local<4 || (local-4)%3 || (local-4)/3>=m || col<3 || col>=n+3)return;
        const auto source=static_cast<std::size_t>((local-4)/3),destination=static_cast<std::size_t>(col-3);
        const auto role=transportation::loopRole(controller.trace()[count-1],{source,destination});
        if(role==transportation::LoopRole::None)return;
        attempt->emplace(source,destination);mark(grid->item(row,col),role);grid->clearSelection();
    });
    const auto advance = [&controller, &shell, shown, attempt, render, grid](bool complete) {
        const auto total=controller.trace().size();
        if (*shown>=total) return;
        controller.lockMultipliers();attempt->clear();*shown=complete ? total : *shown+1; render();
        if (grid->rowCount()) grid->scrollToItem(grid->item(grid->rowCount()-1,1));
        if (*shown==total) QMessageBox::information(&shell,QStringLiteral("Optimal Tableau"),
                                                   QStringLiteral("Iteration %1 is optimal").arg(total));
    };
    QObject::connect(next,&QPushButton::clicked,page,[advance]{advance(false);});
    QObject::connect(all,&QPushButton::clicked,page,[advance]{advance(true);});
    QObject::connect(print,&QPushButton::clicked,page,[&controller,&shell,shown] {
        const auto outcome=requestLpPrint(&shell,[&](QPrinter& printer){return printTransportationTrace(printer,controller,*shown);});
        if(outcome==PrintOutcome::Failed)controller.dialogs().showError(QStringLiteral("Printer Error"),QStringLiteral("The Transportation print job could not be completed."));
    });
    render(); footer(*layout,shell,true);
    return page;
}
void moduleMenus(ApplicationShell& shell, QMenuBar& bar, bool inputPage)
{
    auto* file=bar.addMenu(QStringLiteral("File"));
    auto* create=file->addAction(QStringLiteral("New"),&shell,[&shell]{newProblem(shell);});
    create->setShortcut(QKeySequence::New);
    auto* open=file->addAction(QStringLiteral("Open…"),&shell,[&shell]{openProblem(shell);});
    open->setShortcut(QKeySequence::Open);
    auto& controller=transportationController(shell);
    auto* save=file->addAction(QStringLiteral("Save"),&controller,[&controller]{controller.requestSave();});
    save->setShortcut(QKeySequence::Save);
    auto* saveAs=file->addAction(QStringLiteral("Save As…"),&controller,[&controller]{controller.requestSave();});
    saveAs->setShortcut(QKeySequence::SaveAs);
    if (inputPage) {
        auto* edit = bar.addMenu(QStringLiteral("EditGrid"));
        const QStringList labels{QStringLiteral("Undo Last Insert, Paste, Or Delete"), QStringLiteral("Copy Column or Row"),
            QStringLiteral("Paste on Column or Row"), QStringLiteral("Delete Column or Row"), QStringLiteral("Insert Column or Row")};
        const QStringList names{QStringLiteral("undo"), QStringLiteral("copy"), QStringLiteral("paste"), QStringLiteral("delete"), QStringLiteral("insert")};
        const QStringList shortcuts{QStringLiteral("Ctrl+U"), QStringLiteral("Ctrl+C"), QStringLiteral("Ctrl+P"), QStringLiteral("Ctrl+D"), QStringLiteral("Ctrl+I")};
        QList<QAction*> actions;
        for (int i = 0; i < labels.size(); ++i) {
            auto* action = edit->addAction(labels[i]);
            action->setObjectName(QStringLiteral("transport-grid-") + names[i]); action->setShortcut(QKeySequence(shortcuts[i])); actions.append(action);
        }
        QObject::connect(actions[1], &QAction::triggered, &controller, [&controller] { controller.copySelection(); });
        const auto change = [&controller](int operation) {
            QString error;
            const bool ok = operation == 0 ? controller.undo(error) : operation == 2 ? controller.pasteSelection(error)
                : operation == 3 ? controller.deleteSelection(error) : controller.insertSelection(error);
            if (!ok && !error.isEmpty()) controller.dialogs().showError(QStringLiteral("EditGrid"), error);
        };
        for (const int i : {0, 2, 3, 4}) QObject::connect(actions[i], &QAction::triggered, &controller, [change, i] { change(i); });
        const auto updateEdit = [&controller, actions] {
            actions[0]->setEnabled(controller.canUndo()); actions[1]->setEnabled(controller.canCopy()); actions[2]->setEnabled(controller.canPaste());
            actions[3]->setEnabled(controller.hasSelection() && !controller.readOnly()); actions[4]->setEnabled(controller.hasSelection() && !controller.readOnly());
        };
        QObject::connect(&controller, &TransportationController::changed, edit, updateEdit);
        QObject::connect(&controller, &TransportationController::selectionChanged, edit, updateEdit); updateEdit();
    }
    auto* solve=bar.addMenu(QStringLiteral("Solve Problem"));
    auto* northwest=solve->addAction(QStringLiteral("Iterations (North-west corner start)"),&shell,[&shell]{solveProblem(shell);});
    northwest->setEnabled(transportationController(shell).loaded());
    auto* leastcost=solve->addAction(QStringLiteral("Iterations (Least-cost start)"),&shell,[&shell]{solveProblem(shell,transportation::StartMethod::LeastCost);});
    leastcost->setEnabled(transportationController(shell).loaded());
    auto* vogel=solve->addAction(QStringLiteral("Iterations (Vogel start)"),&shell,[&shell]{solveProblem(shell,transportation::StartMethod::Vogel);});
    vogel->setEnabled(transportationController(shell).loaded());
    auto* final=solve->addAction(QStringLiteral("Final solution"),&shell,[&shell]{finalProblem(shell);});final->setObjectName(QStringLiteral("transport-final-action"));
    const auto update=[&controller,save,saveAs,northwest,leastcost,vogel,final] {
        for (auto* action:{save,saveAs,northwest,leastcost,vogel,final}) action->setEnabled(controller.loaded());
    };
    QObject::connect(&controller,&TransportationController::changed,file,update); update();
    bar.addAction(QStringLiteral("MAIN Menu"),&shell,[&shell]{shell.navigate(QStringLiteral("main"));});
}
void menus(ApplicationShell& shell, QMenuBar& bar) { moduleMenus(shell, bar, false); }
void inputMenus(ApplicationShell& shell, QMenuBar& bar) { moduleMenus(shell, bar, true); }
} // namespace

ModuleDescriptor transportationModule()
{
    ModuleDescriptor module{QStringLiteral("transportation"),QStringLiteral("Transportation model"),id("input-mode"),{}, {}};
    module.beforeLeave=[](ApplicationShell& shell,const QString& from,const QString& to) {
        if (from==id("input") || !to.startsWith(QStringLiteral("transportation/")))
            return transportationController(shell).leaveInput();
        return true;
    };
    module.screens.append({id("input-mode"),QStringLiteral("TRANSPORTATION MODEL"),inputMode,menus});
    module.screens.append({id("input"),QStringLiteral("TRANSPORTATION INPUT DATA"),input,inputMenus});
    module.screens.append({id("solve-menu"),QStringLiteral("TRANSPORTATION MODEL"),solveMenu,menus});
    module.screens.append({id("northwest"),QStringLiteral("TRANSPORTATION MODEL"),iterations,menus});
    module.screens.append({id("leastcost"),QStringLiteral("TRANSPORTATION MODEL"),iterations,menus});
    module.screens.append({id("vogel"),QStringLiteral("TRANSPORTATION MODEL"),iterations,menus});
    module.screens.append({id("final"),QStringLiteral("TRANSPORTATION MODEL"),finalScreen,menus});
    return module;
}
} // namespace tora::desktop
