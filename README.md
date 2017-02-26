# NetworkStorageAccessSample

Qt 5.8 にテクノロジープレビューとして追加された [Qt Network Authorization](https://doc-snapshots.qt.io/qt5-5.8/qtnetworkauth-index.html) を利用した、ネットワークストレージに対するアクセスのサンプルです。

## 実装済み項目

* Dropbox + OAuth 2.0 認証(oob)
* トークンの保存＆読込
* テストファイルアップロード

※ oob = out-of-band 、要するに PIN をアプリケーションに入力する通知方法のこと

## サービスプロバイダ

|ネットワークストレージ|redirect_uri|備考|
|-|-|-|
|Dropbox|oob|実装済み|
|Dropbox|localhost:xxxx|未実装|
