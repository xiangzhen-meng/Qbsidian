#ifndef RENDERENGINE_H
#define RENDERENGINE_H

#pragma once
#include <QString>
#include <QVector>
#include "blockparser.h"  // 需要 LogicalBlock 定义

class RenderEngine {
public:
    static QString render(const QVector<LogicalBlock> &blocks);
};
#endif // RENDERENGINE_H
