#pragma once
#include "application_shell.hpp"
#include <tora/lp/graphical.hpp>
#include <QObject>
#include <QWidget>
#include <memory>
class QPrinter;
class QPainter;
namespace tora::desktop {
class LpGraphicalController final:public QObject {
    Q_OBJECT
public:
    explicit LpGraphicalController(ApplicationShell& shell);
    std::function<void(const QString&,const QString&)> notice;
    bool request();
    bool canStart() const;
    bool start(tora::core::NumberFormat format);
    const tora::lp::GraphicalSession* session() const {return session_.get();}
    void selectRow(int row);
    void zoom(int direction);
signals:
    void changed();
private:
    ApplicationShell& shell_;
    bool requested_=false;
    std::unique_ptr<tora::lp::GraphicalSession> session_;
};
class GraphPlot final:public QWidget {
    Q_OBJECT
public:
    explicit GraphPlot(LpGraphicalController& controller,QWidget* parent=nullptr);
    QRectF plotRect() const;
    QPointF mapPoint(tora::lp::GraphPoint point) const;
protected:
    void paintEvent(QPaintEvent*) override;
private:
    LpGraphicalController& controller_;
};
LpGraphicalController& graphicalController(ApplicationShell& shell);
QWidget* graphicalScreen(ApplicationShell& shell);
bool printGraph(QPrinter& printer,const LpGraphicalController& controller,const QFont& font);
} // namespace tora::desktop
