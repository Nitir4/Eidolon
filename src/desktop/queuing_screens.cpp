#include "queuing_screens.hpp"
#include "ui_font.hpp"
#include "lp_printing.hpp"
#include "result_tables.hpp"
#include <QHash>
#include <QSignalBlocker>
#include <QTabWidget>
#include <QPainter>
#include <QPrinter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QPlainTextEdit>
#include <QMenuBar>
#include <QMenu>
#include <QLabel>
#include <QDialog>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QTextLayout>
#include <QTextOption>
#include <QAction>
#include <QKeySequence>
#include <memory>
namespace tora::desktop {
namespace {
void failure(QueuingController& c,const QString& e){if(!e.isEmpty())c.dialogs().showError("Queuing",e);}
void solve(ApplicationShell& shell){auto& c=queuingController(shell);QString e;if(c.solve(e))shell.navigate("queuing/result");else failure(c,e);}
QWidget* input(ApplicationShell& shell){
 auto& c=queuingController(shell);auto* page=new QWidget;auto* layout=new QVBoxLayout(page);auto* title=new QLineEdit;title->setObjectName("queuing-title");layout->addWidget(new QLabel("Title"));layout->addWidget(title);auto* init=new QPushButton("Initialize");init->setObjectName("queuing-initialize");layout->addWidget(init);auto* help=new QLabel("Rates use a common time unit. Lambda is per idle source for finite N. Enter infinity for unbounded K or N.\nEditGrid: select a scenario heading, then Copy, Paste, Delete or Insert. Single click inserts after; double click inserts before.");help->setWordWrap(true);layout->addWidget(help);
 auto* grid=new QTableWidget;grid->setObjectName("queuing-input-grid");grid->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);layout->addWidget(grid,1);auto updating=std::make_shared<bool>(false);
 auto render=[&c,title,grid,updating]{*updating=true;grid->setColumnCount(5);grid->setHorizontalHeaderLabels({"lambda","mu","c","System limit K","Source limit N"});if(const auto* d=c.document()){title->setText(QString::fromStdString(d->title()));grid->setRowCount(static_cast<int>(d->count()));for(int i=0;i<grid->rowCount();++i){grid->setVerticalHeaderItem(i,new QTableWidgetItem(QString("Scenario %1").arg(i+1)));for(int j=0;j<5;++j)grid->setItem(i,j,new QTableWidgetItem(QString::fromStdString(d->cell(i,j))));}}else{title->clear();grid->setRowCount(0);}grid->setEditTriggers(c.readOnly()?QAbstractItemView::NoEditTriggers:(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed | QAbstractItemView::AnyKeyPressed));title->setReadOnly(c.readOnly());*updating=false;};
 QObject::connect(&c,&QueuingController::inputReset,page,render);QObject::connect(init,&QPushButton::clicked,page,[&c,title]{QString e;if(!c.initialize(title->text(),e))failure(c,e);});QObject::connect(grid,&QTableWidget::cellChanged,page,[&c,grid,updating](int i,int j){if(*updating)return;QString e;if(!c.editCell(i,j,grid->item(i,j)->text(),e))failure(c,e);});
 QObject::connect(grid->verticalHeader(),&QHeaderView::sectionClicked,page,[&c,grid](int row){grid->setCurrentCell(row,0);grid->selectRow(row);c.selectScenario(row);});QObject::connect(grid->verticalHeader(),&QHeaderView::sectionDoubleClicked,page,[&c,grid](int row){grid->setCurrentCell(row,0);grid->selectRow(row);c.selectScenario(row,true);});
 auto commit=[&c,title,grid]{grid->setFocus();QString e;if(!c.setTitle(title->text(),e)){failure(c,e);return;}if(c.readOnly())return;for(int i=0;i<grid->rowCount();++i)for(int j=0;j<5;++j)if(!c.editCell(i,j,grid->item(i,j)->text(),e)){failure(c,e);return;}};QObject::connect(&c,&QueuingController::commitRequested,page,commit);
 auto* row=new QHBoxLayout;auto* add=new QPushButton("Add Scenario");add->setObjectName("queuing-add");auto* erase=new QPushButton("Delete Scenario");erase->setObjectName("queuing-delete");auto* run=new QPushButton("Solve Problem");run->setObjectName("queuing-solve");row->addWidget(add);row->addWidget(erase);row->addWidget(run);layout->addLayout(row);QObject::connect(add,&QPushButton::clicked,page,[&c]{QString e;if(!c.addScenario(e))failure(c,e);});QObject::connect(erase,&QPushButton::clicked,page,[&c,grid]{QString e;if(!c.deleteScenario(grid->currentRow(),e))failure(c,e);});QObject::connect(run,&QPushButton::clicked,page,[&shell]{solve(shell);});auto eligible=[&c,init,add,erase,run]{init->setEnabled(!c.readOnly());add->setEnabled(c.document()&&!c.readOnly()&&c.document()->count()<100);erase->setEnabled(c.document()&&!c.readOnly()&&c.document()->count()>1);run->setEnabled(c.document());};QObject::connect(&c,&QueuingController::changed,page,eligible);eligible();render();polishUsableFonts(*page);return page;
}
QWidget* output(ApplicationShell& shell)
{
    auto& controller = queuingController(shell);
    auto* page = new QWidget;
    auto* layout = new QVBoxLayout(page);
    auto* title = new QLabel;
    title->setTextFormat(Qt::PlainText);
    title->setWordWrap(true);
    auto* options = new QHBoxLayout;
    auto* scenario = new QComboBox;
    scenario->setObjectName("queuing-scenario");
    auto* scenarioLabel = new QLabel("Scenario");
    scenarioLabel->setBuddy(scenario);
    options->addWidget(title,1);
    options->addWidget(scenarioLabel);
    options->addWidget(scenario,1);
    layout->addLayout(options);

    auto* tabs = new QTabWidget;
    tabs->setObjectName("queuing-result-tabs");
    auto* metrics = resultTable("queuing-result");
    metrics->verticalHeader()->hide();
    tabs->addTab(metrics,"Metrics");
    auto* statePage = new QWidget;
    auto* stateLayout = new QVBoxLayout(statePage);
    auto* range = new QHBoxLayout;
    auto* first = new QSpinBox;
    first->setObjectName("queuing-first-state");
    first->setRange(0,1000000);
    auto* count = new QSpinBox;
    count->setObjectName("queuing-state-count");
    count->setRange(1,1000);
    count->setValue(100);
    auto* firstLabel = new QLabel("First state");
    firstLabel->setBuddy(first);
    auto* countLabel = new QLabel("State count");
    countLabel->setBuddy(count);
    range->addWidget(firstLabel);
    range->addWidget(first,1);
    range->addWidget(countLabel);
    range->addWidget(count,1);
    stateLayout->addLayout(range);
    auto* probabilities = resultTable("queuing-state-probabilities");
    probabilities->verticalHeader()->hide();
    probabilities->setColumnCount(3);
    probabilities->setHorizontalHeaderLabels({"State","Probability (pn)","Cumulative probability (Pn)"});
    stateLayout->addWidget(probabilities,1);
    auto* stateNote = new QLabel;
    stateNote->setObjectName("queuing-state-note");
    stateNote->setWordWrap(true);
    stateLayout->addWidget(stateNote);
    const int stateIndex = tabs->addTab(statePage,"State Probabilities");
    auto* notes = new QPlainTextEdit;
    notes->setReadOnly(true);
    notes->setObjectName("queuing-model-notes");
    const int notesIndex = tabs->addTab(notes,"Model Notes");
    layout->addWidget(tabs,1);

    auto* footer = new QHBoxLayout;
    auto* print = new QPushButton("Write to Printer");
    print->setObjectName("queuing-print");
    auto* back = new QPushButton("View/Modify Input Data");
    footer->addWidget(print);
    footer->addWidget(back);
    layout->addLayout(footer);

    const auto renderStates = [&controller,scenario,first,count,probabilities,stateNote] {
        probabilities->setRowCount(0);
        stateNote->clear();
        stateNote->hide();
        const int selected = scenario->currentData().toInt();
        if (selected < 0 || selected >= static_cast<int>(controller.results().size())) return;
        const auto rows = controller.stateProbabilities(selected,first->value(),count->value());
        const core::NumberFormatter format(controller.document()->format());
        probabilities->setRowCount(static_cast<int>(rows.size()));
        for (int row = 0; row < probabilities->rowCount(); ++row) {
            const auto& value = rows[row];
            putResultCell(*probabilities,row,0,QString::number(value.state));
            putResultCell(*probabilities,row,1,QString::fromStdString(format.format(value.probability)));
            putResultCell(*probabilities,row,2,QString::fromStdString(format.format(value.cumulative)));
        }
        const auto& transient = controller.transients()[selected];
        if (rows.empty()) stateNote->setText("No states in the selected range.");
        else if (transient) stateNote->setText(QStringLiteral("Poisson omitted-tail bound = %1")
                                              .arg(QString::number(transient->omittedMass,'g',6)));
        stateNote->setVisible(!stateNote->text().isEmpty());
    };
    const auto render = [&controller,title,scenario,tabs,metrics,notes,stateIndex,notesIndex,print,renderStates] {
        const int total = static_cast<int>(controller.results().size());
        const bool available = total > 0;
        const int selected = scenario->currentData().toInt();
        const bool comparative = selected < 0;
        title->setText(available ? QString::fromStdString(controller.document()->title()) : QStringLiteral("No current solution. Solve the system first."));
        print->setEnabled(available);
        scenario->setEnabled(available);
        tabs->setVisible(available);
        metrics->clearContents();
        if (!available) return;
        QStringList labels{"Model", "Arrival rate (lambda)", "Service rate (mu)", "Servers (c)",
                           "System limit (K)", "Source limit (N)"};
        QStringList headers{"Metric"};
        QList<QHash<QString,QString>> columns;
        QStringList explanations;
        const int from = comparative ? 0 : selected;
        const int to = comparative ? total : selected + 1;
        for (int i = from; i < to; ++i) {
            const auto summary = controller.scenarioSummary(i);
            auto& values = columns.emplaceBack();
            headers << (comparative ? QStringLiteral("Scenario %1").arg(i + 1) : QStringLiteral("Value"));
            values.insert(labels[0],summary.profile);
            for (int field = 0; field < 5; ++field)
                values.insert(labels[field+1],QString::fromStdString(controller.document()->cell(i,field)));
            for (const auto& [label,value] : summary.metrics) {
                if (!labels.contains(label)) labels << label;
                values.insert(label,value);
            }
            if (!summary.notes.isEmpty()) {
                if (comparative) explanations << QStringLiteral("Scenario %1").arg(i + 1);
                explanations << summary.notes << QString{};
            }
        }
        metrics->setColumnCount(static_cast<int>(headers.size()));
        metrics->setHorizontalHeaderLabels(headers);
        metrics->setRowCount(static_cast<int>(labels.size()));
        for (int row = 0; row < metrics->rowCount(); ++row) {
            putResultCell(*metrics,row,0,labels[row],Qt::AlignLeft);
            for (int column = 0; column < columns.size(); ++column)
                putResultCell(*metrics,row,column + 1,columns[column].value(labels[row],"Not applicable"));
        }
        notes->setPlainText(explanations.join('\n').trimmed());
        tabs->setTabVisible(notesIndex,!explanations.isEmpty());
        const bool hasStates = !comparative && (controller.transients()[selected] || controller.results()[selected].metrics);
        tabs->setTabVisible(stateIndex,hasStates);
        renderStates();
    };
    const auto reset = [&controller,scenario,render] {
        const auto old = scenario->currentData();
        const QSignalBlocker blocker(scenario);
        scenario->clear();
        for (std::size_t index = 0; index < controller.results().size(); ++index)
            scenario->addItem(QStringLiteral("Scenario %1").arg(index+1),static_cast<int>(index));
        if (scenario->count()) scenario->addItem("Comparative Analysis",-1);
        if (scenario->count()) scenario->setCurrentIndex(std::max(0,scenario->findData(old)));
        render();
    };
    QObject::connect(&controller,&QueuingController::changed,page,reset);
    QObject::connect(scenario,&QComboBox::currentIndexChanged,page,render);
    QObject::connect(first,&QSpinBox::valueChanged,page,renderStates);
    QObject::connect(count,&QSpinBox::valueChanged,page,renderStates);
    QObject::connect(back,&QPushButton::clicked,page,[&shell]{ shell.navigate("queuing/input"); });
    QObject::connect(print,&QPushButton::clicked,page,[&controller,&shell,scenario,first,count] {
        const auto outcome = requestLpPrint(&shell,[&](QPrinter& printer) {
            return printQueuing(printer,controller,scenario->currentData().toInt(),first->value(),count->value());
        });
        if (outcome == PrintOutcome::Failed) failure(controller,"The print job could not be completed.");
    });
    reset();
    polishUsableFonts(*page);
    return page;
}
void numberFormat(ApplicationShell& shell){auto& c=queuingController(shell);QDialog dialog(&shell);dialog.setWindowTitle("Numeric Format");auto* layout=new QFormLayout(&dialog);auto* notation=new QComboBox;notation->addItems({"Decimal","Scientific"});auto* n=new QSpinBox;n->setRange(0,core::NumberFormatter::MaximumDigits);auto* d=new QSpinBox;d->setRange(0,core::NumberFormatter::MaximumDigits);const auto old=c.document()?c.document()->format():core::NumberFormat{};notation->setCurrentIndex(old.notation==core::Notation::Decimal?0:1);n->setValue(old.n);d->setValue(old.d);layout->addRow("Notation",notation);layout->addRow("N",n);layout->addRow("D",d);auto* buttons=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);layout->addRow(buttons);QObject::connect(buttons,&QDialogButtonBox::accepted,&dialog,&QDialog::accept);QObject::connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);if(dialog.exec()!=QDialog::Accepted)return;QString e;if(!c.setFormat({notation->currentIndex()==0?core::Notation::Decimal:core::Notation::Scientific,n->value(),d->value()},e))failure(c,e);}
void menus(ApplicationShell& shell,QMenuBar& bar,bool editing){auto& c=queuingController(shell);auto* file=bar.addMenu("File");auto* create=file->addAction("New",&shell,[&shell,&c]{if(c.leaveInput()){c.beginNew();shell.navigate("queuing/input");}});auto* open=file->addAction("Open…",&shell,[&shell,&c]{if(c.requestOpen())shell.navigate("queuing/input");});auto* save=file->addAction("Save As…",&c,[&c]{c.requestSave();});create->setShortcut(QKeySequence::New);open->setShortcut(QKeySequence::Open);save->setShortcuts({QKeySequence::Save,QKeySequence::SaveAs});file->addAction("Numeric Format…",&shell,[&shell]{numberFormat(shell);});
 if(editing){auto* edit=bar.addMenu("EditGrid");auto* undo=edit->addAction("Undo Last Insert, Paste, Or Delete");auto* copy=edit->addAction("Copy Scenario");auto* paste=edit->addAction("Paste on Scenario");auto* erase=edit->addAction("Delete Scenario");auto* insert=edit->addAction("Insert Scenario");undo->setObjectName("queuing-undo-action");copy->setObjectName("queuing-copy-action");paste->setObjectName("queuing-paste-action");erase->setObjectName("queuing-delete-action");insert->setObjectName("queuing-insert-action");undo->setShortcut(QKeySequence("Ctrl+U"));copy->setShortcut(QKeySequence("Ctrl+C"));paste->setShortcut(QKeySequence("Ctrl+P"));erase->setShortcut(QKeySequence("Ctrl+D"));insert->setShortcut(QKeySequence("Ctrl+I"));QObject::connect(copy,&QAction::triggered,&c,[&c]{c.copySelection();});auto operation=[&c](int value){QString e;if(!c.editSelection(value,e))failure(c,e);};QObject::connect(undo,&QAction::triggered,&c,[operation]{operation(0);});QObject::connect(paste,&QAction::triggered,&c,[operation]{operation(1);});QObject::connect(erase,&QAction::triggered,&c,[operation]{operation(2);});QObject::connect(insert,&QAction::triggered,&c,[operation]{operation(3);});auto updateEdit=[&c,undo,copy,paste,erase,insert]{undo->setEnabled(c.canUndo());copy->setEnabled(c.hasSelection());paste->setEnabled(c.canPaste());erase->setEnabled(c.hasSelection()&&!c.readOnly()&&c.document()->count()>1);insert->setEnabled(c.hasSelection()&&!c.readOnly()&&c.document()->count()<100);};QObject::connect(&c,&QueuingController::changed,edit,updateEdit);updateEdit();}
 auto* menu=bar.addMenu("Solve Problem");auto* run=menu->addAction("Solve Problem",&shell,[&shell]{solve(shell);});auto update=[&c,run]{run->setEnabled(c.document());};QObject::connect(&c,&QueuingController::changed,menu,update);update();bar.addAction("MAIN Menu",&shell,[&shell]{shell.navigate("main");});}

}
ModuleDescriptor queuingModule(){ModuleDescriptor m{"queuing","Queuing analysis","queuing/input",{}, {}};m.screens.append({"queuing/input","QUEUING",input,[](ApplicationShell& shell,QMenuBar& bar){menus(shell,bar,true);}});m.screens.append({"queuing/result","QUEUING",output,[](ApplicationShell& shell,QMenuBar& bar){menus(shell,bar,false);}});m.beforeLeave=[](ApplicationShell& shell,const QString&,const QString& to){return to.startsWith("queuing/")?queuingController(shell).commitInput():queuingController(shell).leaveInput();};return m;}
bool printQueuing(QPrinter& printer,const QueuingController& c,int scenario,std::size_t first,std::size_t count){if(c.results().empty())return false;QPainter p;if(!p.begin(&printer))return false;const auto area=printer.pageLayout().paintRectPixels(printer.resolution());auto font=p.font();font.setPointSize(10);p.setFont(font);qreal y=area.top();for(const auto& paragraph:c.outputText(scenario,first,count).split('\n')){QTextLayout layout(paragraph.isEmpty()?QString(" "):paragraph,font,&printer);QTextOption option;option.setWrapMode(QTextOption::WrapAnywhere);layout.setTextOption(option);layout.beginLayout();while(true){auto line=layout.createLine();if(!line.isValid())break;line.setLineWidth(area.width());if(y+line.height()>area.bottom()){if(!printer.newPage())return false;y=area.top();}line.draw(&p,QPointF(area.left(),y));y+=line.height()+3;}layout.endLayout();}return p.end();}
}
