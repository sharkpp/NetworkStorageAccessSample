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

  // OAuth 2.0

    // get session save data
    const QString serialize() const;

    // set session saved data
    void deserialize(const QString& token);

    // OAuth authentication method
    void authenticate();

    // OAuth oob(Out-of-Band) PIN code present method
    void setPinCode(const QString& code);

  // API call

    // data upload method
    bool upload(const QByteArray& data, const QString& path);

    // data download method
    bool download(const QString& path);

signals:
    void authenticated();
    void uploaded(const QString& path);
    void downloaded(const QString& path, const QByteArray& data);

protected slots:
    void finishedUpload();
    void finishedDownload();

protected:
    QNetworkReply* post(const QUrl& url, const QVariant& postData, const QMap<QVariant, QString>& headers);

private:
    DropboxOAuthOobReplyHandler* customReplyHandler;
};

#endif // DROPBOX_H
