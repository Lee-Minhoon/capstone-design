#include <opencv2/opencv.hpp>
#include <android/log.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "", __VA_ARGS__)

using namespace cv;
using namespace std;

#ifndef PRACTICEOPENCV_CUSTOMLIBRARY_H
#define PRACTICEOPENCV_CUSTOMLIBRARY_H
#ifdef __cplusplus
extern "C" {
#endif

const char* ItoLog(int v);
const char* DtoLog(double v);

typedef struct _RectInfo {
    int lineNumber;
    Rect rect;
} RectInfo;
bool cmp(const RectInfo &v1, const RectInfo &v2);

int rInt(double v);
int weighted(double v);
double weightedEx(double v);

Rect mergeRect(Rect r1, Rect r2);

#ifdef __cplusplus
}
#endif
#endif