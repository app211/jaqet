// -------------------------------------------------------------------------------
//  Project:		Qt Webbased File Handling
//
//  File:           entities.cpp
//
// (c) 2012 - 2013 by Oblivion Software/Norbert Schlia
// This source code is licensed under The Code Project Open License (CPOL),
// see http://www.codeproject.com/info/cpol10.aspx
// -------------------------------------------------------------------------------
//
#include <QString>
#include <QRegExp>
#include <QHash>

#include "entities.h"

typedef struct KNOWNCHARACTERS
{
    const char *	m_pszHTML;
    const char *    m_pszChar;
} KNOWNCHARACTERS;

// ------------------------------------------------------------------------------
// List of known characters for HTML. Used to convert their &...; representation
// into UTF-8.
// ------------------------------------------------------------------------------

static KNOWNCHARACTERS KnownCharacters[] =
{
    { "AElig", "Æ" },
    { "Aacute", "Á" },
    { "Acirc", "Â" },
    { "Agrave", "À" },
    { "Alpha", "Α" },
    { "Aring", "Å" },
    { "Atilde", "Ã" },
    { "Auml", "Ä" },
    { "Beta", "Β" },
    { "Ccedil", "Ç" },
    { "Chi", "Χ" },
    { "Dagger", "‡" },
    { "Delta", "Δ" },
    { "ETH", "Ð" },
    { "Eacute", "É" },
    { "Ecirc", "Ê" },
    { "Egrave", "È" },
    { "Epsilon", "Ε" },
    { "Eta", "Η" },
    { "Euml", "Ë" },
    { "Gamma", "Γ" },
    { "Iacute", "Í" },
    { "Icirc", "Î" },
    { "Igrave", "Ì" },
    { "Iota", "Ι" },
    { "Iuml", "Ï" },
    { "Kappa", "Κ" },
    { "Lambda", "Λ" },
    { "Mu", "Μ" },
    { "Ntilde", "Ñ" },
    { "Nu", "Ν" },
    { "OElig", "Œ" },
    { "Oacute", "Ó" },
    { "Ocirc", "Ô" },
    { "Ograve", "Ò" },
    { "Omega", "Ω" },
    { "Omicron", "Ο" },
    { "Oslash", "Ø" },
    { "Otilde", "Õ" },
    { "Ouml", "Ö" },
    { "Phi", "Φ" },
    { "Pi", "Π" },
    { "Prime", "″" },
    { "Psi", "Ψ" },
    { "Rho", "Ρ" },
    { "Scaron", "Š" },
    { "Sigma", "Σ" },
    { "THORN", "Þ" },
    { "Tau", "Τ" },
    { "Theta", "Θ" },
    { "Uacute", "Ú" },
    { "Ucirc", "Û" },
    { "Ugrave", "Ù" },
    { "Upsilon", "Υ" },
    { "Uuml", "Ü" },
    { "Xi", "Ξ" },
    { "Yacute", "Ý" },
    { "Yuml", "Ÿ" },
    { "Zeta", "Ζ" },
    { "aacute", "á" },
    { "acirc", "â" },
    { "acute", "´" },
    { "aelig", "æ" },
    { "agrave", "à" },
    { "alefsym", "ℵ" },
    { "alpha", "α" },
    { "amp", "&" },
    { "and", "∧" },
    { "ang", "∠" },
    { "apos", "'" },
    { "aring", "å" },
    { "asymp", "≈" },
    { "atilde", "ã" },
    { "auml", "ä" },
    { "bdquo", "„" },
    { "beta", "β" },
    { "brvbar", "¦" },
    { "bull", "•" },
    { "cap", "∩" },
    { "ccedil", "ç" },
    { "cedil", "¸" },
    { "cent", "¢" },
    { "chi", "χ" },
    { "circ", "ˆ" },
    { "clubs", "♣" },
    { "cong", "≅" },
    { "copy", "©" },
    { "crarr", "↵" },
    { "cup", "∪" },
    { "curren", "¤" },
    { "dArr", "⇓" },
    { "dagger", "†" },
    { "darr", "↓" },
    { "deg", "°" },
    { "delta", "δ" },
    { "diams", "♦" },
    { "divide", "÷" },
    { "eacute", "é" },
    { "ecirc", "ê" },
    { "egrave", "è" },
    { "empty", "∅" },
    { "emsp", " " },
    { "ensp", " " },
    { "epsilon", "ε" },
    { "equiv", "≡" },
    { "eta", "η" },
    { "eth", "ð" },
    { "euml", "ë" },
    { "euro", "€" },
    { "exist", "∃" },
    { "fnof", "ƒ" },
    { "forall", "∀" },
    { "frac12", "½" },
    { "frac14", "¼" },
    { "frac34", "¾" },
    { "frasl", "⁄" },
    { "gamma", "γ" },
    { "ge", "≥" },
    { "gt", ">" },
    { "hArr", "⇔" },
    { "harr", "↔" },
    { "hearts", "♥" },
    { "hellip", "…" },
    { "iacute", "í" },
    { "icirc", "î" },
    { "iexcl", "¡" },
    { "igrave", "ì" },
    { "image", "ℑ" },
    { "infin", "∞" },
    { "int", "∫" },
    { "iota", "ι" },
    { "iquest", "¿" },
    { "isin", "∈" },
    { "iuml", "ï" },
    { "kappa", "κ" },
    { "lArr", "⇐" },
    { "lambda", "λ" },
    { "lang", "〈" },
    { "laquo", "«" },
    { "larr", "←" },
    { "lceil", "⌈" },
    { "ldquo", "“" },
    { "le", "≤" },
    { "lfloor", "⌊" },
    { "lowast", "∗" },
    { "loz", "◊" },
    { "lrm", "\xE2\x80\x8E" },
    { "lsaquo", "‹" },
    { "lsquo", "‘" },
    { "lt", "<" },
    { "macr", "¯" },
    { "mdash", "—" },
    { "micro", "µ" },
    { "middot", "·" },
    { "minus", "−" },
    { "mu", "μ" },
    { "nabla", "∇" },
    { "nbsp", " " },
    { "ndash", "–" },
    { "ne", "≠" },
    { "ni", "∋" },
    { "not", "¬" },
    { "notin", "∉" },
    { "nsub", "⊄" },
    { "ntilde", "ñ" },
    { "nu", "ν" },
    { "oacute", "ó" },
    { "ocirc", "ô" },
    { "oelig", "œ" },
    { "ograve", "ò" },
    { "oline", "‾" },
    { "omega", "ω" },
    { "omicron", "ο" },
    { "oplus", "⊕" },
    { "or", "∨" },
    { "ordf", "ª" },
    { "ordm", "º" },
    { "oslash", "ø" },
    { "otilde", "õ" },
    { "otimes", "⊗" },
    { "ouml", "ö" },
    { "para", "¶" },
    { "part", "∂" },
    { "permil", "‰" },
    { "perp", "⊥" },
    { "phi", "φ" },
    { "pi", "π" },
    { "piv", "ϖ" },
    { "plusmn", "±" },
    { "pound", "£" },
    { "prime", "′" },
    { "prod", "∏" },
    { "prop", "∝" },
    { "psi", "ψ" },
    { "quot", "\"" },
    { "rArr", "⇒" },
    { "radic", "√" },
    { "rang", "〉" },
    { "raquo", "»" },
    { "rarr", "→" },
    { "rceil", "⌉" },
    { "rdquo", "”" },
    { "real", "ℜ" },
    { "reg", "®" },
    { "rfloor", "⌋" },
    { "rho", "ρ" },
    { "rlm", "\xE2\x80\x8F" },
    { "rsaquo", "›" },
    { "rsquo", "’" },
    { "sbquo", "‚" },
    { "scaron", "š" },
    { "sdot", "⋅" },
    { "sect", "§" },
    { "shy", "\xC2\xAD" },
    { "sigma", "σ" },
    { "sigmaf", "ς" },
    { "sim", "∼" },
    { "spades", "♠" },
    { "sub", "⊂" },
    { "sube", "⊆" },
    { "sum", "∑" },
    { "sup", "⊃" },
    { "sup1", "¹" },
    { "sup2", "²" },
    { "sup3", "³" },
    { "supe", "⊇" },
    { "szlig", "ß" },
    { "tau", "τ" },
    { "there4", "∴" },
    { "theta", "θ" },
    { "thetasym", "ϑ" },
    { "thinsp", " " },
    { "thorn", "þ" },
    { "tilde", "˜" },
    { "times", "×" },
    { "trade", "™" },
    { "uArr", "⇑" },
    { "uacute", "ú" },
    { "uarr", "↑" },
    { "ucirc", "û" },
    { "ugrave", "ù" },
    { "uml", "¨" },
    { "upsih", "ϒ" },
    { "upsilon", "υ" },
    { "uuml", "ü" },
    { "weierp", "℘" },
    { "xi", "ξ" },
    { "yacute", "ý" },
    { "yen", "¥" },
    { "yuml", "ÿ" },
    { "zeta", "ζ" },
    { "zwj", "\xE2\x80\x8D" },
    { "zwnj", "\xE2\x80\x8C" },
    { NULL, NULL }
};

static bool findEntity(const QString & strKnown, QString & strReplace)
{
    bool bFound = false;

    strReplace.clear();

    for (int i = 0; KnownCharacters[i].m_pszChar != NULL; i++)
    {
        if (strKnown == KnownCharacters[i].m_pszHTML)
        {
            strReplace = KnownCharacters[i].m_pszChar;
            bFound = true;
            break;
        }
    }

    return bFound;
}

void decode_html_entities(QString & strLine)
{
    QRegExp rx("&([^;]*)", Qt::CaseSensitive);
    QHash<QString, int> hash;
    int pos = 0;

    while ((pos = rx.indexIn(strLine, pos)) != -1)
    {
        hash[rx.cap(1)]++;
        pos += rx.matchedLength();
    }

    for (QHash<QString, int>::const_iterator i = hash.constBegin(); i != hash.constEnd(); i++)
    {
        QString strReplace;

        if (findEntity(i.key(), strReplace))
        {
            strLine.replace("&" + i.key() + ";", strReplace);
        }
    }
}
