# 4d-plugin-md4c

English | [日本語](README.ja.md)

Convert Markdown to HTML in 4D with [md4c](https://github.com/mity/md4c) (v0.6.0), a fast
CommonMark parser written in C. md4c is linked statically into the plugin: nothing else
needs to be installed.

For other output formats (XML, CommonMark, man, LaTeX, plain text), see
[4d-plugin-cmark-gfm](https://github.com/miyako/4d-plugin-cmark-gfm), which has the same
command shape.

## Requirements

- 4D v21.1 or later
- macOS 11 or later (Apple silicon and Intel), or Windows x64

## Installation

Download the latest release from the [Releases](../../releases) page.

1. Download the `.zip` (macOS and Windows) or the notarized `.dmg` (macOS)
2. Copy `md4c.bundle` into the **Plugins** folder of your project or of your 4D application
3. Restart 4D

## Commands

### `MD4C Convert`

```4d
$result:=MD4C Convert($source {; $options})
```

| Parameter | Type | Description |
|---|---|---|
| `$source` | Text | Markdown source |
| `$options` | Object | Optional. Dialect and flags, see below |
| `$result` | Object | `success`, `status`, `result` and, on failure, `error` |

The command is thread-safe and can be called from preemptive processes.

**Result object**

| Property | Type | Description |
|---|---|---|
| `success` | Boolean | `True` when the conversion succeeded |
| `status` | Integer | `0` on success; `1` internal error, `2` invalid option, `3` md4c failed |
| `result` | Text | The HTML fragment (no `<html>`/`<body>` wrapper); `""` on failure |
| `error` | Text | Present on failure only: what went wrong |

**Options**

| Property | Type | Default | Description |
|---|---|---|---|
| `dialect` | Text | `"commonmark"` | `"commonmark"` or `"github"` (tables, strikethrough, task lists, permissive autolinks, admonitions, footnotes). Any other value is an error |
| `tables` | Boolean | | Tables |
| `strikethrough` | Boolean | | `~~text~~` |
| `tasklists` | Boolean | | `- [x] item` |
| `permissiveAutolinks` | Boolean | | URL, e-mail and `www.` autolinks without `<` `>` (the three flags below together) |
| `permissiveURLAutolinks` | Boolean | | URLs without `<` `>` |
| `permissiveEmailAutolinks` | Boolean | | E-mail addresses without `<` `>` and `mailto:` |
| `permissiveWWWAutolinks` | Boolean | | `www.` links without a scheme |
| `permissiveATXHeaders` | Boolean | | `#Heading` without a space |
| `collapseWhitespace` | Boolean | | Collapse runs of whitespace in normal text |
| `noIndentedCodeBlocks` | Boolean | | Only fenced code blocks |
| `noHTML` | Boolean | | Disable raw HTML (`noHTMLBlocks` and `noHTMLSpans` together) |
| `noHTMLBlocks` | Boolean | | Disable raw HTML blocks |
| `noHTMLSpans` | Boolean | | Disable inline raw HTML |
| `latexMathSpans` | Boolean | | `$...$` and `$$...$$` |
| `wikiLinks` | Boolean | | `[[target]]` |
| `underline` | Boolean | | `_text_` is underline instead of emphasis |
| `hardSoftBreaks` | Boolean | | Every line break is a `<br>` |
| `spoilers` | Boolean | | `\|\|hidden\|\|` |
| `superscripts` | Boolean | | `^text^` |
| `subscripts` | Boolean | | `~text~` |
| `highlight` | Boolean | | `==text==` |
| `insert` | Boolean | | `++text++` |
| `admonitions` | Boolean | | Admonition blocks |
| `footnotes` | Boolean | | `[^label]` footnotes |
| `preserveBlankLines` | Boolean | | Extra blank lines between blocks become `<br>` |
| `xhtml` | Boolean | `False` | XHTML output (`<br />`) |
| `verbatimEntities` | Boolean | `False` | Keep entities such as `&copy;` as written |
| `skipUTF8BOM` | Boolean | `False` | Drop a leading byte order mark |

`dialect` is applied first; each Boolean then switches its flag on (`True`) or off
(`False`), so `{dialect: "github"; strikethrough: False}` is GitHub without strikethrough.
Values of the wrong type are ignored. Raw HTML in the source is passed through unless
`noHTML` is set: escape or sanitize untrusted input.

**Example**

```4d
var $result : Object
$result:=MD4C Convert("# Hello\n\n~~old~~ new"; {dialect: MD4C dialect GitHub})
If ($result.success)
	WA SET PAGE CONTENT(*; "WebArea"; $result.result; "")
Else
	ALERT($result.error)
End if
```

## Constants

| Constant | Type | Value |
|---|---|---|
| `MD4C dialect CommonMark` | Text | `commonmark` |
| `MD4C dialect GitHub` | Text | `github` |

## Building from Source

### Prerequisites

- CMake 3.20+
- Xcode (macOS) or Visual Studio 2022 or later (Windows)

### Clone

```bash
git clone --recurse-submodules https://github.com/miyako/4d-plugin-md4c.git
cd 4d-plugin-md4c
```

### Build (macOS)

```bash
cmake -S md4c -B md4c/build -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
cmake --build md4c/build --config Release
```

### Build (Windows)

```pwsh
cmake -S md4c -B md4c/build -A x64
cmake --build md4c/build --config Release
```

The plugin is written to `md4c/md4c-test/Plugins`.

### Run Tests

Requires [tool4d](https://developer.4d.com/docs/Admin/cli/) (free, no license needed):

```bash
/path/to/tool4d --dataless --startup-method=test_all --project=$(pwd)/md4c/md4c-test/Project/md4c.4DProject
```

The run prints `PASS` when every test passed.

## CI/CD

This project uses the canonical generated three-workflow native-plugin pipeline from
[4d-plugin-tutorial](https://github.com/miyako/4d-plugin-tutorial):

| Workflow | Trigger | Purpose |
|---|---|---|
| `test.yml` | Push / PR / manual / reusable | Build, test and validate same-revision artifacts |
| `bump-version.yml` | Manual, default branch only | Bump `VERSION` and commit; no tag |
| `release.yml` | Manual, default branch only; no inputs | Reuse tests, sign, notarize, verify, draft and publish `vVERSION` |

### Required Secrets (for `release.yml` only)

Configure these in **Settings → Secrets and variables → Actions**:

| Secret | Description |
|---|---|
| `APPLE_DEVELOPER_ID_CERTIFICATE` | Base64-encoded Developer ID Application `.p12` including its private key |
| `APPLE_DEVELOPER_ID_CERTIFICATE_PASSWORD` | Password for the `.p12` export |
| `KEYCHAIN_PASSWORD` | Independent password for the CI runner's temporary keychain |
| `NOTARYTOOL_APPLE_ID` | Apple ID email for notarization |
| `NOTARYTOOL_TEAM_ID` | Apple Developer Team ID (10-char alphanumeric) |
| `NOTARYTOOL_PASSWORD` | Apple app-specific password, not the account password |

## License

The plugin is released under the [MIT License](LICENSE). md4c is © Martin Mitáš, MIT
License; see [THIRD-PARTY-NOTICES](THIRD-PARTY-NOTICES/md4c.txt).
