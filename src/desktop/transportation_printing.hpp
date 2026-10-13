#pragma once
#include "lp_printing.hpp"
#include <tora/transportation/final_summary.hpp>
#include <tora/transportation/north_west.hpp>

namespace tora::desktop {
class TransportationController;
bool printTransportationInput(QPrinter&,const transportation::Problem&,bool finalStyle=false);
bool printTransportationTrace(QPrinter&,const TransportationController&,std::size_t shown);
bool printTransportationSummary(QPrinter&,const transportation::FinalSummary&);
// The observed Final command submits original input and summary separately.
// Cancellation of the second selection cannot retract the first completed job.
PrintOutcome requestTransportationFinalPrint(QWidget*,const TransportationController&,
    const QPrinterInfo& = QPrinterInfo::defaultPrinter(),const PrintSelector& = {});
} // namespace tora::desktop
