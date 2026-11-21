#pragma once
#include <QWidget>
#include <QStringList>
#include <functional>
class QComboBox;
class QPlainTextEdit;
class QPushButton;
class QPrinter;
namespace tora::desktop {
class HistoryView final:public QWidget {
    Q_OBJECT
public:
    explicit HistoryView(const QString& prefix,QWidget* parent=nullptr);
    void setHistory(const QStringList& titles,std::function<QString(int)> render);
    QString completeText()const;
    bool exportTo(const QString& path)const;
    bool printTo(QPrinter& printer)const;
private:
    void showStep(int index);
    QComboBox* selector_;QPlainTextEdit* text_;QPushButton* previous_;QPushButton* next_;
    std::function<QString(int)> render_;
};
}
