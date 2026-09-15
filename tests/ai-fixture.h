// Local protocol fixture: never connects to an AI service or reads credentials.
#ifndef AI_FIXTURE_H
#define AI_FIXTURE_H
#include "ai/aiprovider.h"
#include "ai/aitools.h"
#include <QFile>
#include <QJsonDocument>
#include <QTextStream>
#include <cstdio>

class FakeAiProvider : public AiProvider
{
public:
    bool running = false;
    int asks = 0;
    QJsonObject context, lastResult;
    QString lastId, prompt;
    bool lastSuccess = false;
    bool ready() const override { return true; }
    bool connected() const override { return true; }
    bool busy() const override { return running; }
    QString status() const override { return "Fixture"; }
    QJsonArray models() const override { return {}; }
    void connectAccount(const QString &) override {}
    void login() override {}
    void logout() override {}
    void ask(const QString &text, const QJsonObject &value, const QString &) override { prompt = text; context = value; running = true; ++asks; }
    void cancel() override { running = false; emit cancelled(); }
    void newConversation() override {}
    void toolResult(const QString &id, const QJsonObject &result, bool success) override {
        lastId = id; lastResult = result; lastSuccess = success;
    }
};

inline int runAiServerFixture()
{
    QTextStream input(stdin), output(stdout);
    const auto write = [&output](const QJsonObject &value) {
        output << QJsonDocument(value).toJson(QJsonDocument::Compact) << '\n'; output.flush();
    };
    QString thread, turn, directory;
    int serial = 0;
    while (!input.atEnd()) {
        const auto message = QJsonDocument::fromJson(input.readLine().toUtf8()).object();
        const auto method = message["method"].toString();
        const auto params = message["params"].toObject();
        const auto reply = [&](const QJsonObject &result) { write({{"id", message["id"]}, {"result", result}}); };
        if (method == "initialize") reply({{"userAgent", "Parts-test-fixture"}});
        else if (method == "account/read") reply({{"account", QJsonObject{{"type", "chatgpt"}}}});
        else if (method == "model/list") reply({{"data", QJsonArray{QJsonObject{{"model", "fixture"}, {"displayName", "Fixture"}}}}});
        else if (method == "thread/start") {
            if (params["sandbox"] != "read-only" || params["approvalPolicy"] != "never"
                || !params["ephemeral"].toBool() || params["dynamicTools"].toArray() != PartsAi::toolDefinitions()
                || params["developerInstructions"].toString() != PartsAi::hostInstructions()) return 20;
            // Optional protocol export for a separate, explicitly run live smoke check.
            if (qEnvironmentVariableIsSet("PARTS_AI_THREAD_EXPORT")) {
                QFile file(qEnvironmentVariable("PARTS_AI_THREAD_EXPORT"));
                if (!file.open(QIODevice::WriteOnly)) return 21;
                file.write(QJsonDocument(params).toJson());
            }
            directory = params["cwd"].toString();
            thread = QString("fixture-%1").arg(++serial);
            reply({{"thread", QJsonObject{{"id", thread}}}});
        } else if (method == "turn/start") {
            turn = QString("turn-%1").arg(++serial);
            reply({{"turn", QJsonObject{{"id", turn}}}});
            const auto text = params["input"].toArray().first().toObject()["text"].toString();
            if (text.contains("INVALID_JSON")) { output << "not json\n"; output.flush(); continue; }
            if (text.contains("WAIT_FOREVER")) continue;
            if (text.contains("NATIVE_TOOL")) {
                write({{"id", "denied-native"}, {"method", "item/commandExecution/requestApproval"},
                    {"params", QJsonObject{{"threadId", thread}, {"turnId", turn}, {"command", "delete files"}}}});
                continue;
            }
            write({{"id", "fixture-tool"}, {"method", "item/tool/call"}, {"params", QJsonObject{
                {"threadId", thread}, {"turnId", turn}, {"callId", "call-1"}, {"tool", "directory_list"},
                {"arguments", QJsonObject{{"path", "."}, {"offset", 0}}}}}});
        } else if (method == "thread/unsubscribe") reply({});
        else if (method == "account/logout") reply({});
        else if (method.isEmpty() && message["id"] == "fixture-tool") {
            write({{"method", "item/completed"}, {"params", QJsonObject{{"threadId", thread}, {"turnId", turn},
                {"item", QJsonObject{{"type", "agentMessage"}, {"phase", "final_answer"}, {"text", directory}}}}}});
            write({{"method", "turn/completed"}, {"params", QJsonObject{{"threadId", thread},
                {"turn", QJsonObject{{"id", turn}, {"status", "completed"}}}}}});
        }
    }
    return 0;
}
#endif
