#pragma once

#include "application_shell.hpp"
#include <tora/transportation/north_west.hpp>
#include <QObject>

namespace tora::desktop {
class TransportationController final : public QObject {
    Q_OBJECT
public:
    explicit TransportationController(QObject* parent);
    void load(transportation::Problem problem);
    void loadExample();
    bool openFile(const QString& filename, QString& error);
    bool solve(QString& error, transportation::StartMethod method = transportation::StartMethod::NorthWest);
    transportation::StartMethod method() const { return method_; }
    bool loaded() const { return loaded_; }
    const transportation::Problem& problem() const { return problem_; }
    const std::vector<transportation::Tableau>& trace() const { return trace_; }
signals:
    void changed();
private:
    bool loaded_ = false;
    transportation::StartMethod method_ = transportation::StartMethod::NorthWest;
    transportation::Problem problem_;
    std::vector<transportation::Tableau> trace_;
};

TransportationController& transportationController(ApplicationShell& shell);
ModuleDescriptor transportationModule();
} // namespace tora::desktop
