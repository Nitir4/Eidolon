#pragma once
#include <tora/lp/final_solution.hpp>
#include <QFont>
#include <QRectF>
#include <tora/lp/graphical.hpp>
#include <functional>
#include <QPrinterInfo>
class QPrinter;
class QPainter;
class QWidget;
namespace tora::desktop {
enum class PrintOutcome {Cancelled,Complete,Failed};
PrintOutcome requestLpPrint(QWidget*,const std::function<bool(QPrinter&)>&,
                            const QPrinterInfo& = QPrinterInfo::defaultPrinter());
bool printLpSummary(QPrinter&,const tora::lp::FinalSolutionSession&,
                    tora::core::NumberFormat,const QFont&);
bool printLpTrace(QPrinter&,const tora::lp::PrimalSimplexSession&,
                  tora::core::NumberFormat,const QFont&);
bool printLpGraph(QPrinter&,const tora::lp::GraphicalSession&,const QFont&,
                  const std::function<void(QPainter&,const QRectF&,const QFont&)>&);
} // namespace tora::desktop
