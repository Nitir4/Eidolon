#pragma once
#include <QProcess>
#include <QString>

namespace tora::test {
// Use the same configured Python/pypdf as the existing PDF verification gates.
inline bool extractPdfText(const QString& path, QString& text, QString& error) {
    text.clear();
    error.clear();
    QProcess process;
    process.start(QString::fromUtf8(TORA_PDF_PYTHON),
                  {QString::fromUtf8(TORA_PDF_EXTRACTOR), path});
    if (!process.waitForStarted(10000)) {
        error = QStringLiteral("Cannot start PDF text reader: %1").arg(process.errorString());
        return false;
    }
    if (!process.waitForFinished(30000)) {
        error = QStringLiteral("PDF text reader did not finish: %1").arg(process.errorString());
        process.kill();
        process.waitForFinished();
        return false;
    }
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        error = QStringLiteral("PDF text reader failed (exit %1): %2")
                    .arg(process.exitCode())
                    .arg(QString::fromUtf8(process.readAllStandardError()));
        return false;
    }
    text = QString::fromUtf8(process.readAllStandardOutput())
               .normalized(QString::NormalizationForm_KC).simplified();
    return true;
}
}
