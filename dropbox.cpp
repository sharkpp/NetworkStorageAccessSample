#include "dropbox.h"
#include <QOAuthHttpServerReplyHandler>

#define STR__(x) #x
#define STR_(x) STR__(x) // うーむ、

Q_DECLARE_METATYPE(QNetworkRequest::KnownHeaders)
Q_DECLARE_METATYPE(QUrlQuery)
static const int userTypeQNetworkRequestKnownHeaders = qRegisterMetaType<QNetworkRequest::KnownHeaders>();
static const int userTypeQUrlQuery = qRegisterMetaType<QUrlQuery>();

// from qabstractoauth2.cpp
namespace Key {
const QString error =              QStringLiteral("error");
const QString errorDescription =   QStringLiteral("error_description");
const QString errorUri =           QStringLiteral("error_uri");
const QString code =               QStringLiteral("code");
const QString redirectUri =        QStringLiteral("redirect_uri");
const QString state =              QStringLiteral("state");
}

// from qabstractoauth2_p.h
const QString bearerFormat = QStringLiteral("Bearer %1"); // Case sensitive

////////////////////////////////////////////////////////////
// DropboxOAuthOobReply class

class DropboxOAuthOobReply
    : public QNetworkReply
{
public:
    DropboxOAuthOobReply(QObject *parent)
        : QNetworkReply(parent)
    {
    }

    void fixContentType()
    {
        setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    }
};

////////////////////////////////////////////////////////////
// DropboxOAuthOobReplyHandler class

class DropboxOAuthOobReplyHandler : public QOAuthOobReplyHandler
{
public:

    DropboxOAuthOobReplyHandler(QObject *parent)
        : QOAuthOobReplyHandler(parent)
    {
    }

    void setPinCode(const QString& code)
    {
        QVariantMap data;
        data.insert(Key::error, "");
        data.insert(Key::code, code);
        data.insert(Key::state, currentState);
        Q_EMIT callbackReceived(data);
    }

    void setState(const QString& state)
    {
        currentState = state;
    }

protected:

    void networkReplyFinished(QNetworkReply *reply)
    {
        DropboxOAuthOobReply *reply_ = (DropboxOAuthOobReply *)reply;
        reply_->fixContentType(); // fix content-type, "text/javascript" to "application/json"
        QOAuthOobReplyHandler::networkReplyFinished(reply);
    }

private:
    QString currentState;
};

////////////////////////////////////////////////////////////
// Dropbox API access class

Dropbox::Dropbox(QObject *parent)
    : QOAuth2AuthorizationCodeFlow(parent)
    , customReplyHandler(new DropboxOAuthOobReplyHandler(parent))
{
    setNetworkAccessManager(new QNetworkAccessManager(this));

    // QAbstractOAuth2::setClientIdentifier()
    // > qmake ... DEFINES+=DROPBOX_APP_KEY="{App key}"
    // --> https://www.dropbox.com/developers/apps/info/{App key}
    setClientIdentifier(STR_(DROPBOX_APP_KEY));

    // QAbstractOAuth2::setClientIdentifierSharedKey()
    // > qmake ... DEFINES+=DROPBOX_APP_SECRET="{App secret}"
    // --> https://www.dropbox.com/developers/apps/info/{App key}
    setClientIdentifierSharedKey(STR_(DROPBOX_APP_SECRET));

    // https://www.dropbox.com/developers/documentation/http/documentation#oauth2-authorize
    setAuthorizationUrl(QUrl("https://www.dropbox.com/oauth2/authorize"));

    // https://www.dropbox.com/developers/documentation/http/documentation#oa2-token
    setAccessTokenUrl(QUrl("https://api.dropbox.com/oauth2/token"));

    //QOAuthHttpServerReplyHandler *replyHandler = new QOAuthHttpServerReplyHandler(12345);
    // 閉じるのはうまくいかない
    // http://qiita.com/rtshaaaa/items/b93e0cd8501411893cba
    //replyHandler->setCallbackText("<script>(function () {window.open('about:blank','_self').close();})();</script>");

    setReplyHandler(customReplyHandler);

    setModifyParametersFunction([&](Stage stage, QVariantMap* data) {
        qDebug() << "stage:" << (int)stage;
        if (Stage::RequestingAuthorization == stage ||
            Stage::RequestingAccessToken   == stage)
        {
            data->remove(Key::redirectUri);
        }
    });

    /*
    // なんか呼ばれない
    connect(this, &QOAuth2AuthorizationCodeFlow::stateChanged, [&](const QString &state) {
        // save authorization connection state
        qDebug() << "state:" << state
        customReplyHandler->setState(state);
    });*/

    connect(this, &QAbstractOAuth::authorizeWithBrowser, [&](QUrl url) {
        QUrlQuery query(url);
        customReplyHandler->setState(query.queryItemValue(Key::state));
        QDesktopServices::openUrl(url);
    });

    connect(this, &QOAuth2AuthorizationCodeFlow::statusChanged, [=](QAbstractOAuth::Status status) {
        if (QAbstractOAuth::Status::Granted == status) {
            Q_EMIT authenticated();
        }
    });

//    起動時のトークン設定では呼ばれないっぽい
//    connect(this, &QOAuth2AuthorizationCodeFlow::granted,
//            this, &Dropbox::authenticated);
}

