#include "assignment_controller.hpp"
#include "equations_document.hpp"
#include "games_document.hpp"
#include "networks_document.hpp"
#include "planning_document.hpp"
#include "queuing_document.hpp"
#include <tora/compat/integer_document.hpp>
#include <tora/compat/transportation_document.hpp>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTest>
using namespace tora::desktop;
class PracticeBank final : public QObject {
    Q_OBJECT
private slots:
    void everyWorkflowHasFive() {
        QFile file(QStringLiteral(TORA_PRACTICE_BANK)+"/bank.json"); QVERIFY(file.open(QIODevice::ReadOnly));
        const auto bank=QJsonDocument::fromJson(file.readAll()).object();
        const auto workflows=bank["workflows"].toObject(); QVERIFY(workflows.size()>=42);
        for(auto it=workflows.begin();it!=workflows.end();++it) QVERIFY2(it.value().toArray().size()>=5,qPrintable(it.key()));
        QCOMPARE(bank["original_comparison"].toString(),QString("pending"));
    }
    void inputFiles_data() {
        QTest::addColumn<QJsonObject>("record");
        QFile file(QStringLiteral(TORA_PRACTICE_BANK)+"/bank.json"); QVERIFY(file.open(QIODevice::ReadOnly));
        for(const auto value:QJsonDocument::fromJson(file.readAll()).object()["cases"].toArray()) {
            const auto record=value.toObject(); QTest::newRow(qPrintable(record["id"].toString()))<<record;
        }
    }
    void inputFiles() {
        QFETCH(QJsonObject,record);
        QFile file(QStringLiteral(TORA_PRACTICE_BANK)+"/"+record["input"].toString()); QVERIFY(file.open(QIODevice::ReadOnly));
        const auto bytes=file.readAll(); const std::string source(bytes.constData(),static_cast<std::size_t>(bytes.size()));
        const auto family=record["family"].toString(); auto model=record["model"].toObject();
        QVERIFY(!record["expected"].toObject().isEmpty());
        if(family=="lp" || family=="integer") {
            if(model.contains("baseline")) model=model["baseline"].toObject();
            const auto lp=family=="lp"?tora::compat::ToraLpCodec::decode(source):tora::compat::IntegerDocument::decode(source).lp;
            QCOMPARE(lp.problem.objective.size(),std::size_t(2));
            QCOMPARE(lp.problem.objective[0],model["objective"].toArray()[0].toDouble());
            QCOMPARE(lp.problem.constraints.size(),static_cast<std::size_t>(model["constraints"].toArray().size()));
            for(int i=0;i<model["constraints"].toArray().size();++i) QCOMPARE(lp.problem.constraints[static_cast<std::size_t>(i)].rhs,model["constraints"].toArray()[i].toArray()[2].toDouble());
            if(family=="integer") {const auto doc=tora::compat::IntegerDocument::decode(source); QCOMPARE(doc.encode(),source); QCOMPARE(doc.integral[1],model["integral"].toArray()[1]=="y");}
            else QCOMPARE(tora::compat::ToraLpCodec::encode(lp),source);
        } else if(family=="transportation") {
            const auto doc=tora::compat::TransportationDocument::decode(source); QCOMPARE(doc.encode(),source);
            QCOMPARE(doc.problem().costs[0][0],model["costs"].toArray()[0].toArray()[0].toDouble());
        } else if(family=="assignment") {
            const auto doc=decodeAssignment(bytes); QCOMPARE(doc.model.costs(0,0),model["costs"].toArray()[0].toArray()[0].toDouble());
            QCOMPARE(doc.workers.size(),model["workers"].toArray().size());
        } else if(family=="matrix" || family=="equations") {
            const auto doc=EquationsDocument::decode(source); QCOMPARE(doc.encode(),source); QCOMPARE(doc.matrixOnly(),family=="matrix");
            QCOMPARE(doc.augmented().rows(),static_cast<std::size_t>(model["A"].toArray().size()));
        } else if(family=="game") {
            const auto doc=GamesDocument::decode(source); QCOMPARE(doc.encode(),source); QCOMPARE(doc.payoff().rows(),std::size_t(2));
            QCOMPARE(doc.payoff()(1,1),model["payoffs"].toArray()[1].toArray()[1].toDouble());
        } else if(family=="mst" || family=="route" || family=="flow") {
            const auto doc=NetworksDocument::decode(source,family=="mst"?NetworkMode::MinimalTree:family=="route"?NetworkMode::ShortestRoute:NetworkMode::MaximalFlow);
            QCOMPARE(doc.encode(),source); const auto graph=doc.graph(); QCOMPARE(graph.nodeCount,std::size_t(4));
            QCOMPARE(graph.directed,family=="flow");
        } else if(family=="planning") {
            const auto doc=PlanningDocument::decode(source); QCOMPARE(doc.encode(),source);
            QCOMPARE(doc.problem().activities.size(),std::size_t(5));
            QCOMPARE(doc.problem().method==tora::planning::Method::Pert,model["method"]=="PERT");
        } else if(family=="queue") {
            const auto doc=QueuingDocument::decode(source); QCOMPARE(doc.encode(),source); QCOMPARE(doc.count(),std::size_t(1));
            if(model.contains("arrival")) {const auto queue=doc.model(0); QCOMPARE(queue.arrivalRate,model["arrival"].toDouble()); QCOMPARE(queue.serviceRate,model["service"].toDouble());}
        } else QFAIL("Unvalidated practice input family");
    }
};
QTEST_APPLESS_MAIN(PracticeBank)
#include "practice_bank.moc"
