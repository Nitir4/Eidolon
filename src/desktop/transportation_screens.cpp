#include "transportation_screens.hpp"
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
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QResizeEvent>
#include <QStyle>
#include <QTableWidget>
#include <QVBoxLayout>
#include <algorithm>
#include <exception>
#include <memory>

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
    addButton(*row, QStringLiteral("Exit TORA"), "transport-exit", &shell, [&shell] { shell.close(); });
    row->addStretch();
    layout.addLayout(row);
}
void openProblem(ApplicationShell& shell)
{
    const auto filename = QFileDialog::getOpenFileName(&shell, QStringLiteral("Open Transportation Problem"), {},
                                                      QStringLiteral("TORA files (*.txt);;All files (*)"));
    if (filename.isEmpty()) return;
    QString error;
    if (!transportationController(shell).openFile(filename, error)) {
        QMessageBox::warning(&shell, QStringLiteral("Cannot Open Problem"), error);
        return;
    }
    shell.navigate(id("input"));
}
void solveProblem(ApplicationShell& shell)
{
    QString error;
    if (!transportationController(shell).solve(error)) {
        QMessageBox::warning(&shell, QStringLiteral("Cannot Solve Problem"), error);
        return;
    }
    shell.navigate(id("northwest"));
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
QWidget* inputMode(ApplicationShell& shell)
{
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);
    auto* panel = new QWidget;
    auto* choices = new QVBoxLayout(panel);
    auto* title = new QLabel(QStringLiteral("TRANSPORTATION MODEL"));
    title->setAlignment(Qt::AlignCenter);
    choices->addWidget(title);
    auto* description = new QLabel(QStringLiteral("Open an existing TORA problem or load the example.\nNorth-west corner start · automatic iterations\nSaved problems are opened read only."));
    description->setAlignment(Qt::AlignCenter);
    choices->addWidget(description);
    button(QStringLiteral("Enter/Edit Existing Problem"), QStringLiteral("transport-open"), *choices, [&shell] { openProblem(shell); });
    button(QStringLiteral("Load Example"), QStringLiteral("transport-example"), *choices, [&shell] {
        transportationController(shell).loadExample();
        shell.navigate(id("input"));
    });
    centerPanel(*layout, panel);
    footer(*layout, shell, false);
    return page;
}
QWidget* input(ApplicationShell& shell)
{
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);
    auto* title = new QLabel;
    title->setWordWrap(true);
    layout->addWidget(title);
    auto* status = new QLabel(QStringLiteral("Saved problem · read only"));
    layout->addWidget(status);
    auto* grid = table(page, "transport-input-grid");
    layout->addWidget(grid, 1);
    auto& controller = transportationController(shell);
    auto* row = new QHBoxLayout;
    addButton(*row, QStringLiteral("Open Problem"), "transport-input-open", page, [&shell] { openProblem(shell); });
    row->addStretch();
    auto* solve = addButton(*row, QStringLiteral("SOLVE Menu"), "transport-solve-menu", page,
                            [&shell] { shell.navigate(id("solve-menu")); });
    layout->addLayout(row);
    const auto refresh = [&controller, title, grid, solve] {
        grid->clear();
        solve->setEnabled(controller.loaded());
        if (!controller.loaded()) {
            title->setText(QStringLiteral("Open a Transportation problem to view its input data."));
            grid->setRowCount(0); grid->setColumnCount(0); return;
        }
        const auto& p = controller.problem();
        const int m = static_cast<int>(p.supply.size()), n = static_cast<int>(p.demand.size());
        title->setText(QStringLiteral("%1\nNo. Sources: %2     No. Dest'ns: %3").arg(text(p.title)).arg(m).arg(n));
        grid->setRowCount(m+3); grid->setColumnCount(n+3);
        const core::NumberFormatter formatter({core::Notation::Decimal,5,2});
        put(*grid, 1, 1, QStringLiteral("S/D Name"), true);
        put(*grid, 0, n+2, QStringLiteral("Supply"), true);
        for (int j=0; j<n; ++j) {
            put(*grid, 0, j+2, QStringLiteral("D%1").arg(j+1), true);
            put(*grid, 1, j+2, text(p.destinationNames[static_cast<std::size_t>(j)]));
            put(*grid, m+2, j+2, text(transportation::quantity(p.demand[static_cast<std::size_t>(j)])), true);
        }
        for (int i=0; i<m; ++i) {
            const auto r=static_cast<std::size_t>(i);
            put(*grid, i+2, 0, QStringLiteral("S%1").arg(i+1), true);
            put(*grid, i+2, 1, text(p.sourceNames[r]));
            for (int j=0; j<n; ++j) put(*grid, i+2, j+2, text(formatter.format(p.costs[r][static_cast<std::size_t>(j)])));
            put(*grid, i+2, n+2, text(transportation::quantity(p.supply[r])), true);
        }
        put(*grid, m+2, 0, QStringLiteral("Demand"), true);
        grid->fitColumns(2,n);
    };
    QObject::connect(&controller, &TransportationController::changed, page, refresh);
    refresh();
    footer(*layout, shell, false);
    return page;
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
    centerPanel(*layout, panel);
    footer(*layout, shell, true);
    return page;
}
QWidget* iterations(ApplicationShell& shell)
{
    auto& controller = transportationController(shell);
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);
    auto* heading = new QLabel(QStringLiteral("TRANSPORTATION TABLEAU - (North-West Corner Method)"));
    heading->setWordWrap(true); layout->addWidget(heading);
    auto* title = new QLabel;
    title->setWordWrap(true); layout->addWidget(title);
    auto* controls = new QHBoxLayout;
    auto* next = new QPushButton(QStringLiteral("Next Iteration"));
    next->setObjectName(QStringLiteral("transport-next")); controls->addWidget(next);
    auto* all = new QPushButton(QStringLiteral("All Iterations"));
    all->setObjectName(QStringLiteral("transport-all")); controls->addWidget(all);
    controls->addStretch();
    controls->addWidget(new QLabel(QStringLiteral("Initialize u or v: u1 = 0")));
    layout->addLayout(controls);
    auto* grid = table(page, "transport-tableau"); layout->addWidget(grid, 1);
    auto shown = std::make_shared<std::size_t>(1);
    const auto render = [&controller, grid, next, all, shown, title] {
        grid->clearSpans(); grid->clear();
        const auto& history=controller.trace();
        const auto& p=controller.problem();
        title->setText(QStringLiteral("Title: %1 — (minimum cost)").arg(text(p.title)));
        const int m=static_cast<int>(p.supply.size()), n=static_cast<int>(p.demand.size()), height=3*m+4;
        const auto count=std::min(*shown,history.size());
        grid->setRowCount(static_cast<int>(count)*height); grid->setColumnCount(n+4);
        next->setEnabled(count<history.size()); all->setEnabled(count<history.size());
        for (std::size_t k=0; k<count; ++k) {
            const int base=static_cast<int>(k)*height;
            const auto d=transportation::display(p,history[k],k+1);
            put(*grid, base, 0, QStringLiteral("Iter %1").arg(k+1), true);
            put(*grid, base, 1, QStringLiteral("ObjVal ="), true);
            put(*grid, base, 2, text(d.objective), true);
            put(*grid, base+1, 1, QStringLiteral("Name"), true);
            put(*grid, base+1, n+3, QStringLiteral("Supply"), true);
            for (int j=0; j<n; ++j) {
                put(*grid, base, j+3, QStringLiteral("D%1").arg(j+1), true);
                put(*grid, base+1, j+3, text(p.destinationNames[static_cast<std::size_t>(j)]), true);
                put(*grid, base+2, j+3, text(d.v[static_cast<std::size_t>(j)]), true);
                put(*grid, base+height-1, j+3, text(d.demand[static_cast<std::size_t>(j)]), true);
            }
            put(*grid, base+height-1, 1, QStringLiteral("Demand"), true);
            for (int i=0; i<m; ++i) {
                const int row=base+3+3*i; const auto r=static_cast<std::size_t>(i);
                put(*grid, row, 0, QStringLiteral("S%1").arg(i+1), true); grid->setSpan(row,0,3,1);
                put(*grid, row, 1, text(p.sourceNames[r])); grid->setSpan(row,1,3,1);
                put(*grid, row, 2, text(d.u[r]), true); grid->setSpan(row,2,3,1);
                put(*grid, row, n+3, text(d.supply[r]), true); grid->setSpan(row,n+3,3,1);
                for (int j=0; j<n; ++j) {
                    const auto c=static_cast<std::size_t>(j);
                    put(*grid,row,j+3,text(d.costs[r][c]));
                    auto* shipment=put(*grid,row+1,j+3,text(d.shipments[r][c]));
                    QFont large=grid->font(); large.setPointSizeF(large.pointSizeF()*1.2); large.setBold(true); shipment->setFont(large);
                    put(*grid,row+2,j+3,text(d.reducedCosts[r][c]));
                    if (k+1<count && history[k].nextPivot) {
                        const auto& pivot=*history[k].nextPivot;
                        for (const auto& cell:pivot.loop) if (cell.source==r && cell.destination==c)
                            shipment->setBackground(QColor(colors::header));
                        if (pivot.entering.source==r && pivot.entering.destination==c) shipment->setBackground(QColor(colors::entering));
                        if (pivot.leaving.source==r && pivot.leaving.destination==c) shipment->setBackground(QColor(colors::leaving));
                    }
                }
            }
        }
        grid->fitColumns(3,n);
    };
    QObject::connect(&controller, &TransportationController::changed, page, [shown,render] { *shown=1; render(); });
    const auto advance = [&controller, &shell, shown, render, grid](bool complete) {
        const auto total=controller.trace().size();
        if (*shown>=total) return;
        *shown=complete ? total : *shown+1; render();
        if (grid->rowCount()) grid->scrollToItem(grid->item(grid->rowCount()-1,1));
        if (*shown==total) QMessageBox::information(&shell,QStringLiteral("Optimal Tableau"),
                                                   QStringLiteral("Iteration %1 is optimal").arg(total));
    };
    QObject::connect(next,&QPushButton::clicked,page,[advance]{advance(false);});
    QObject::connect(all,&QPushButton::clicked,page,[advance]{advance(true);});
    render(); footer(*layout,shell,true);
    return page;
}
void menus(ApplicationShell& shell, QMenuBar& bar)
{
    auto* file=bar.addMenu(QStringLiteral("File"));
    auto* open=file->addAction(QStringLiteral("Open…"),&shell,[&shell]{openProblem(shell);});
    open->setShortcut(QKeySequence::Open);
    auto* solve=bar.addMenu(QStringLiteral("Solve Problem"));
    auto* northwest=solve->addAction(QStringLiteral("Iterations — North-west corner start"),&shell,[&shell]{solveProblem(shell);});
    northwest->setEnabled(transportationController(shell).loaded());
    bar.addAction(QStringLiteral("MAIN Menu"),&shell,[&shell]{shell.navigate(QStringLiteral("main"));});
}
} // namespace

