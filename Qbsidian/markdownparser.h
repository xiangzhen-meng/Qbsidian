#ifndef MARKDOWNPARSER_H
#define MARKDOWNPARSER_H

#include <QString>

class MarkdownParser {
public:
    static QString parse(const QString &markdown);
};

#endif // MARKDOWNPARSER_H
