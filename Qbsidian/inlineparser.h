#ifndef INLINEPARSER_H
#define INLINEPARSER_H

#include <QString>

class InlineParser
{
public:
    static QString process(const QString &text);
    static QString escapeHtml(const QString &text);
};

#endif // INLINEPARSER_H