TransportationController::TransportationController(QObject* parent):QObject(parent)
{ setObjectName(QStringLiteral("transportation-controller")); }
void TransportationController::load(transportation::Problem problem)
{
    transportation::validate(problem);
    problem_=std::move(problem); trace_.clear(); loaded_=true; emit changed();
}
void TransportationController::loadExample()
{
    auto p=transportation::Problem::blank(2,3);
    p.title="Transportation example"; p.costs={{2,3,1},{5,4,8}}; p.supply={20,30}; p.demand={10,25,15}; load(std::move(p));
}
bool TransportationController::openFile(const QString& filename, QString& error)
{
    error.clear();
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) { error=file.errorString(); return false; }
    if (file.size()>4*1024*1024) { error=QStringLiteral("Transportation file exceeds the 4 MiB limit."); return false; }
    try {
        const auto bytes=file.readAll();
        if (file.error()!=QFileDevice::NoError) { error=file.errorString(); return false; }
        load(compat::readTransportation(std::string_view(bytes.constData(),static_cast<std::size_t>(bytes.size()))));
        return true;
    } catch (const std::exception& e) { error=QString::fromUtf8(e.what()); return false; }
}
bool TransportationController::solve(QString& error)
{
    error.clear();
    if (!loaded_) { error=QStringLiteral("Open a Transportation problem first."); return false; }
    try {
        auto result=transportation::solveNorthWest(problem_);
        const auto cellsPerTable=(3*problem_.supply.size()+4)*(problem_.demand.size()+4);
        if (result.size()>250000/cellsPerTable) {
            error=QStringLiteral("This Transportation preview limits the displayed trace to 250,000 cells. The problem requires a larger trace.");
            return false;
        }
        trace_=std::move(result); emit changed(); return true;
    } catch (const std::exception& e) { error=QString::fromUtf8(e.what()); return false; }
}
TransportationController& transportationController(ApplicationShell& shell)
{
    if (auto* existing=shell.findChild<TransportationController*>(QStringLiteral("transportation-controller"))) return *existing;
    return *new TransportationController(&shell);
}
ModuleDescriptor transportationModule()
{
    ModuleDescriptor module{QStringLiteral("transportation"),QStringLiteral("Transportation model"),id("input-mode"),{}, {}};
    module.screens.append({id("input-mode"),QStringLiteral("TRANSPORTATION MODEL"),inputMode,menus});
    module.screens.append({id("input"),QStringLiteral("TRANSPORTATION INPUT DATA"),input,menus});
    module.screens.append({id("solve-menu"),QStringLiteral("TRANSPORTATION MODEL"),solveMenu,menus});
    module.screens.append({id("northwest"),QStringLiteral("TRANSPORTATION MODEL"),iterations,menus});
    return module;
}
} // namespace tora::desktop
