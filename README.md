# 概要

本プログラムはGTFS-JP(zip形式)を読んで処理を行うコマンドラインツールです。<p>
GTFS-JPの妥当性をチェックする検証機能、複数のGTFS-JPをひとつにまとめるマージ機能、複数のエージェンシーから構成されているGTFS-JPをエージェント単位に分割する機能、経路別のバス時刻表を表示する機能を有しています。<p>
GTFS-JPファイルの指定はコンピュータにダウンロードされている場合はファイル名を指定します。インターネット上に公開さているGTFS-JPの場合はURLを指定します。

# 使用方法
```
$ gtfstool [コマンドオプション] GTFS-JPファイル名 ...
```
```
コマンドオプション
    [-c gtfs.zip ...] GTFS-JPの妥当性チェックを行います(default)
    [-d gtfs.zip ...] GTFS-JPのルート別にバス時刻表を表示します
    [-s output_dir gtfs.zip ...] 複数のagencyを分割します
    [-m merge.conf] 複数のGTFS-JPを一つにマージします
    [-v] プログラムバージョンを表示します
    [-w] 警告を無視します
    [-i] チェック時にcalendar_dates.txtのservice_idがcalender.txtに存在するかチェックします
    [-e error_file] システムエラーを出力するファイルを指定します
    [-t] トレースモードをオンにして実行します
```

# 使用例
```
$ gtfstool /path/to/gtfs-jp_20181001.zip
$ gtfstool -w http://loc.bus-vision.jp/gtfs/ryobi/gtfsFeed
```
チェック結果は標準出力に表示されますので必要に応じてリダイレクト機能を使ってファイルに保存してください。
```
$ gtfstool /path/to/gtfs-jp_20181001.zip >result.txt
```

# 実行形式
WindowsとMacOS X用ではコンパイル済みの実行形式が用意されています。
zip形式で提供されていますので展開することでプログラムが出現します。

# ソースコードからのビルド方法
ソールコードをGitHubからダウンロードしてターミナルでconfigureとmakeを行います。
環境に合わせて実行ファイルが作成されます。
```
$ ./configure && make
```
