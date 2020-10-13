#include "customLibrary.h"

extern "C" {

const char* ItoLog(int v) {
    string str = to_string(v);
    const char* chp = str.data();
    return chp;
}

const char* DtoLog(double v) {
    string str = to_string(v);
    const char* chp = str.data();
    return chp;
}

bool cmp(const RectInfo &v1, const RectInfo &v2) {
    if (v1.lineNumber < v2.lineNumber) return true;
    else if (v1.lineNumber == v2.lineNumber) {
        if (v1.rect.x < v2.rect.x) return true;
    }
    return false;
}

int rInt(double v) {
    v = round(v);
    int result = int(v);
    return result;
}

int weighted(double v) {
    double weightStd = 20.0;
    double weight = weightStd / 20.0;
    double weightedValue = v * weight;
    int result = rInt(weightedValue);
    return result;
}

double weightedEx(double v) {
    double weightStd = 20.0;
    double weight = weightStd / 20.0;
    double result = v * weight;
    return result;
}

Rect mergeRect(Rect r1, Rect r2) {
    int mergeStd = weighted(5);
    int r1_Left = r1.x - mergeStd;
    int r1_Right = r1.x + r1.width + mergeStd;
    int r1_Top = r1.y - mergeStd;
    int r1_Bot = r1.y + r1.height + mergeStd;
    int r2_Left = r2.x;
    int r2_Right = r2.x + r2.width;
    int r2_Top = r2.y;
    int r2_Bot = r2.y + r2.height;
    Rect rect = r1;
    if (r2_Left >= r1_Left && r2_Left <= r1_Right && r2_Top >= r1_Top && r2_Top <= r1_Bot) rect = r1 | r2;
    else if (r2_Left >= r1_Left && r2_Left <= r1_Right && r2_Bot >= r1_Top && r2_Bot <= r1_Bot) rect = r1 | r2;
    else if (r2_Left >= r1_Left && r2_Left <= r1_Right && r1_Top >= r2_Top && r1_Top <= r2_Bot) rect = r1 | r2;
    else if (r2_Left >= r1_Left && r2_Left <= r1_Right && r1_Bot >= r2_Top && r1_Bot <= r2_Bot) rect = r1 | r2;
    else if (r1_Left >= r2_Left && r1_Left <= r2_Right && r1_Top >= r2_Top && r1_Top <= r2_Bot) rect = r1 | r2;
    else if (r1_Left >= r2_Left && r1_Left <= r2_Right && r1_Bot >= r2_Top && r1_Bot <= r2_Bot) rect = r1 | r2;
    else if (r1_Left >= r2_Left && r1_Left <= r2_Right && r2_Top >= r1_Top && r2_Top <= r1_Bot) rect = r1 | r2;
    else if (r1_Left >= r2_Left && r1_Left <= r2_Right && r2_Bot >= r1_Top && r2_Bot <= r1_Bot) rect = r1 | r2;
    else if (r2_Right >= r1_Left && r2_Right <= r1_Right && r2_Top >= r1_Top && r2_Top <= r1_Bot) rect = r1 | r2;
    else if (r2_Right >= r1_Left && r2_Right <= r1_Right && r2_Bot >= r1_Top && r2_Bot <= r1_Bot) rect = r1 | r2;
    else if (r2_Right >= r1_Left && r2_Right <= r1_Right && r1_Top >= r2_Top && r1_Top <= r2_Bot) rect = r1 | r2;
    else if (r2_Right >= r1_Left && r2_Right <= r1_Right && r1_Bot >= r2_Top && r1_Bot <= r2_Bot) rect = r1 | r2;
    else if (r1_Right >= r2_Left && r1_Right <= r2_Right && r1_Top >= r2_Top && r1_Top <= r2_Bot) rect = r1 | r2;
    else if (r1_Right >= r2_Left && r1_Right <= r2_Right && r1_Bot >= r2_Top && r1_Bot <= r2_Bot) rect = r1 | r2;
    else if (r1_Right >= r2_Left && r1_Right <= r2_Right && r2_Top >= r1_Top && r2_Top <= r1_Bot) rect = r1 | r2;
    else if (r1_Right >= r2_Left && r1_Right <= r2_Right && r2_Bot >= r1_Top && r2_Bot <= r1_Bot) rect = r1 | r2;
    return rect;
}

}