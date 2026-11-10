/*
 md4c: Markdown to HTML for 4D, using md4c (https://github.com/mity/md4c).

 md4c keeps no global state, so conversions run in parallel in preemptive processes. The
 command never calls the 4D runtime while md4c is working.
*/

#include "md4c-4dplugin.h"
#include "4d-helpers.h"

#include "md4c.h"
#include "md4c-html.h"

#include <string>

namespace {

struct Flag {
    const char *key;
    unsigned value;
};

// Parser flags, by option name. Composite flags come first, so a specific flag that follows
// can switch one of their members back off ({permissiveAutolinks: true; permissiveWWWAutolinks: false}).
const Flag parser_flags[] = {
    {"permissiveAutolinks", MD_FLAG_PERMISSIVEAUTOLINKS},
    {"noHTML", MD_FLAG_NOHTML},
    {"collapseWhitespace", MD_FLAG_COLLAPSEWHITESPACE},
    {"permissiveATXHeaders", MD_FLAG_PERMISSIVEATXHEADERS},
    {"permissiveURLAutolinks", MD_FLAG_PERMISSIVEURLAUTOLINKS},
    {"permissiveEmailAutolinks", MD_FLAG_PERMISSIVEEMAILAUTOLINKS},
    {"permissiveWWWAutolinks", MD_FLAG_PERMISSIVEWWWAUTOLINKS},
    {"noIndentedCodeBlocks", MD_FLAG_NOINDENTEDCODEBLOCKS},
    {"noHTMLBlocks", MD_FLAG_NOHTMLBLOCKS},
    {"noHTMLSpans", MD_FLAG_NOHTMLSPANS},
    {"tables", MD_FLAG_TABLES},
    {"strikethrough", MD_FLAG_STRIKETHROUGH},
    {"tasklists", MD_FLAG_TASKLISTS},
    {"latexMathSpans", MD_FLAG_LATEXMATHSPANS},
    {"wikiLinks", MD_FLAG_WIKILINKS},
    {"underline", MD_FLAG_UNDERLINE},
    {"hardSoftBreaks", MD_FLAG_HARD_SOFT_BREAKS},
    {"spoilers", MD_FLAG_SPOILERS},
    {"superscripts", MD_FLAG_SUPERSCRIPTS},
    {"subscripts", MD_FLAG_SUBSCRIPTS},
    {"admonitions", MD_FLAG_ADMONITIONS},
    {"footnotes", MD_FLAG_FOOTNOTES},
    {"highlight", MD_FLAG_HIGHLIGHT},
    {"insert", MD_FLAG_INSERT},
    {"preserveBlankLines", MD_FLAG_PRESERVEBLANKLINES},
};

const Flag renderer_flags[] = {
    {"xhtml", MD_HTML_FLAG_XHTML},
    {"verbatimEntities", MD_HTML_FLAG_VERBATIM_ENTITIES},
    {"skipUTF8BOM", MD_HTML_FLAG_SKIP_UTF8_BOM},
};

void append_output(const MD_CHAR *text, MD_SIZE size, void *userdata) {
    static_cast<std::string *>(userdata)->append(text, size);
}

}  // namespace

void PluginMain(PA_long32 selector, PA_PluginParameters params) {
    try {
        switch (selector) {
            case 1: MD4C_Convert(params); break;
        }
    } catch (...) {
        // A command that failed half way still owes 4D its result.
        try {
            if (selector == 1) return_failure(params, 1, "internal error");
        } catch (...) {
        }
    }
}

// MD4C Convert(source : Text {; options : Object}) : Object
static void MD4C_Convert(PA_PluginParameters params) {
    std::string source = unistring_utf8(PA_GetStringParameter(params, 1));
    PA_ObjectRef options = PA_GetObjectParameter(params, 2);

    unsigned pflags = MD_DIALECT_COMMONMARK;
    unsigned rflags = 0;

    std::string dialect;
    if (ob_get_s(options, "dialect", dialect)) {
        if (dialect == "github") {
            pflags = MD_DIALECT_GITHUB;
        } else if (dialect != "commonmark") {
            return_failure(params, 2, "unknown dialect \"" + dialect + "\"; use \"commonmark\" or \"github\"");
            return;
        }
    }
    for (const Flag &f : parser_flags) ob_get_flag(options, f.key, f.value, pflags);
    for (const Flag &f : renderer_flags) ob_get_flag(options, f.key, f.value, rflags);

    std::string html;
    html.reserve(source.size() + source.size() / 4 + 64);
    if (md_html(source.data(), MD_SIZE(source.size()), append_output, &html, pflags, rflags) != 0) {
        return_failure(params, 3, "md4c could not convert the source");
        return;
    }
    return_success(params, html.data(), html.size());
}