QNetworkReply* Dropbox::post(const QUrl& url, const QVariant& postData, const QMap<QVariant, QString>& headers)
{
    QNetworkRequest request(url);

    QMap<QVariant, QString>::const_iterator ite = headers.constBegin();
    while (ite != headers.constEnd()) {
        if (QMetaType::QString == ite.key().type()) {
            request.setRawHeader(ite.key().value<QString>().toUtf8(), ite.value().toUtf8());
        }
        else if (userTypeQNetworkRequestKnownHeaders == ite.key().userType()) {
            request.setHeader(ite.key().value<QNetworkRequest::KnownHeaders>(), ite.value());
        }
        ++ite;
    }

    request.setHeader(QNetworkRequest::UserAgentHeader, userAgent());
    const QString bearer = bearerFormat.arg(token());
    request.setRawHeader("Authorization", bearer.toUtf8());

    QByteArray data;
    if (postData.canConvert<QUrlQuery>()) {
        request.setHeader(QNetworkRequest::ContentTypeHeader,
            "application/x-www-form-urlencoded");
        data = postData.value<QUrlQuery>().toString(QUrl::FullyEncoded).toUtf8();
    }
    else if (postData.canConvert<QByteArray>()) {
        request.setHeader(QNetworkRequest::ContentTypeHeader,
            "application/octet-stream");
        data = postData.value<QByteArray>();
    }

    qDebug() << request.rawHeaderList();

    QNetworkReply *reply = networkAccessManager()->post(request, data);
    return reply;
}

void Dropbox::finishedUpload()
{
    auto reply = qobject_cast<QNetworkReply*>(sender());

    QJsonParseError parseError;
    const auto data = reply->readAll();
    const auto document = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error) {
        qDebug() << QString(data);
        qCritical() << "TwitterTimelineModel::parseJson. Error at:" << parseError.offset
                    << parseError.errorString();
        return;
    }
    else if (!document.isObject()) {
        qDebug() << QString(data);
        return;
    }

    const auto object = document.object();
    if (!object.value("error_summary").isUndefined()) {
        qDebug() << document.toJson();
        return;
    }

    qDebug() << "****" << document.toJson();

    QString path = object.value("path_display").toString();

    Q_EMIT uploaded(path);
}

void Dropbox::authenticate()
{
    grant();
}

void Dropbox::setPinCode(const QString& code)
{
    customReplyHandler->setPinCode(code);
}

bool Dropbox::upload(const QByteArray& data, const QString& path)
{
    QUrl url("https://content.dropboxapi.com/2/files/upload");

    QJsonObject json;
    json["path"] = path;
    json["mode"] = "overwrite";
    json["autorename"] = false;
    json["mute"] = false;

    QMap<QVariant, QString> headers;
    headers.insert("Dropbox-API-Arg", QJsonDocument(json).toJson(QJsonDocument::Compact));

    QNetworkReply *reply = post(url, data, headers);
    connect(reply, &QNetworkReply::finished, std::bind(&QAbstractOAuth::finished, this, reply));
    connect(reply, &QNetworkReply::finished, this, &Dropbox::finishedUpload);

    return true;
}

bool Dropbox::download(const QString& path)
{
    return false;
}

const QString Dropbox::serialize() const
{
    if (QAbstractOAuth::Status::Granted != status()) {
        return "";
    }
    return token();
}

void Dropbox::deserialize(const QString& token)
{
    if (token.isEmpty()) {
        return;
    }

    setToken(token);
    setStatus(QAbstractOAuth::Status::Granted);
}
