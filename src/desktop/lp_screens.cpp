#include "lp_screens.hpp"
#include "screen_widgets.hpp"

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

namespace tora::desktop {
namespace {
constexpr auto previewReason = "Problem editing and solving are not available in this preview.";

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

QGroupBox* formatPanel(const QString& title, const QString& prefix)
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
        entry->setReadOnly(true);
        entry->setToolTip(QStringLiteral("Number formatting is not available in this preview."));
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
        action->setEnabled(false);
        action->setToolTip(QString::fromLatin1(previewReason));
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
    ModuleDescriptor module{QStringLiteral("lp"), QStringLiteral("Linear Programming"), QStringLiteral("lp/input-mode"), {}};
    const QString heading = QStringLiteral("LINEAR PROGRAMMING");
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
        auto* format = formatPanel(QStringLiteral("Select Input Format"), QStringLiteral("input-format"));
        inner->addWidget(format);
        auto* go = button(QStringLiteral("Go to Input Screen"), QStringLiteral("input-go"), *inner,
            [&shell] { shell.navigate(QStringLiteral("lp/input")); });
        QObject::connect(existing, &QRadioButton::toggled, go, [go, format](bool checked) {
            go->setEnabled(!checked);
            format->setEnabled(!checked);
            go->setToolTip(checked ? QStringLiteral("File loading is not available in this preview.") : QString());
        });
        centerPanel(*layout, group);
        previewNotice(*layout);
        footer(*layout, shell);
        return page;
    }, {}});

    module.screens.append({QStringLiteral("lp/input"), heading, [](ApplicationShell& shell) {
        auto* page = new QWidget;
        auto* layout = new QVBoxLayout(page);
        auto* fields = new QGroupBox;
        auto* row = new QHBoxLayout(fields);
        auto* form = new QFormLayout;
        const QStringList labels{QStringLiteral("Problem Title:"), QStringLiteral("Nbr. of Variables:"), QStringLiteral("No. of Constraints:")};
        const QStringList ids{QStringLiteral("problem-title"), QStringLiteral("variable-count"), QStringLiteral("constraint-count")};
        for (int i = 0; i < labels.size(); ++i) {
            auto* value = new QLineEdit;
            value->setObjectName(ids[i]);
            value->setAccessibleName(labels[i]);
            value->setEnabled(false);
            value->setToolTip(QString::fromLatin1(previewReason));
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
        layout->addStretch();
        previewNotice(*layout);
        layout->addStretch();
        footer(*layout, shell, false, true);
        page->findChild<QPushButton*>(QStringLiteral("input-solve-menu"))->setToolTip(
            QStringLiteral("Preview the method menu. Solving is not available yet."));
        return page;
    }, [](ApplicationShell& shell, QMenuBar& bar) {
        auto* file = bar.addMenu(QStringLiteral("File"));
        file->addAction(QStringLiteral("Save"))->setEnabled(false);
        file->addAction(QStringLiteral("Save As"))->setEnabled(false);
        file->addAction(QStringLiteral("Main Menu"), &shell, [&shell] { shell.navigate(QStringLiteral("main")); });
        file->addAction(QStringLiteral("Exit TORA"), &shell, &QWidget::close);
        auto* edit = bar.addMenu(QStringLiteral("EditGrid"));
        const QStringList labels{QStringLiteral("Undo Last Insert, Paste, Or Delete"), QStringLiteral("Copy Column or Row"),
            QStringLiteral("Paste on Column or Row"), QStringLiteral("Delete Column or Row"), QStringLiteral("Insert Column or Row")};
        const QStringList shortcuts{QStringLiteral("Ctrl+U"), QStringLiteral("Ctrl+C"), QStringLiteral("Ctrl+P"),
            QStringLiteral("Ctrl+D"), QStringLiteral("Ctrl+I")};
        for (int i = 0; i < labels.size(); ++i) {
            auto* action = edit->addAction(labels[i]);
            action->setShortcut(QKeySequence(shortcuts[i]));
            action->setEnabled(false);
        }
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
        previewNotice(*layout);
        return page;
    }, [](ApplicationShell& shell, QMenuBar& bar) {
        solveEntries(*bar.addMenu(QStringLiteral("SolveModify")), shell);
    }});

    // Registered for layout review via --screen. Disabled solver commands do
    // not pretend to compute a result or advance a document to these pages.
    module.screens.append({QStringLiteral("lp/output-format"), heading, [](ApplicationShell& shell) {
        auto* page = new QWidget;
        auto* layout = new QVBoxLayout(page);
        auto* format = formatPanel(QStringLiteral("Select Output Format"), QStringLiteral("output-format"));
        auto* inner = qobject_cast<QVBoxLayout*>(format->layout());
        auto* go = button(QStringLiteral("Go To Output Screen"), QStringLiteral("output-go"), *inner, [] {});
        go->setEnabled(false);
        go->setToolTip(QString::fromLatin1(previewReason));
        centerPanel(*layout, format);
        previewNotice(*layout);
        footer(*layout, shell, true);
        return page;
    }, {}});
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
