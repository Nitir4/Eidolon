#include "lp_screens.hpp"
#include "screen_widgets.hpp"
#include "lp_input_controller.hpp"
#include "lp_simplex_controller.hpp"

#include <QAction>
#include <QFormLayout>
#include <QFrame>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMenuBar>
#include <QMenu>
#include <QPushButton>
#include <QRadioButton>
#include <QToolButton>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTableView>
#include <QSignalBlocker>
#include <QApplication>
#include <QStyledItemDelegate>
#include <QKeyEvent>
#include <QStyle>
#include <QRegularExpressionValidator>

namespace tora::desktop {
namespace {
constexpr auto previewReason = "This solution method is not available yet.";
class InputDelegate final : public QStyledItemDelegate {
public:
    InputDelegate(LpInputController& c,QObject* parent):QStyledItemDelegate(parent),controller_(c) {}
    QWidget* createEditor(QWidget* parent,const QStyleOptionViewItem& option,const QModelIndex& index) const override {
        auto* editor=QStyledItemDelegate::createEditor(parent,option,index);
        if(auto* text=qobject_cast<QLineEdit*>(editor);text && index.row()>0) {
            const int n=index.model()->columnCount()-3,m=index.model()->rowCount()-5;
            QString pattern=QStringLiteral("[-+0-9.eE]*");
            if(index.row()==m+4) {
                pattern=QStringLiteral("[yYnN]?");
                QObject::connect(text,&QLineEdit::textEdited,text,[text](const QString& value){text->setText(value.toLower());});
            }else if(index.column()==n+1) {
                pattern=QStringLiteral("(?:<=?|>=?|=)?");
                QObject::connect(text,&QLineEdit::textEdited,text,[text](const QString& value){if(value==QStringLiteral("<")||value==QStringLiteral(">"))text->setText(value+QLatin1Char('='));});
            }else if(index.row()==m+2 || index.row()==m+3)pattern=QStringLiteral("(?:[-+0-9.eE]*|-?infinity)");
            text->setValidator(new QRegularExpressionValidator(QRegularExpression(pattern),text));
        }
        controller_.setEditing(editor!=nullptr);return editor;
    }
    void destroyEditor(QWidget* editor,const QModelIndex& index) const override {
        controller_.setEditing(false);QStyledItemDelegate::destroyEditor(editor,index);
    }
    bool eventFilter(QObject* object,QEvent* event) override {
        if(event->type()==QEvent::KeyPress) {
            auto* key=static_cast<QKeyEvent*>(event);
            if(key->key()==Qt::Key_Return || key->key()==Qt::Key_Enter) {
                auto* editor=qobject_cast<QWidget*>(object);
                emit commitData(editor);emit closeEditor(editor,QAbstractItemDelegate::EditNextItem);return true;
            }
        }
        return QStyledItemDelegate::eventFilter(object,event);
    }
private:
    LpInputController& controller_;
};
class InputView final : public QTableView {
public:
    QSize sizeHint() const override {
        auto size=QTableView::sizeHint();
        size.setHeight(18*verticalHeader()->defaultSectionSize()+horizontalHeader()->sizeHint().height()+2*frameWidth());
        return size;
    }
protected:
    QModelIndex moveCursor(CursorAction action,Qt::KeyboardModifiers modifiers) override {
        if(action!=MoveNext && action!=MovePrevious)return QTableView::moveCursor(action,modifiers);
        const int count=model()->rowCount()*model()->columnCount();
        if(!count)return {};
        int at=currentIndex().row()*model()->columnCount()+currentIndex().column();
        for(int tries=0;tries<count;++tries) {
            at=(at+(action==MoveNext?1:-1)+count)%count;
            const auto next=model()->index(at/model()->columnCount(),at%model()->columnCount());
            if(model()->flags(next)&Qt::ItemIsEditable)return next;
        }
        return {};
    }
};

class NotationCountEdit final : public QLineEdit {
public:
    explicit NotationCountEdit(const QString& value) : QLineEdit(value) {
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    }
    QSize sizeHint() const override {
        const QFontMetrics metrics(font());
        return {metrics.horizontalAdvance(QStringLiteral("00000")) + 2 * metrics.averageCharWidth(),
                QLineEdit::sizeHint().height()};
    }
    QSize minimumSizeHint() const override { return sizeHint(); }
};

class ConstraintCountEdit final : public QLineEdit {
public:
    std::function<void()> initialize;
protected:
    bool event(QEvent* event) override {
        if(event->type()==QEvent::KeyPress) {
            const auto* key=static_cast<QKeyEvent*>(event);
            if(key->key()==Qt::Key_Return || key->key()==Qt::Key_Enter || key->key()==Qt::Key_Tab) {
                // Qt editingFinished omits an unchanged field on Tab; TORA
                // reinitializes on that key even when the counts are unchanged.
                if(initialize)initialize();
                event->accept();return true;
            }
        }
        return QLineEdit::event(event);
    }
};

QGroupBox* formatPanel(const QString& title, const QString& prefix, bool editable = false)
{
    auto* group = new QGroupBox(title);
    auto* layout = new QVBoxLayout(group);
    auto* decimal = new QRadioButton(QStringLiteral("Decimal Notation (NNNNN.DD)"));
    decimal->setObjectName(prefix + QStringLiteral("-decimal"));
    decimal->setChecked(true);
    auto* scientific = new QRadioButton(QStringLiteral("Scientific Notation (N.NNNNeDD)"));
    scientific->setObjectName(prefix + QStringLiteral("-scientific"));
    layout->addWidget(decimal);
    layout->addWidget(scientific);
    auto* fields = new QFormLayout;
    for (const auto& field : {std::make_pair(QStringLiteral("N"), QStringLiteral("5")),
                              std::make_pair(QStringLiteral("D"), QStringLiteral("2"))}) {
        auto* entry = new NotationCountEdit(field.second);
        entry->setObjectName(prefix + QStringLiteral("-") + field.first);
        entry->setAccessibleName(QStringLiteral("How many %1?").arg(field.first));
        // Numerical acceptance/formatting is a later foundation. Keep the
        // observed defaults visible without inventing validation limits.
        entry->setReadOnly(!editable);
        fields->addRow(QStringLiteral("How many %1?").arg(field.first), entry);
    }
    layout->addLayout(fields);
    return group;
}

void footer(QVBoxLayout& layout, ApplicationShell& shell, bool modify = false, bool solveMenu = false)
{
    auto* row = new QHBoxLayout;
    row->addStretch();
    const auto add = [&](const QString& text, const QString& id, const QString& screen) {
        auto* control = new QPushButton(text);
        control->setObjectName(id);
        row->addWidget(control);
        QObject::connect(control, &QPushButton::clicked, &shell, [&shell, screen] {
            if (screen.isEmpty()) shell.close(); else shell.navigate(screen);
        });
    };
    if (modify) add(QStringLiteral("View/Modify Input Data"), QStringLiteral("view-input"), QStringLiteral("lp/input"));
    if (solveMenu) {
        add(QStringLiteral("SOLVE Menu"), QStringLiteral("input-solve-menu"), QStringLiteral("lp/solve-menu"));
    }
    add(QStringLiteral("MAIN Menu"), QStringLiteral("return-main"), QStringLiteral("main"));
    add(QStringLiteral("Exit TORA"), QStringLiteral("exit-tora"), {});
    row->addStretch();
    layout.addLayout(row);
}

void solveEntries(QMenu& menu, ApplicationShell& shell)
{
    auto* solve = menu.addMenu(QStringLiteral("Solve Problem"));
    auto* graphical = solve->addAction(QStringLiteral("Graphical"));
    graphical->setEnabled(false);
    graphical->setToolTip(QString::fromLatin1(previewReason));
    auto* algebraic = solve->addMenu(QStringLiteral("Algebraic"));
    auto* final = algebraic->addAction(QStringLiteral("Final solution"));
    final->setEnabled(false);
    final->setToolTip(QString::fromLatin1(previewReason));
    auto* iterations = algebraic->addMenu(QStringLiteral("Iterations"));
    for (const auto& label : {"All-slack starting solution", "M-method", "Two-phase method", "Bounded simplex", "Dual simplex"}) {
        auto* action = iterations->addAction(QString::fromLatin1(label));
        if(QString::fromLatin1(label)==QStringLiteral("All-slack starting solution")) {
            action->setObjectName(QStringLiteral("solve-all-slack"));auto& input=inputController(shell);
            const auto update=[action,&input]{action->setEnabled(input.document()!=nullptr);};
            QObject::connect(&input,&LpInputController::changed,iterations,update);update();
            QObject::connect(action,&QAction::triggered,&shell,[&shell]{simplexController(shell).requestAllSlack();});
        }else {action->setEnabled(false);action->setToolTip(QString::fromLatin1(previewReason));}
    }
    menu.setToolTipsVisible(true);
    solve->setToolTipsVisible(true);
    algebraic->setToolTipsVisible(true);
    iterations->setToolTipsVisible(true);
    menu.addAction(QStringLiteral("View/Modify Input Data"), &shell,
        [&shell] { shell.navigate(QStringLiteral("lp/input")); });
    menu.addSeparator();
    menu.addAction(QStringLiteral("MAIN Menu"), &shell, [&shell] { shell.navigate(QStringLiteral("main")); });
    menu.addSeparator();
    menu.addAction(QStringLiteral("Exit TORA"), &shell, &QWidget::close);
}
}

ModuleDescriptor lpModule()
{
    ModuleDescriptor module{QStringLiteral("lp"), QStringLiteral("Linear Programming"), QStringLiteral("lp/input-mode"), {}, {}};
    const QString heading = QStringLiteral("LINEAR PROGRAMMING");
    module.beforeLeave=[](ApplicationShell& shell,const QString& from,const QString& to) {
        if(from!=QStringLiteral("lp/input"))return true;
        auto& c=inputController(shell);
        if(!c.document())return true;
        if(to==QStringLiteral("lp/solve-menu") && !c.validateForSolve())return false;
        return c.leaveInput();
    };
    module.screens.append({QStringLiteral("lp/input-mode"), {}, [](ApplicationShell& shell) {
        auto* page = new QWidget;
        auto* layout = new QVBoxLayout(page);
        auto* group = new QGroupBox(QStringLiteral("Select Input Mode"));
        auto* inner = new QVBoxLayout(group);
        auto* newProblem = new QRadioButton(QStringLiteral("Enter New Problem"));
        newProblem->setObjectName(QStringLiteral("input-new"));
        newProblem->setChecked(true);
        auto* existing = new QRadioButton(QStringLiteral("Select Existing File"));
        existing->setObjectName(QStringLiteral("input-existing"));
        inner->addWidget(newProblem);
        inner->addWidget(existing);
        auto& c=inputController(shell);
        auto* format = formatPanel(QStringLiteral("Select Input Format"), QStringLiteral("input-format"),true);
        inner->addWidget(format);
        auto* go = button(QStringLiteral("Go to Input Screen"), QStringLiteral("input-go"), *inner,
            [&shell,&c,existing,format] {
                if(existing->isChecked()) {c.beginNew();shell.navigate(QStringLiteral("lp/input"));c.requestOpen();return;}
                bool nOk=false,dOk=false;
                const int n=format->findChild<QLineEdit*>(QStringLiteral("input-format-N"))->text().toInt(&nOk);
                const int d=format->findChild<QLineEdit*>(QStringLiteral("input-format-D"))->text().toInt(&dOk);
                tora::core::NumberFormat settings{format->findChild<QRadioButton*>(QStringLiteral("input-format-scientific"))->isChecked()?tora::core::Notation::Scientific:tora::core::Notation::Decimal,n,d};
                try {if(!nOk || !dOk)throw std::invalid_argument("Enter whole numbers for N and D");(void)tora::core::NumberFormatter(settings);}
                catch(const std::exception& e){c.dialogs().showError(QStringLiteral("Input Format"),QString::fromUtf8(e.what()));return;}
                c.beginNew(settings);shell.navigate(QStringLiteral("lp/input"));
            });
        QObject::connect(existing, &QRadioButton::toggled, go, [go, format](bool checked) {
            go->setEnabled(true);
            format->setEnabled(!checked);
        });
        centerPanel(*layout, group);
        footer(*layout, shell);
        return page;
    }, {}});

    module.screens.append({QStringLiteral("lp/input"), heading, [](ApplicationShell& shell) {
        auto& c=inputController(shell);
        auto* page = new QWidget;
        auto* layout = new QVBoxLayout(page);
        auto* fields = new QGroupBox;
        auto* row = new QHBoxLayout(fields);
        auto* form = new QFormLayout;
        const QStringList labels{QStringLiteral("Problem Title:"), QStringLiteral("Nbr. of Variables:"), QStringLiteral("No. of Constraints:")};
        const QStringList ids{QStringLiteral("problem-title"), QStringLiteral("variable-count"), QStringLiteral("constraint-count")};
        for (int i = 0; i < labels.size(); ++i) {
            QLineEdit* value = i==2?new ConstraintCountEdit:new QLineEdit;
            value->setObjectName(ids[i]);
            value->setAccessibleName(labels[i]);
            value->setStyleSheet(QStringLiteral("QLineEdit {background: white; color: black;}"));
            if(i>0)value->setMaximumWidth(value->fontMetrics().horizontalAdvance(QStringLiteral("00000"))+4*value->fontMetrics().averageCharWidth());
            form->addRow(labels[i], value);
        }
        row->addLayout(form);
        auto* instructions = new QLabel(QStringLiteral("Enter value then press RETURN or TAB to initialize input grid"));
        instructions->setWordWrap(true);
        row->addWidget(instructions, 1);
        layout->addWidget(fields);
        auto* gridHeading = new QLabel(QStringLiteral("INPUT GRID - LINEAR PROGRAMMING"));
        gridHeading->setAlignment(Qt::AlignCenter);
        layout->addWidget(gridHeading);
        auto* view=new InputView;
        view->setObjectName(QStringLiteral("input-grid"));view->setModel(c.grid());
        view->setItemDelegate(new InputDelegate(c,view));
        view->verticalHeader()->hide();view->horizontalHeader()->setHighlightSections(false);
        view->setSelectionMode(QAbstractItemView::NoSelection);
        view->setEditTriggers(QAbstractItemView::DoubleClicked|QAbstractItemView::SelectedClicked|QAbstractItemView::EditKeyPressed|QAbstractItemView::AnyKeyPressed);
        view->setStyleSheet(QStringLiteral("QTableView {background: #00ffff; gridline-color: white; border: 1px solid white;} QHeaderView::section {background: #00ffff; color: blue; border: 1px solid white;} QTableView QLineEdit {background: white; color: black; selection-background-color: #000080; selection-color: white;}"));
        auto* gridRow=new QHBoxLayout;gridRow->addStretch();gridRow->addWidget(view,1);gridRow->addStretch();
        auto* gridArea=new QVBoxLayout;gridArea->addLayout(gridRow);gridArea->addStretch(1);
        layout->addLayout(gridArea,1);
        footer(*layout, shell, false, true);
        auto* title=page->findChild<QLineEdit*>(QStringLiteral("problem-title"));
        auto* variables=page->findChild<QLineEdit*>(QStringLiteral("variable-count"));
        auto* constraints=page->findChild<QLineEdit*>(QStringLiteral("constraint-count"));
        const auto metrics=[view,gridArea,&c] {
            const QFontMetrics fm(view->font());
            auto bold=view->font();bold.setBold(true);const QFontMetrics numericFm(bold);
            view->verticalHeader()->setDefaultSectionSize(fm.height()+fm.height()/2);
            const auto* model=view->model();
            int total=0;
            QString numeric=QStringLiteral("-00000.00");
            if(c.document()) {
                const auto f=c.document()->inputFormat;
                numeric=f.notation==tora::core::Notation::Decimal?QStringLiteral("-")+QString(f.n,QLatin1Char('0'))+QLatin1Char('.')+QString(f.d,QLatin1Char('0')):
                    QStringLiteral("-0.")+QString(f.n,QLatin1Char('0'))+QStringLiteral("e+")+QString(f.d,QLatin1Char('0'));
            }
            for(int col=0;col<model->columnCount();++col) {
                int width=col==0?fm.horizontalAdvance(QStringLiteral("Unrestr'd (y/n)?")):numericFm.horizontalAdvance(numeric);
                width=std::max(width,fm.horizontalAdvance(model->headerData(col,Qt::Horizontal).toString()));
                if(col>0)for(int row=1;row<model->rowCount();++row)width=std::max(width,numericFm.horizontalAdvance(model->data(model->index(row,col)).toString()));
                width+=2*fm.averageCharWidth();view->setColumnWidth(col,width);total+=width;
            }
            if(total>0)view->setMaximumWidth(total+view->style()->pixelMetric(QStyle::PM_ScrollBarExtent)+2*view->frameWidth());
            view->setMaximumHeight(std::max(18,model->rowCount())*view->verticalHeader()->defaultSectionSize()+view->horizontalHeader()->sizeHint().height()+2*view->frameWidth());
            gridArea->setStretch(0,model->rowCount()>18?1:0);gridArea->setStretch(1,model->rowCount()>18?0:1);
        };
        const auto update=[page,title,variables,constraints,view,gridHeading,&c,metrics] {
            const QSignalBlocker a(title),b(variables),d(constraints);
            const auto* doc=c.document();
            title->setText(doc?legacyText(doc->problem.title):QString());
            variables->setText(doc?QString::number(doc->problem.variables.size()):QString());
            constraints->setText(doc?QString::number(doc->problem.constraints.size()):QString());
            page->findChild<QPushButton*>(QStringLiteral("input-solve-menu"))->setEnabled(doc!=nullptr);
            view->setVisible(doc!=nullptr);gridHeading->setVisible(doc!=nullptr);
            metrics();
        };
        QObject::connect(&c,&LpInputController::changed,page,update);
        QObject::connect(&shell,&ApplicationShell::displayMetricsChanged,page,metrics);
        QObject::connect(&c,&LpInputController::commitRequested,view,[view,title,&c] {
            if(!c.setTitle(title->text()))return;
            if(auto* focused=QApplication::focusWidget();focused && view->isAncestorOf(focused) && focused!=view) view->setFocus();
        });
        QObject::connect(&c,&LpInputController::editRequested,view,[view](int row,int col) {
            const auto index=view->model()->index(row,col);view->setCurrentIndex(index);view->scrollTo(index);view->edit(index);
        });
        QObject::connect(title,&QLineEdit::editingFinished,page,[title,&c]{c.setTitle(title->text());});
        QObject::connect(variables,&QLineEdit::returnPressed,constraints,[constraints]{constraints->setFocus();constraints->selectAll();});
        static_cast<ConstraintCountEdit*>(constraints)->initialize=[title,variables,constraints,view,&c] {
            if(c.initialize(title->text(),variables->text().toInt(),constraints->text().toInt())) {
                const auto first=c.grid()->index(0,1);view->setFocus();view->setCurrentIndex(first);view->edit(first);
            }
        };
        QObject::connect(view,&QTableView::clicked,page,[&c](const QModelIndex& index) {
            if(index.column()==0) {if(index.row()==1)c.toggleObjective();else c.selectRow(index.row());}
            else c.clearSelection();
        });
        QObject::connect(view->horizontalHeader(),&QHeaderView::sectionClicked,page,[&c](int section){c.selectColumn(section);});
        update();
        return page;
    }, [](ApplicationShell& shell, QMenuBar& bar) {
        auto& c=inputController(shell);
        auto* file = bar.addMenu(QStringLiteral("File"));
        auto* save=file->addAction(QStringLiteral("Save"),&c,[&c]{c.requestSave();});
        auto* saveAs=file->addAction(QStringLiteral("Save As"),&c,[&c]{c.requestSave();});
        file->addAction(QStringLiteral("Main Menu"), &shell, [&shell] { shell.navigate(QStringLiteral("main")); });
        file->addAction(QStringLiteral("Exit TORA"), &shell, &QWidget::close);
        auto* edit = bar.addMenu(QStringLiteral("EditGrid"));
        const QStringList labels{QStringLiteral("Undo Last Insert, Paste, Or Delete"), QStringLiteral("Copy Column or Row"),
            QStringLiteral("Paste on Column or Row"), QStringLiteral("Delete Column or Row"), QStringLiteral("Insert Column or Row")};
        const QStringList shortcuts{QStringLiteral("Ctrl+U"), QStringLiteral("Ctrl+C"), QStringLiteral("Ctrl+P"),
            QStringLiteral("Ctrl+D"), QStringLiteral("Ctrl+I")};
        QList<QAction*> commands;
        for (int i = 0; i < labels.size(); ++i) {
            auto* action = edit->addAction(labels[i]);
            action->setShortcut(QKeySequence(shortcuts[i]));
            action->setEnabled(false);
            commands.append(action);
        }
        QObject::connect(commands[1],&QAction::triggered,&c,[&c]{c.copySelection();});
        QObject::connect(commands[2],&QAction::triggered,&c,[&c]{c.pasteSelection();});
        QObject::connect(commands[3],&QAction::triggered,&c,[&c]{c.deleteSelection();});
        QObject::connect(commands[4],&QAction::triggered,&c,[&c]{c.insertSelection();});
        const auto update=[&c,save,saveAs,commands] {
            save->setEnabled(c.document() && !c.hasSelection() && !c.editing());saveAs->setEnabled(save->isEnabled());
            commands[1]->setEnabled(c.canCopy());commands[2]->setEnabled(c.canPaste());
            commands[3]->setEnabled(c.canCopy());commands[4]->setEnabled(c.canCopy());
        };
        QObject::connect(&c,&LpInputController::changed,file,update);
        QObject::connect(&c,&LpInputController::selectionChanged,file,update);update();
    }});

    module.screens.append({QStringLiteral("lp/solve-menu"), heading, [](ApplicationShell& shell) {
        auto* page = new QWidget;
        auto* layout = new QVBoxLayout(page);
        auto* panel = new QFrame;
        panel->setStyleSheet(QStringLiteral("QFrame { background: white; color: black; }"
            "QPushButton, QToolButton { background: white; color: black; border: none; text-align: left; padding: 0.15em 0.25em; }"
            "QPushButton:focus, QToolButton:focus, QPushButton:hover, QToolButton:hover { background: #000080; color: white; }"));
        auto* inner = new QVBoxLayout(panel);
        auto* title = new QLabel(QStringLiteral("SOLVE/MODIFY"));
        title->setAlignment(Qt::AlignCenter);
        title->setStyleSheet(QStringLiteral("background: #efefef; color: black;"));
        inner->addWidget(title);
        auto* solve = new QToolButton;
        solve->setText(QStringLiteral("Solve Problem"));
        solve->setObjectName(QStringLiteral("solve-problem"));
        solve->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        solve->setPopupMode(QToolButton::InstantPopup);
        auto* full = new QMenu(solve);
        solveEntries(*full, shell);
        // Share the exact same hierarchy between the native menubar and the
        // centered menu button. The remaining commands are direct buttons.
        solve->setMenu(full->actions().front()->menu());
        inner->addWidget(solve);
        button(QStringLiteral("View/Modify Input Data"), QStringLiteral("solve-view-input"), *inner,
            [&shell] { shell.navigate(QStringLiteral("lp/input")); });
        auto* separator = new QFrame;
        separator->setFrameShape(QFrame::HLine);
        inner->addWidget(separator);
        button(QStringLiteral("MAIN Menu"), QStringLiteral("solve-main"), *inner,
            [&shell] { shell.navigate(QStringLiteral("main")); });
        auto* exitSeparator = new QFrame;
        exitSeparator->setFrameShape(QFrame::HLine);
        inner->addWidget(exitSeparator);
        button(QStringLiteral("Exit TORA"), QStringLiteral("solve-exit"), *inner, [&shell] { shell.close(); });
        centerPanel(*layout, panel);
        return page;
    }, [](ApplicationShell& shell, QMenuBar& bar) {
        solveEntries(*bar.addMenu(QStringLiteral("SolveModify")), shell);
    }});

    // Registered for layout review via --screen. Disabled solver commands do
    // not pretend to compute a result or advance a document to these pages.
    module.screens.append({QStringLiteral("lp/output-format"), heading, [](ApplicationShell& shell) {
        auto* page = new QWidget;
        auto* layout = new QVBoxLayout(page);
        auto& c=simplexController(shell);
        auto* format = formatPanel(QStringLiteral("Select Output Format"), QStringLiteral("output-format"),true);
        auto* inner = qobject_cast<QVBoxLayout*>(format->layout());
        auto* go = button(QStringLiteral("Go To Output Screen"), QStringLiteral("output-go"), *inner, [&c,format] {
            bool nOk=false,dOk=false;const int n=format->findChild<QLineEdit*>(QStringLiteral("output-format-N"))->text().toInt(&nOk);
            const int d=format->findChild<QLineEdit*>(QStringLiteral("output-format-D"))->text().toInt(&dOk);
            if(!nOk || !dOk){c.notice(QStringLiteral("Output Format"),QStringLiteral("Enter whole numbers for N and D"));return;}
            c.start({format->findChild<QRadioButton*>(QStringLiteral("output-format-scientific"))->isChecked()?tora::core::Notation::Scientific:tora::core::Notation::Decimal,n,d});
        });
        const auto update=[go,format,&c]{go->setEnabled(c.canStart());format->findChild<QLineEdit*>(QStringLiteral("output-format-N"))->setReadOnly(!c.canStart());format->findChild<QLineEdit*>(QStringLiteral("output-format-D"))->setReadOnly(!c.canStart());go->setToolTip(c.canStart()?QString():QString::fromLatin1(previewReason));};
        QObject::connect(&c,&LpSimplexController::changed,page,update);update();
        centerPanel(*layout, format);
        footer(*layout, shell, true);
        return page;
    }, {}});
    module.screens.append({QStringLiteral("lp/simplex"),heading,[](ApplicationShell& shell){return simplexScreen(shell);},{}});
    module.screens.append({QStringLiteral("lp/m-value"), heading, [](ApplicationShell& shell) {
        auto* page = new QWidget;
        auto* layout = new QVBoxLayout(page);
        auto* panel = new QGroupBox;
        auto* inner = new QVBoxLayout(panel);
        auto* form = new QFormLayout;
        auto* value = new QLineEdit(QStringLiteral("100"));
        value->setObjectName(QStringLiteral("m-value"));
        value->setReadOnly(true);
        form->addRow(QStringLiteral("Enter value of M:"), value);
        inner->addLayout(form);
        button(QStringLiteral("Go To Output Format Screen"), QStringLiteral("m-go"), *inner,
            [&shell] { shell.navigate(QStringLiteral("lp/output-format")); });
        centerPanel(*layout, panel);
        previewNotice(*layout);
        footer(*layout, shell, true);
        return page;
    }, {}});
    return module;
}
} // namespace tora::desktop
