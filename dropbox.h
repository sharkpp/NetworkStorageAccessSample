#ifndef DROPBOX_H
#define DROPBOX_H

#include <QtCore>
#include <QtNetwork>
#include <QtNetworkAuth>

class DropboxOAuthOobReplyHandler;

class Dropbox
    : public QOAuth2AuthorizationCodeFlow
{
    Q_OBJECT

public:
    Dropbox(QObject *parent = nullptr);

    void authenticate();

    void setPinCode(const QString& code);

    const QString serialize() const;
    void deserialize(const QString& token);

    bool upload(const QByteArray& data, const QString& path);
    bool download(const QString& path);

signals:
    void authenticated();
    void uploaded(const QString& path);

protected slots:
    void finishedUpload();

protected:
    QNetworkReply* post(const QUrl& url, const QVariant& postData, const QMap<QVariant, QString>& headers);

private:
    DropboxOAuthOobReplyHandler* customReplyHandler;
};

#endif // DROPBOX_H
