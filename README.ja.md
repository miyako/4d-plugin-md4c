# 4d-plugin-md4c

[English](README.md) | 日本語

C言語で書かれた高速な CommonMark パーサー [md4c](https://github.com/mity/md4c)（v0.6.0）で、
4D から Markdown を HTML に変換します。md4c はプラグインに静的リンクされているため、
ほかに何もインストールする必要はありません。

ほかの出力形式（XML、CommonMark、man、LaTeX、プレーンテキスト）が必要な場合は、
同じ形のコマンドを持つ [4d-plugin-cmark-gfm](https://github.com/miyako/4d-plugin-cmark-gfm) を参照してください。

## 動作環境

- 4D v21.1 以降
- macOS 11 以降（Apple シリコンおよび Intel）、または Windows x64

## インストール

[Releases](../../releases) ページから最新リリースをダウンロードします。

1. `.zip`（macOS と Windows）または公証済みの `.dmg`（macOS）をダウンロードします
2. `md4c.bundle` を、プロジェクトまたは 4D アプリケーションの **Plugins** フォルダーにコピーします
3. 4D を再起動します

## コマンド

### `MD4C Convert`

```4d
$result:=MD4C Convert($source {; $options})
```

| 引数 | 型 | 説明 |
|---|---|---|
| `$source` | テキスト | Markdown ソース |
| `$options` | オブジェクト | 省略可。方言とフラグ（下記参照） |
| `$result` | オブジェクト | `success`、`status`、`result`、失敗時は `error` |

このコマンドはスレッドセーフで、プリエンプティブプロセスから呼び出せます。

**戻り値のオブジェクト**

| プロパティ | 型 | 説明 |
|---|---|---|
| `success` | ブール | 変換に成功した場合は `True` |
| `status` | 整数 | 成功時は `0`。`1` 内部エラー、`2` 不正なオプション、`3` md4c の失敗 |
| `result` | テキスト | HTML の断片（`<html>`/`<body>` は含まない）。失敗時は `""` |
| `error` | テキスト | 失敗時のみ。エラーの内容 |

**オプション**

| プロパティ | 型 | デフォルト | 説明 |
|---|---|---|---|
| `dialect` | テキスト | `"commonmark"` | `"commonmark"` または `"github"`（表、取り消し線、タスクリスト、寛容な自動リンク、注記ブロック、脚注）。それ以外の値はエラー |
| `tables` | ブール | | 表 |
| `strikethrough` | ブール | | `~~text~~` |
| `tasklists` | ブール | | `- [x] item` |
| `permissiveAutolinks` | ブール | | `<` `>` なしの URL、メール、`www.` 自動リンク（下の3つのフラグをまとめたもの） |
| `permissiveURLAutolinks` | ブール | | `<` `>` なしの URL |
| `permissiveEmailAutolinks` | ブール | | `<` `>` と `mailto:` なしのメールアドレス |
| `permissiveWWWAutolinks` | ブール | | スキームなしの `www.` リンク |
| `permissiveATXHeaders` | ブール | | スペースなしの `#Heading` |
| `collapseWhitespace` | ブール | | 通常のテキスト中の連続する空白をまとめる |
| `noIndentedCodeBlocks` | ブール | | フェンス付きコードブロックのみ |
| `noHTML` | ブール | | 生の HTML を無効にする（`noHTMLBlocks` と `noHTMLSpans` をまとめたもの） |
| `noHTMLBlocks` | ブール | | 生の HTML ブロックを無効にする |
| `noHTMLSpans` | ブール | | インラインの生の HTML を無効にする |
| `latexMathSpans` | ブール | | `$...$` と `$$...$$` |
| `wikiLinks` | ブール | | `[[target]]` |
| `underline` | ブール | | `_text_` を強調ではなく下線にする |
| `hardSoftBreaks` | ブール | | すべての改行を `<br>` にする |
| `spoilers` | ブール | | `\|\|hidden\|\|` |
| `superscripts` | ブール | | `^text^` |
| `subscripts` | ブール | | `~text~` |
| `highlight` | ブール | | `==text==` |
| `insert` | ブール | | `++text++` |
| `admonitions` | ブール | | 注記ブロック |
| `footnotes` | ブール | | `[^label]` 脚注 |
| `preserveBlankLines` | ブール | | ブロック間の余分な空行を `<br>` にする |
| `xhtml` | ブール | `False` | XHTML で出力する（`<br />`） |
| `verbatimEntities` | ブール | `False` | `&copy;` などのエンティティを書かれたまま残す |
| `skipUTF8BOM` | ブール | `False` | 先頭のバイトオーダーマークを取り除く |

まず `dialect` が適用され、その後に各ブール値がフラグをオン（`True`）またはオフ（`False`）にします。
たとえば `{dialect: "github"; strikethrough: False}` は取り消し線なしの GitHub になります。
型の異なる値は無視されます。ソース中の生の HTML は `noHTML` を指定しない限りそのまま出力されるため、
信頼できない入力はエスケープまたはサニタイズしてください。

**例**

```4d
var $result : Object
$result:=MD4C Convert("# Hello\n\n~~old~~ new"; {dialect: MD4C dialect GitHub})
If ($result.success)
	WA SET PAGE CONTENT(*; "WebArea"; $result.result; "")
Else
	ALERT($result.error)
End if
```

## 定数

| 定数 | 型 | 値 |
|---|---|---|
| `MD4C dialect CommonMark` | テキスト | `commonmark` |
| `MD4C dialect GitHub` | テキスト | `github` |

## ソースからのビルド

### 必要なもの

- CMake 3.20 以降
- Xcode（macOS）または Visual Studio 2022 以降（Windows）

### クローン

```bash
git clone --recurse-submodules https://github.com/miyako/4d-plugin-md4c.git
cd 4d-plugin-md4c
```

### ビルド（macOS）

```bash
cmake -S md4c -B md4c/build -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
cmake --build md4c/build --config Release
```

### ビルド（Windows）

```pwsh
cmake -S md4c -B md4c/build -A x64
cmake --build md4c/build --config Release
```

プラグインは `md4c/md4c-test/Plugins` に出力されます。

### テストの実行

[tool4d](https://developer.4d.com/docs/Admin/cli/)（無料、ライセンス不要）が必要です。

```bash
/path/to/tool4d --dataless --startup-method=test_all --project=$(pwd)/md4c/md4c-test/Project/md4c.4DProject
```

すべてのテストに合格すると `PASS` と出力されます。

## CI/CD

このプロジェクトは、[4d-plugin-tutorial](https://github.com/miyako/4d-plugin-tutorial) から生成された、
3つのワークフローからなる標準のネイティブプラグイン用パイプラインを使用します。

| ワークフロー | トリガー | 目的 |
|---|---|---|
| `test.yml` | プッシュ / PR / 手動 / 再利用 | 同じリビジョンの成果物をビルド、テスト、検証 |
| `bump-version.yml` | 手動、デフォルトブランチのみ | `VERSION` を上げてコミット。タグなし |
| `release.yml` | 手動、デフォルトブランチのみ。入力なし | テストを再利用し、署名、公証、検証、ドラフト作成、`vVERSION` を公開 |

### 必要なシークレット（`release.yml` のみ）

**Settings → Secrets and variables → Actions** で設定します。

| シークレット | 説明 |
|---|---|
| `APPLE_DEVELOPER_ID_CERTIFICATE` | 秘密鍵を含む Developer ID Application `.p12` を Base64 エンコードしたもの |
| `APPLE_DEVELOPER_ID_CERTIFICATE_PASSWORD` | `.p12` 書き出し時のパスワード |
| `KEYCHAIN_PASSWORD` | CI ランナーの一時キーチェーン用の独立したパスワード |
| `NOTARYTOOL_APPLE_ID` | 公証に使う Apple ID のメールアドレス |
| `NOTARYTOOL_TEAM_ID` | Apple Developer のチーム ID（英数字10文字） |
| `NOTARYTOOL_PASSWORD` | アカウントのパスワードではなく、Apple の App 用パスワード |

## ライセンス

プラグインは [MIT License](LICENSE) で公開されています。md4c は © Martin Mitáš、MIT License です。
[THIRD-PARTY-NOTICES](THIRD-PARTY-NOTICES/md4c.txt) を参照してください。
