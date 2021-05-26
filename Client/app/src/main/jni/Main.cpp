// ============================================================================================
// Last Updated 2021. 05. 25 By Team Guru
// ============================================================================================
#include "com_example_practiceopencv_CreateFragment.h"
#include "com_example_practiceopencv_MainActivity.h"
#include "com_example_practiceopencv_PracticeActivity.h"
#include "com_example_practiceopencv_TestActivity.h"
#include "customLibrary.h"

extern "C" {

// ============================================================================================
// 전처리 과정 1 - 악보 영상 그레이스케일 및 이진화 진행
// ============================================================================================
JNIEXPORT void JNICALL Java_com_example_practiceopencv_SheetFragment_preprocess1(
        JNIEnv *env,
        jobject thiz,
        jlong sheetMusicAddr) {

    // 로그 태그 및 악보 영상 레퍼런스 변수 생성
    const char* LOGTAG = "preprocess1";
    Mat &sheetMusic = *(Mat *) sheetMusicAddr;

    // 그레이스케일 (RGB → GRAY)
    cvtColor(sheetMusic, sheetMusic, COLOR_RGB2GRAY);

    // 이진화 (흑백 거꾸로, OTSU 알고리즘 사용)
    threshold(sheetMusic, sheetMusic, 127, 255, THRESH_BINARY_INV | THRESH_OTSU);

    // 로그
    LOGI("Function End :: %s", LOGTAG);
}

// ============================================================================================
// 전처리 과정 2 - 오선 영역 도출 및 그 외 영역 제거
// ============================================================================================
JNIEXPORT void JNICALL Java_com_example_practiceopencv_SheetFragment_preprocess2(
        JNIEnv *env,
        jobject thiz,
        jlong sheetMusicAddr) {

    // 로그 태그 및 악보 영상 레퍼런스 변수 생성
    const char* LOGTAG = "preprocess2";
    Mat &sheetMusic = *(Mat *) sheetMusicAddr;

    // 악보 영상 행, 열 길이
    int rows = sheetMusic.rows, cols = sheetMusic.cols;

    // 윤곽선 검출(findContours) 함수로 모든 객체를 찾음(저장은 contorus 벡터에)
    vector<vector<Point>> contours;
    findContours(sheetMusic, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);

    // 악보 영상을 3채널로 변환 후
    cvtColor(sheetMusic, sheetMusic, COLOR_GRAY2RGB);

    // 악보 영상 행 길이의 70%보다 가로로 긴 contours들에 대해 빨간색 박스를 침
    for (int i = 0; i < contours.size(); i++) {
        Rect rect = boundingRect(contours[i]);
        if(rect.width >= cols * 0.7 && rect.width != cols) {
            rectangle(sheetMusic, rect, Scalar(255, 0, 0), 3);
        }
    }

    // 오선 영역 외 데이터 제거
    uchar* pixelData = sheetMusic.data;
    // 영상의 각 행을 탐색하며..
    for (int y = 0; y < rows; y++) {
        double histogram = 0;
        // 빨간색이 포함되어 있는지 검사함
        for (int x = 0; x < cols; x++) {
            uchar rPixel = pixelData[y * cols * 3 + x * 3];
            uchar gPixel = pixelData[y * cols * 3 + x * 3 + 1];
            uchar bPixel = pixelData[y * cols * 3 + x * 3 + 2];
            if (rPixel > 200 && gPixel < 50 && bPixel < 50) {
                histogram++;
                for (int i = 0; i < 3; i++) {
                    pixelData[y * cols * 3 + x * 3 + i] = 0;
                }
            }
        }
        // 빨간색이 포함되어 있지 않은 행은 다 날려버림
        if (histogram == 0) {
            for (int x = 0; x < cols; x++) {
                for (int i = 0; i < 3; i++) {
                    pixelData[y * cols * 3 + x * 3 + i] = 0;
                }
            }
        }
    }

    LOGI("Function End :: %s", LOGTAG);
}

// ============================================================================================
// 전처리 과정 3 - 수평 히스토그램을 사용해 오선을 제거함
// ============================================================================================
JNIEXPORT jintArray JNICALL Java_com_example_practiceopencv_SheetFragment_preprocess3(
        JNIEnv *env,
        jobject thiz,
        jlong sheetMusicAddr) {

    // 로그 태그 및 악보 영상 레퍼런스 변수 생성
    const char* LOGTAG = "preprocess3";
    Mat &sheetMusic = *(Mat *) sheetMusicAddr;

    // 악보 영상 행, 열 길이
    int rows = sheetMusic.rows, cols = sheetMusic.cols;

    // 그레이스케일 및 이진화
    cvtColor(sheetMusic, sheetMusic, COLOR_RGB2GRAY);
    threshold(sheetMusic, sheetMusic, 127, 255, THRESH_BINARY_INV | THRESH_OTSU);

    // 오선 좌표와 길이를 담을 벡터를 생성
    vector<int> staffsLocation;
    vector<int> staffsHeight;
    uchar* pixelData = sheetMusic.data;
    double histogram = 0;
    // 영상의 각 행을 탐색하며..
    for (int y = 0; y < rows; y++) {
        // 검은색 픽셀 개수만큼 변수를 증가시키고
        for (int x = 0; x < cols; x++) {
            if (pixelData[y * cols + x] != 255) {
                histogram++;
            }
        }
        // 픽셀의 개수가 행 길이의 50% 이상이라면 오선으로 판단한다.
        if (histogram / cols > 0.5) {
            // 벡터에 오선 좌표가 없다면..
            if (staffsLocation.size() == 0) {
                staffsLocation.push_back(y);
                staffsHeight.push_back(0);
            }
            // 새로 발견된 오선 좌표가 이전 좌표와 1보다 크게 차이 나면 새로운 오선임
            else if (abs(staffsLocation.back() - y) > 1) {
                staffsLocation.push_back(y);
                staffsHeight.push_back(0);
            }
            // 아니라면 같은 오선으로 오선 좌표와 오선 길이를 업데이트함
            else {
                staffsLocation[staffsLocation.size() - 1] = y;
                staffsHeight[staffsHeight.size() - 1] += 1;
            }
            // 오선을 확대해보면 1픽셀로만 이루어진 게 아니므로 좌표와 길이를 각각 유지
        }
        histogram = 0;
    }

    // 오선 영역 제거
    const int SIZE = staffsLocation.size();
    pixelData = sheetMusic.data;
    for (int x = 0; x < cols; x++) {
        for (int i = 0; i < SIZE; i++) {
            // 오선 최하단 좌표에서 오선 길이를 빼면 최상단 좌표가 나옴
            int topPixel = staffsLocation[i] - staffsHeight[i];
            int botPixel = staffsLocation[i];
            // 최상단 좌표 위와 최하단 좌표 아래에 아무 픽셀이 없다면
            if (pixelData[(topPixel - 1) * cols + x] != 0 && pixelData[(botPixel + 1) * cols + x] != 0) {
                // 해당 좌표의 픽셀은 지운다.
                for (int j = 0; j < staffsHeight[i] + 1; j++) {
                    int removePixel = staffsLocation[i] - j;
                    pixelData[removePixel * cols + x] = 255;
                }
                // 위, 아래 좌표를 확인하지 않으면 음표나 쉼표 등의 객체들이 지워질 수 있으므로 픽셀을 확인 후 지움
            }
        }
    }

    // 오선의 좌표들을 리턴 해주기 위해 JintArray로 변환
    jintArray intArray = (*env).NewIntArray(SIZE);
    for (int i = 0; i < SIZE; i++) {
        jint element = staffsLocation[i];
        (*env).SetIntArrayRegion(intArray, i, 1, &element);
    }

    LOGI("Function End :: %s", LOGTAG);
    return intArray;
}

// ============================================================================================
// 전처리 과정 4 - 악보 영상에 가중치를 곱해줌(객체들이 항상 비슷한 크기를 갖게 하기 위함)
// ============================================================================================
JNIEXPORT jintArray JNICALL Java_com_example_practiceopencv_SheetFragment_preprocess4(
        JNIEnv *env,
        jobject thiz,
        jlong sheetMusicAddr,
        jintArray staffsInfoArray) {

    // 로그 태그 및 악보 영상 레퍼런스 변수 생성
    const char* LOGTAG = "preprocess4";
    Mat &sheetMusic = *(Mat *) sheetMusicAddr;

    // 해당 함수를 호출한 자바 클래스에서 avgDistance라는 변수를 참조함
    jclass cls = (*env).GetObjectClass(thiz);
    jfieldID fid = (*env).GetFieldID(cls, "avgDistance", "D");

    // 전처리 과정 3에서 반환했던 오선 좌표 정보를 불러옴
    const int arrayLength = (*env).GetArrayLength(staffsInfoArray);
    jint staffsInfo[arrayLength];
    (*env).GetIntArrayRegion(staffsInfoArray, 0, arrayLength, staffsInfo);

    // 악보 영상 행, 열 길이
    int rows = sheetMusic.rows, cols = sheetMusic.cols;

    // 오선 좌표들 간의 거리를 모두 더한 후 나눠 평균을 구함
    double avgDistance = 0.00;
    for (int i = 0; i < arrayLength / 5; i++) {
        for (int j = 0; j < 4; j++) {
            avgDistance += abs(staffsInfo[i * 5 + j] - staffsInfo[i * 5 + j + 1]);
        }
    }
    avgDistance /= (arrayLength - arrayLength / 5);

    // 가중치를 구해 영상에 곱해줌(오선 간격은 항상 20픽셀이 되도록)
    const double weightStd = 20.0;
    double weight = weightStd / avgDistance;
    double newWidth = sheetMusic.cols * weight;
    double newHeight = sheetMusic.rows * weight;
    LOGI("%s :: weight : %s", LOGTAG, to_string(weight).data());
    LOGI("%s :: Original width : %s, Original height : %s", LOGTAG, to_string(cols).data(), to_string(rows).data());
    LOGI("%s :: Weighted width : %s, Weighted height : %s", LOGTAG, to_string(newWidth).data(), to_string(newHeight).data());
    resize(sheetMusic, sheetMusic, Size(newWidth, newHeight));

    // 오선 평균 거리와 오선 좌표 정보도 업데이트해 줌
    LOGI("%s :: Original avgDistance : %s", LOGTAG, to_string(avgDistance).data());
    avgDistance *= weight;
    LOGI("%s :: Weighted avgDistance : %s", LOGTAG, to_string(avgDistance).data());
    for (int i = 0; i < arrayLength; i++) {
        staffsInfo[i] *= weight;
    }

    // 업데이트된 오선 정보를 리턴 해주기 위해 JintArray로 변환
    jintArray intArray = (*env).NewIntArray(arrayLength);
    for (int i = 0; i < arrayLength; i++) {
        jint element = staffsInfo[i];
        (*env).SetIntArrayRegion(intArray, i, 1, &element);
    }

    LOGI("Function End :: %s", LOGTAG);
    (*env).SetDoubleField(thiz, fid, avgDistance);
    return intArray;
};

// ============================================================================================
// process
// ============================================================================================
JNIEXPORT jobjectArray JNICALL Java_com_example_practiceopencv_SheetFragment_process(
        JNIEnv *env,
        jobject thiz,
        jlong sheetMusicAddr,
        jintArray staffsInfoArray) {

    // 로그 태그 및 악보 영상 레퍼런스 변수 생성
    const char* LOGTAG = "process";
    Mat &sheetMusic = *(Mat *) sheetMusicAddr;

    // 해당 함수를 호출한 자바 클래스에서 key, avgDistance라는 변수를 참조함
    jclass cls = (*env).GetObjectClass(thiz);
    jfieldID key = (*env).GetFieldID(cls, "key", "I");
    jfieldID fid = (*env).GetFieldID(cls, "avgDistance", "D");
    jdouble avgDistance = (*env).GetDoubleField(thiz, fid);

    // 오선의 정보를 불러옴
    const int arrayLength = (*env).GetArrayLength(staffsInfoArray);
    jint staffsInfo[arrayLength];
    (*env).GetIntArrayRegion(staffsInfoArray, 0, arrayLength, staffsInfo);

    // 악보 영상 행, 열 길이
    int rows = sheetMusic.rows, cols = sheetMusic.cols;
    int keyTemp = 0;
    uchar* pixelData = sheetMusic.data;

    // 악보 영상 내 모든 객체를 찾음
    vector<vector<Point>> contours;
    findContours(sheetMusic, contours, RETR_LIST, CHAIN_APPROX_NONE);
    vector<RectInfo> objectInfo;
    RectInfo rectInfo;
    for (int i = 0; i < contours.size(); i++) {
        Rect rect = boundingRect(contours[i]);
        // j는 몇 번째 오선 영역인지를 나타냄
        for (int j = 0; j < arrayLength / 5; j++) {
            // 객체의 중간 y 좌표
            double yCenter = (rect.y + rect.y + rect.height) / 2;
            double errorW = rect.width / cols;
            double errorH = rect.height / rows;
            double topPixel, botPixel;
            // 객체의 가로, 세로 길이가 음표 조건에 맞는지 검사
            if ((rect.width >= weighted(20) && rect.height >= weighted(50)) || (rect.width >= weighted(20) && rect.height <= weighted(25) && rect.height >= weighted(15))) {
                topPixel = staffsInfo[j * 5] - weighted(40);
                botPixel = staffsInfo[(j + 1) * 5 - 1] + weighted(40);
            }
            // 아니라면 쉼표로 일단 판단
            else {
                topPixel = staffsInfo[j * 5] - weighted(10);
                botPixel = staffsInfo[(j + 1) * 5 - 1] + weighted(10);
            }
            // 오선 영역 안에 들어오고 일정 크기보다 작아야 음표, 쉼표로 판단
            if (rect.width >= weighted(3) && yCenter >=  topPixel && yCenter <= botPixel && errorW < 0.3 && errorH < 0.3) {
                rectInfo = {j, rect};
                objectInfo.push_back(rectInfo);
            }
        }
    }

    // 정렬 조건은 몇 번째 오선 영역에 포함 돼 있는지, 이후 왼쪽에서 오른쪽으로
    sort(objectInfo.begin(), objectInfo.end(), cmp);
    LOGI("%s :: objectInfo.size : %s", LOGTAG, to_string(objectInfo.size()).data());

    // 분할된 객체들을 합침
    for (int i = 0; i < objectInfo.size(); i++) {
        for (int j = 0; j < objectInfo.size(); j++) {
            Rect r1 = objectInfo[i].rect;
            Rect r2 = objectInfo[j].rect;
            objectInfo[i].rect = mergeRect(r1, r2);
        }
    }
    for (int i = 0; i < objectInfo.size(); i++) {
        for (int j = 0; j < objectInfo.size(); j++) {
            Rect r1 = objectInfo[i].rect;
            Rect r2 = objectInfo[j].rect;
            objectInfo[i].rect = mergeRect(r1, r2);
        }
    }

    // 중복 제거
    vector<RectInfo> objectInfoEx;
    int overlap = 0;
    for (int i = 0; i < objectInfo.size(); i++) {
        if (objectInfo[i].rect.width >= weighted(15) && objectInfo[i].rect.height >= weighted(10)) {
            if (objectInfoEx.size() == 0) {
                objectInfoEx.push_back(objectInfo[i]);
            } else {
                for (int j = 0; j < objectInfoEx.size(); j++) {
                    if (objectInfo[i].rect == objectInfoEx[j].rect) {
                        overlap++;
                    }
                }
                if (overlap == 0) {
                    objectInfoEx.push_back(objectInfo[i]);
                }
                overlap = 0;
            }
        }
    }
    int SIZE = objectInfoEx.size();
    LOGI("%s :: objectInfoEx.size : %s", LOGTAG, to_string(SIZE).data());

//    cvtColor(sheetMusic, sheetMusic, COLOR_GRAY2RGB);
//    for(int i = 0; i < SIZE; i++) {
//        rectangle(sheetMusic, objectInfoEx[i].rect, Scalar(255, 0, 0), 3);
//    }

    // 객체에서 얻을 수 있는 특징점들을 확보
    typedef struct _NoteInfo {
        int lineNumber;
        Rect rect;
        vector<int> tailLoc;
        vector<int> tailStart;
        vector<int> tailEnd;
        vector<int> tailWidth;
        int headWay;
    } NoteInfo;
    NoteInfo noteInfo;
    vector<NoteInfo> notesInfo;
    for (int i = 0; i < SIZE; i++) {
        int xLoc = objectInfoEx[i].rect.x;
        int yLoc = objectInfoEx[i].rect.y;
        int width = objectInfoEx[i].rect.width;
        int height = objectInfoEx[i].rect.height;
        vector<int> tailLoc, tailStart, tailEnd, tailWidth;
        int headWay = 0;
        for (int x = xLoc; x < xLoc + width; x++) {
            int histogram = 0;
            int tailEndTemp = 0;
            for (int y = yLoc; y < yLoc + height; y++) {
                if (pixelData[y * cols + x] != 255) {
                    if (histogram >= 5) {
                        if (pixelData[(y + 1) * cols + x] != 0) {
                            tailEndTemp = y;
                            if (histogram < 50) {
                                histogram = 0;
                            } else {
                                break;
                            }
                        }
                    }
                    histogram++;
                }
            }
            if (histogram > weighted(50)) {
                int tailStartTemp = tailEndTemp - histogram;
                if (x <= xLoc + weighted(10)) {
                    headWay = 1;
                }
                if (tailLoc.size() == 0) {
                    tailLoc.push_back(x);
                    tailStart.push_back(tailStartTemp);
                    tailEnd.push_back(tailEndTemp);
                    tailWidth.push_back(0);
                } else if (abs(tailLoc.back() - x) > 1) {
                    tailLoc.push_back(x);
                    tailStart.push_back(tailStartTemp);
                    tailEnd.push_back(tailEndTemp);
                    tailWidth.push_back(0);
                } else {
                    tailLoc[tailLoc.size() - 1] = x;
                    tailWidth[tailWidth.size() - 1]++;
                }
            }
        }
        noteInfo = {objectInfoEx[i].lineNumber, objectInfoEx[i].rect, tailLoc, tailStart, tailEnd, tailWidth, headWay};
        notesInfo.push_back(noteInfo);
    }
    SIZE = notesInfo.size();
    LOGI("%s :: notesInfo.size : %s", LOGTAG, to_string(notesInfo.size()).data());

    // 인식 프로세스
    vector<int> beats;
    vector<int> notes;
    pixelData = sheetMusic.data;
    for (int i = 0; i < SIZE; i++) {
        int lineNumber = notesInfo[i].lineNumber;
        int staffsLoc[5] = {
                staffsInfo[(lineNumber * 5) + 0],
                staffsInfo[(lineNumber * 5) + 1],
                staffsInfo[(lineNumber * 5) + 2],
                staffsInfo[(lineNumber * 5) + 3],
                staffsInfo[(lineNumber * 5) + 4]
        };
        int width = notesInfo[i].rect.width;
        int height = notesInfo[i].rect.height;
        int left = notesInfo[i].rect.x;
        int top = notesInfo[i].rect.y;
        int right = left + width;
        int bot = top + height;
        double xCenter = left + width * 0.5;
        double yCenter = top + height * 0.5;
        int headWay = notesInfo[i].headWay;
        int tailCnt = notesInfo[i].tailLoc.size();
        int value = 0;

        // 키 인식
        int keyAreaTop[2] = {staffsLoc[0] - weighted(5), staffsLoc[0] + weighted(5)};
        int keyAreaBot[2] = {staffsLoc[4] - weighted(5), staffsLoc[4] + weighted(5)};
        int keyCenterTop = staffsLoc[2] - weighted(5);
        int keyCenterBot = staffsLoc[2] + weighted(5);
        if (i == 1) {
            if (top >= keyAreaTop[0] && top <= keyAreaTop[1] && bot >= keyAreaBot[0] && bot <= keyAreaBot[1] && yCenter >= keyCenterTop && yCenter <= keyCenterBot) {
                if (width >= weighted(20) && width <= weighted(35) && height >= weighted(75) && height <= weighted(90)) {
                    int pixelValue = 0;
                    for (int x = left; x < right; x++) {
                        for (int y = top; y < bot; y++) {
                            if (pixelData[y * cols + x] != 255) {
                                pixelValue++;
                            }
                        }
                    }
                    if (pixelValue >= weighted(800)) {
                        Point point = {left, top - weighted(20)};
                        putText(sheetMusic, "T", point, 2, weightedEx(1.5), Scalar(0, 0, 0));
                    }
                }
            } else {
                vector<int> tailCnt;
                int tailStart;
                int tailEnd;
                for (int x = left; x < right; x++) {
                    int histogram = 0;
                    int tailEndTemp = 0;
                    for (int y = top; y < bot; y++) {
                        if (pixelData[y * cols + x] != 255) {
                            if (histogram >= 5) {
                                if (pixelData[(y + 1) * cols + x] != 0) {
                                    tailEndTemp = y;
                                    if (histogram < 30) {
                                        histogram = 0;
                                    } else {
                                        break;
                                    }
                                }
                            }
                            histogram++;
                        }
                    }
                    if (histogram > weighted(30)) {
                        int tailStartTemp = tailEndTemp - histogram;
                        if (tailCnt.size() == 0) {
                            tailCnt.push_back(x);
                            tailStart = tailStartTemp;
                            tailEnd = tailEndTemp;
                        } else if (abs(tailCnt.back() - x) > 1) {
                            tailCnt.push_back(x);
                        } else {
                            tailCnt[tailCnt.size() - 1] = x;
                        }
                    }
                }
                LOGI("%s :: i : %s, tailCnt : %s, tailStart : %s, tailEnd : %s", LOGTAG, to_string(i).data(), to_string(tailCnt.size()).data(), to_string(tailStart).data(), to_string(tailEnd).data());
                int keyCnt;
                int keyAreaTop[2] = {staffsLoc[0] - weighted(10), staffsLoc[0] + weighted(10)};
                int keyAreaBot[2] = {staffsLoc[2], staffsLoc[3]};
                Point point = {left, top - weighted(20)};
                if (tailStart >= keyAreaTop[0] && tailStart <= keyAreaTop[1] && tailEnd >= keyAreaBot[0] && tailEnd <= keyAreaBot[1]) {
                    keyCnt = tailCnt.size();
                    putText(sheetMusic, "b" + to_string(keyCnt), point, 2, weightedEx(1.5), Scalar(0, 0, 0));
                    value = 1;
                    keyTemp = 10 + keyCnt;
                } else {
                    keyCnt = tailCnt.size() / 2;
                    putText(sheetMusic, "#" + to_string(keyCnt), point, 2, weightedEx(1.5), Scalar(0, 0, 0));
                    value = 1;
                    keyTemp = 20 + keyCnt;
                }
            }
        }

        // 음표 인식
        LOGI("%s :: i : %s, tailCnt : %s, headWay : %s", LOGTAG, to_string(i).data(), to_string(tailCnt).data(), to_string(headWay).data());
        if (value == 0 && width >= weighted(20) && height >= weighted(50)){
            for (int j = 0; j < tailCnt; j++) {
                // 음표의 위치 파악
                int noteAreaTop, noteAreaBot;
                if (headWay == 0) {
                    noteAreaTop = notesInfo[i].tailEnd[j] - weighted(20);
                    noteAreaBot = notesInfo[i].tailEnd[j] + weighted(20);
                } else {
                    noteAreaTop = notesInfo[i].tailStart[j] - weighted(20);
                    noteAreaBot = notesInfo[i].tailStart[j] + weighted(20);
                }
                vector<int> noteVec;
                vector<int> histoVec;
                vector<int> histoExVec;
                for (int y = noteAreaTop; y < noteAreaBot; y++) {
                    int histo = 0, histoEx = 0, noteLeft, noteRight;
                    if (headWay == 0) {
                        noteRight = notesInfo[i].tailLoc[j] - notesInfo[i].tailWidth[j];
                        noteLeft = noteRight - weighted(20);
                    } else {
                        noteLeft = notesInfo[i].tailLoc[j] + 1;
                        noteRight = noteLeft + weighted(20) + 1;
                    }
                    for (int x = noteLeft; x < noteRight; x++) {
                        if (pixelData[y * cols + x] != 255) {
                            histo++;
                        }
                    }
                    if (histo >= 10) {
                        noteVec.push_back(y);
                        histoVec.push_back(histo);
                    }
                    for (int x = noteLeft; x < noteRight; x++) {
                        if (pixelData[y * cols + x] != 255) {
                            if (histoEx >= 5) {
                                if (pixelData[y * cols + x + 1] != 0) {
                                    break;
                                }
                            }
                            histoEx++;
                        }
                    }
                    if (histoEx >= 10) {
                        histoExVec.push_back(histoEx);
                    }
                }
                int total = 0, avg = 0, noteLoc = 0;
                for (int i = 0; i < histoVec.size(); i++) {
                    total += noteVec[i];
                    avg++;
                }
                if (avg >= 15) {
                    // 꼬리 개수 파악
                    noteLoc = rInt(total / avg);
                    int wingAreaTop, wingAreaBot;
                    if (headWay == 0) {
                        wingAreaTop = notesInfo[i].tailStart[j];
                        wingAreaBot = noteLoc - weighted(20);
                    } else {
                        wingAreaTop = noteLoc + weighted(20);
                        wingAreaBot = notesInfo[i].tailEnd[j];
                    }
                    int wings = 0, wideX = 0;
                    if (j == 0) wideX = notesInfo[i].tailLoc[j] + weighted(7);
                    else wideX = notesInfo[i].tailLoc[j] - notesInfo[i].tailWidth[j] - weighted(7);
                    for (int y = wingAreaTop; y < wingAreaBot; y++) {
                        if (pixelData[y * cols + wideX] != 255) {
                            wings++;
                        }
                    }
                    // 점음표인지 탐색
                    int pointAreaTop = noteLoc - weighted(10);
                    int pointAreaBot = noteLoc + weighted(10);
                    int points = 0;
                    int pointAreaLeft, pointAreaRight;
                    if (headWay == 0) {
                        pointAreaLeft = notesInfo[i].tailLoc[j] + weighted(5);
                        pointAreaRight = notesInfo[i].tailLoc[j] + weighted(15);
                    } else {
                        pointAreaLeft = notesInfo[i].tailLoc[j] + weighted(29);
                        pointAreaRight = notesInfo[i].tailLoc[j] + weighted(39);
                    }
                    // rectangle(sheetMusic, Point(pointAreaLeft, pointAreaTop), Point(pointAreaRight, pointAreaBot), Scalar(0), 3);
                    for (int y = pointAreaTop; y < pointAreaBot; y++) {
                        for (int x = pointAreaLeft; x < pointAreaRight; x++) {
                            if (pixelData[y * cols + x] != 255) {
                                points++;
                            }
                        }
                    }
                    // 몇 분 음표인지 판단
                    int histogram = *max_element(histoVec.begin(), histoVec.end());
                    int histogramEx = *max_element(histoExVec.begin(), histoExVec.end());
                    int beat = 0;
                    double std = weighted(18);
                    int staffsLocTop = staffsLoc[0] - 15;
                    int staffsLocBot = staffsLoc[4] + 15;
                    int pointStd;
                    if (noteLoc >= staffsLocTop && noteLoc <= staffsLocBot) pointStd = weighted(15);
                    else pointStd = weighted(60);
                    if (histogramEx >= std && histoExVec.size() >= weighted(15) && wings == weighted(0) && points < pointStd) beat = 4;
                    else if (histogramEx >= std && histoExVec.size() >= weighted(15) && wings == weighted(0) && points >= pointStd) beat = -4;
                    else if (histogramEx >= std && histoExVec.size() >= weighted(15) && wings >= weighted(5) && wings < weighted(17) && points < pointStd) beat = 8;
                    else if (histogramEx >= std && histoExVec.size() >= weighted(15) && wings >= weighted(5) && wings < weighted(17) && points >= pointStd) beat = -8;
                    else if (histogramEx >= std && histoExVec.size() >= weighted(15) && wings >= weighted(17) && points < pointStd) beat = 16;
                    else if (histogramEx >= std && histoExVec.size() >= weighted(15) && wings >= weighted(17) && points >= pointStd) beat = -16;
                    else if (histoExVec.size() < weighted(15) && histogram >= weighted(10) && wings < weighted(5) && points < pointStd) beat = 2;
                    else if (histoExVec.size() < weighted(15) && histogram >= weighted(10) && wings < weighted(5) && points >= pointStd) beat = -2;
                    if (beat != 0) {
                        LOGI("%s :: i : %s, j : %s, histoExVecSize : %s, wings : %s, points : %s", LOGTAG, to_string(i).data(), to_string(j).data(), to_string(histoExVec.size()).data(), to_string(wings).data(), to_string(points).data());
                        Point point = {notesInfo[i].tailLoc[j] - weighted(20), top - weighted(20)};
                        putText(sheetMusic, to_string(beat), point, 2, weightedEx(1.5), Scalar(0, 0, 0));
                        notes.push_back(noteLoc);
                        beats.push_back(beat);
                        value = 1;
                    }
                }
            }
        }

        // 쉼표 인식
        int restAreaTop[2] = {staffsLoc[1], staffsLoc[1] + 10};
        int restAreaBot[2] = {staffsLoc[1] + 10, staffsLoc[2]};
        int restCenterTop = staffsLoc[1];
        int restCenterBot = staffsLoc[3];
        if (value == 0 && yCenter >= restCenterTop && yCenter <= restCenterBot && top >= staffsLoc[0] && bot <= staffsLoc[4]) {
            if (width >= weighted(25) && width <= weighted(30) && height >= weighted(14) && height <= weighted(18)) {
                int pixelValue = 0;
                for (int x = left; x < right; x++) {
                    for (int y = top; y < bot; y++) {
                        if (pixelData[y * cols + x] != 255) {
                            pixelValue++;
                        }
                    }
                }
                LOGI("%s :: i : %s, pixelValue : %s", LOGTAG, to_string(i).data(), to_string(pixelValue).data());
                int restValue = 0;
                if (pixelValue >= weighted(370) && pixelValue <= weighted(470)) {
                    if (yCenter >= restAreaTop[0] && yCenter <= restAreaBot[0]) restValue = 1;
                    else if (yCenter >= restAreaTop[1] && yCenter <= restAreaBot[1]) restValue = 2;
                    Point point = {left, top - weighted(20)};
                    putText(sheetMusic, "R" + to_string(restValue), point, 2, weightedEx(1.5), Scalar(0, 0, 0));
                    notes.push_back(-1);
                    beats.push_back(restValue);
                    value = 1;
                }
            } else if (width >= weighted(20) && width <= weighted(25) && height >= weighted(50) && height <= weighted(65)) {
                int pixelValue = 0;
                for (int x = left; x < right; x++) {
                    for (int y = top; y < bot; y++) {
                        if (pixelData[y * cols + x] != 255) {
                            pixelValue++;
                        }
                    }
                }
                LOGI("%s :: i : %s, pixelValue : %s", LOGTAG, to_string(i).data(), to_string(pixelValue).data());
                if (pixelValue >= weighted(550) && pixelValue <= weighted(800)) {
                    Point point = {left, top - weighted(20)};
                    putText(sheetMusic, "R" + to_string(4), point, 2, weightedEx(1.5), Scalar(0, 0, 0));
                    notes.push_back(-1);
                    beats.push_back(4);
                    value = 1;
                }
            } else if (width >= weighted(20) && width <= weighted(25) && height >= weighted(36) && height <= weighted(46)) {
                int pixelValue = 0;
                for (int x = left; x < right; x++) {
                    for (int y = top; y < bot; y++) {
                        if (pixelData[y * cols + x] != 255) {
                            pixelValue++;
                        }
                    }
                }
                LOGI("%s :: i : %s, pixelValue : %s", LOGTAG, to_string(i).data(), to_string(pixelValue).data());
                if (pixelValue >= weighted(250) && pixelValue <= weighted(400)) {
                    Point point = {left, top - weighted(20)};
                    putText(sheetMusic, "R" + to_string(8), point, 2, weightedEx(1.5), Scalar(0, 0, 0));
                    notes.push_back(-1);
                    beats.push_back(8);
                    value = 1;
                }
            }
        }
    }
    cvtColor(sheetMusic, sheetMusic, COLOR_GRAY2RGB);
    for (int i = 0; i < notesInfo.size(); i++) {
        int x;
        if (i < 10) {
            x = notesInfo[i].rect.x;
        } else {
            x = notesInfo[i].rect.x - weighted(10);
        }
        int y = notesInfo[i].rect.y + notesInfo[i].rect.height + weighted(60);
        putText(sheetMusic, to_string(i), Point(x, y), 2, weightedEx(1.5), Scalar(0, 0, 0));
        rectangle(sheetMusic, notesInfo[i].rect, Scalar(255, 0, 0), 3);
        LOGI("%s :: i : %s, width : %s, height : %s", LOGTAG, to_string(i).data(), to_string(notesInfo[i].rect.width).data(), to_string(notesInfo[i].rect.height).data());
    }

    // 음의 높낮이 파악
    int totalStaffs = arrayLength / 5;
    for (int i = 0; i < notes.size(); i++) {
        for (int j = 0; j < totalStaffs; j++) {
            int Lines[21] = {
                    staffsInfo[(j * 5) + 4] + weighted(20) * 3,
                    staffsInfo[(j * 5) + 4] + weighted(20) * 2 + weighted(10), staffsInfo[(j * 5) + 4] + weighted(20) * 2,
                    staffsInfo[(j * 5) + 4] + weighted(20) * 1 + weighted(10), staffsInfo[(j * 5) + 4] + weighted(20) * 1,
                    staffsInfo[(j * 5) + 4] + weighted(10), staffsInfo[(j * 5) + 4],
                    staffsInfo[(j * 5) + 3] + weighted(10), staffsInfo[(j * 5) + 3],
                    staffsInfo[(j * 5) + 2] + weighted(10), staffsInfo[(j * 5) + 2],
                    staffsInfo[(j * 5) + 1] + weighted(10), staffsInfo[(j * 5) + 1],
                    staffsInfo[(j * 5) + 0] + weighted(10), staffsInfo[(j * 5) + 0],
                    staffsInfo[(j * 5) + 0] - weighted(20) * 1 + weighted(10), staffsInfo[(j * 5) + 0] - weighted(20) * 1,
                    staffsInfo[(j * 5) + 0] - weighted(20) * 2 + weighted(10), staffsInfo[(j * 5) + 0] - weighted(20) * 2,
                    staffsInfo[(j * 5) + 0] - weighted(20) * 3 + weighted(10), staffsInfo[(j * 5) + 0] - weighted(20) * 3
            };
            int include = weighted(7);
            if (notes[i] != -1) {
                for (int k = 0; k < 21; k++) {
                    int topLoc = Lines[k] - include;
                    int botLoc = Lines[k] + include;
                    if (notes[i] > topLoc && notes[i] < botLoc) {
                        notes[i] = k;
                    }
                }
            }
        }
    }

    for (int i = 0; i < notes.size(); i++) {
        LOGI("%s :: i : %s, notes : %s, beats : %s", LOGTAG, to_string(i).data(), to_string(notes[i]).data(), to_string(beats[i]).data());
    }

    // 음표에 대한 정보를 반환하기 위해 변환
    jclass intArrayCls = (*env).FindClass("[I");
    jintArray intArray = (*env).NewIntArray(2);
    jobjectArray objArray = (*env).NewObjectArray(notes.size(), intArrayCls, intArray);
    for (int i = 0; i < notes.size(); i++) {
        jintArray elementArray = (*env).NewIntArray(2);
        jint element[] = {notes[i], beats[i]};
        (*env).SetIntArrayRegion(elementArray, 0, 2, element);
        (*env).SetObjectArrayElement(objArray, i, elementArray);
    }

    LOGI("Function End :: %s", LOGTAG);
    (*env).SetIntField(thiz, key, keyTemp);
    return objArray;
}

// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// 아래로는 연습했던 코드
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================

// ============================================================================================
// preprocess1 - Convert Three Channel Image to Binary Image & Invert Color
// ============================================================================================
JNIEXPORT void JNICALL Java_com_example_practiceopencv_PracticeActivity_preprocess1(
        JNIEnv *env,
        jobject thiz,
        jlong sheetMusicAddr) {

    // Create Variable LOGTAG
    const char* LOGTAG = "preprocess1";

    // Create Reference Variable Sheet Music
    Mat &sheetMusic = *(Mat *) sheetMusicAddr;

    // Grayscale & Thresholding
    cvtColor(sheetMusic, sheetMusic, COLOR_RGB2GRAY);
    threshold(sheetMusic, sheetMusic, 127, 255, THRESH_BINARY_INV | THRESH_OTSU);

    LOGI("Function End :: %s", LOGTAG);
}

// ============================================================================================
// preprocess2 - Find the Staff Area and Clear other areas
// ============================================================================================
JNIEXPORT void JNICALL Java_com_example_practiceopencv_PracticeActivity_preprocess2(
        JNIEnv *env,
        jobject thiz,
        jlong sheetMusicAddr) {

    // Create Variable LOGTAG
    const char* LOGTAG = "preprocess2";

    // Create Reference Variable Sheet Music
    Mat &sheetMusic = *(Mat *) sheetMusicAddr;

    // Create Variable Sheet Music Data
    int rows = sheetMusic.rows, cols = sheetMusic.cols;
    uchar* pixelData;

    // Get Staff Area
    vector<vector<Point>> contours;
    findContours(sheetMusic, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);
    cvtColor(sheetMusic, sheetMusic, COLOR_GRAY2RGB);
    for (int i = 0; i < contours.size(); i++) {
        Rect rect = boundingRect(contours[i]);
        if(rect.width >= cols * 0.7) {
            rectangle(sheetMusic, rect, Scalar(255, 0, 0), 3);
        }
    }

    // Clear Out of Staffs Area
    pixelData = sheetMusic.data;
    for (int y = 0; y < rows; y++) {
        double histogram = 0;
        for (int x = 0; x < cols; x++) {
            uchar rPixel = pixelData[y * cols * 3 + x * 3];
            uchar gPixel = pixelData[y * cols * 3 + x * 3 + 1];
            uchar bPixel = pixelData[y * cols * 3 + x * 3 + 2];
            if (rPixel > 200 && gPixel < 50 && bPixel < 50) {
                histogram++;
                for (int i = 0; i < 3; i++) {
                    pixelData[y * cols * 3 + x * 3 + i] = 0;
                }
            }
        }
        if (histogram == 0) {
            for (int x = 0; x < cols; x++) {
                for (int i = 0; i < 3; i++) {
                    pixelData[y * cols * 3 + x * 3 + i] = 0;
                }
            }
        }
    }

    LOGI("Function End :: %s", LOGTAG);
}

// ============================================================================================
// preprocess3 - Clear the Staff through the Vertical Histogram
// ============================================================================================
JNIEXPORT jintArray JNICALL Java_com_example_practiceopencv_PracticeActivity_preprocess3(
        JNIEnv *env,
        jobject thiz,
        jlong sheetMusicAddr) {

    // Create Variable LOGTAG
    const char* LOGTAG = "preprocess3";

    // Create Reference Variable Sheet Music
    Mat &sheetMusic = *(Mat *) sheetMusicAddr;

    // Create Variable Sheet Music Data
    int rows = sheetMusic.rows, cols = sheetMusic.cols;
    uchar* pixelData;
    double histogram = 0;

    // Grayscale & Thresholding
    cvtColor(sheetMusic, sheetMusic, COLOR_RGB2GRAY);
    threshold(sheetMusic, sheetMusic, 127, 255, THRESH_BINARY | THRESH_OTSU);

    // Histogram & Get Information
    pixelData = sheetMusic.data;
    vector<int> staffsLocation;
    vector<int> staffsHeight;
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            if (pixelData[y * cols + x] != 0) {
                pixelData[y * cols + x] = 0;
                histogram++;
            } else {
                pixelData[y * cols + x] = 255;
            }
        }
        if (histogram / cols > 0.5) {
            if (staffsLocation.size() == 0) {
                staffsLocation.push_back(y);
                staffsHeight.push_back(0);
            } else if (abs(staffsLocation.back() - y) > 1) {
                staffsLocation.push_back(y);
                staffsHeight.push_back(0);
            } else {
                staffsLocation[staffsLocation.size() - 1] = y;
                staffsHeight[staffsHeight.size() - 1] += 1;
            }
        }
        histogram = 0;
    }

    // Clear The Staffs
    const int SIZE = staffsLocation.size();
    pixelData = sheetMusic.data;
    for (int x = 0; x < cols; x++) {
        for (int i = 0; i < SIZE; i++) {
            int topPixel = staffsLocation[i] - staffsHeight[i];
            int botPixel = staffsLocation[i];
            if (pixelData[(topPixel - 1) * cols + x] != 0 && pixelData[(botPixel + 1) * cols + x] != 0) {
                for (int j = 0; j < staffsHeight[i] + 1; j++) {
                    int removePixel = staffsLocation[i] - j;
                    pixelData[removePixel * cols + x] = 255;
                }
            }
        }
    }

    // Convert Vector to intArray for Return
    jintArray intArray = (*env).NewIntArray(SIZE);
    for (int i = 0; i < SIZE; i++) {
        jint element = staffsLocation[i];
        (*env).SetIntArrayRegion(intArray, i, 1, &element);
    }

    // Return Staffs Location
    LOGI("Function End :: %s", LOGTAG);
    return intArray;
}

// ============================================================================================
// preprocess4 - Multiply Sheet Music Image and Data by Weight
// ============================================================================================
JNIEXPORT jintArray JNICALL Java_com_example_practiceopencv_PracticeActivity_preprocess4(
        JNIEnv *env,
        jobject thiz,
        jlong sheetMusicAddr,
        jintArray staffsInfoArray) {

    // Create Variable LOGTAG
    const char* LOGTAG = "preprocess3";

    // Create Reference Variable Sheet Music
    Mat &sheetMusic = *(Mat *) sheetMusicAddr;

    // Create Reference Variable(Average Distance)
    jclass cls = (*env).GetObjectClass(thiz);
    jfieldID fid = (*env).GetFieldID(cls, "avgDistance", "D");

    // Get Staffs Info
    const int arrayLength = (*env).GetArrayLength(staffsInfoArray);
    jint staffsInfo[arrayLength];
    (*env).GetIntArrayRegion(staffsInfoArray, 0, arrayLength, staffsInfo);

    // Create Variable Sheet Music Data
    int rows = sheetMusic.rows, cols = sheetMusic.cols;
    double avgDistance = 0.00;

    // Get a Average Distance Between Staffs
    for (int i = 0; i < arrayLength / 5; i++) {
        for (int j = 0; j < 4; j++) {
            avgDistance += abs(staffsInfo[i * 5 + j] - staffsInfo[i * 5 + j + 1]);
        }
    }
    avgDistance /= (arrayLength - arrayLength / 5);

    // Multiply Sheet Music Image by Weight
    const double weightStd = 20.0;
    double weight = weightStd / avgDistance;
    double newWidth = sheetMusic.cols * weight;
    double newHeight = sheetMusic.rows * weight;
    LOGI("%s :: weight : %s", LOGTAG, to_string(weight).data());
    LOGI("%s :: Original width : %s, Original height : %s", LOGTAG, to_string(cols).data(), to_string(rows).data());
    LOGI("%s :: Weighted width : %s, Weighted height : %s", LOGTAG, to_string(newWidth).data(), to_string(newHeight).data());
    resize(sheetMusic, sheetMusic, Size(newWidth, newHeight));

    // Multiply Data by Weight
    LOGI("%s :: Original avgDistance : %s", LOGTAG, to_string(avgDistance).data());
    avgDistance *= weight;
    LOGI("%s :: Weighted avgDistance : %s", LOGTAG, to_string(avgDistance).data());
    for (int i = 0; i < arrayLength; i++) {
        staffsInfo[i] *= weight;
    }

    // Convert Vector to intArray for Return
    jintArray intArray = (*env).NewIntArray(arrayLength);
    for (int i = 0; i < arrayLength; i++) {
        jint element = staffsInfo[i];
        (*env).SetIntArrayRegion(intArray, i, 1, &element);
    }

    // Return avgDistance & Staffs Location
    LOGI("Function End :: %s", LOGTAG);
    (*env).SetDoubleField(thiz, fid, avgDistance);
    return intArray;
};

// ============================================================================================
// process
// ============================================================================================
JNIEXPORT jobjectArray JNICALL Java_com_example_practiceopencv_PracticeActivity_process(
        JNIEnv *env,
        jobject thiz,
        jlong sheetMusicAddr,
        jintArray staffsInfoArray) {

    // Create Variable LOGTAG
    const char* LOGTAG = "process";

    // Create Reference Variable(Sheet Music)
    Mat &sheetMusic = *(Mat *) sheetMusicAddr;

    // Create Reference Variable(Key) & Get Average Distance
    jclass cls = (*env).GetObjectClass(thiz);
    jfieldID key = (*env).GetFieldID(cls, "key", "I");
    jfieldID fid = (*env).GetFieldID(cls, "avgDistance", "D");
    jdouble avgDistance = (*env).GetDoubleField(thiz, fid);

    // Get Staffs Info
    const int arrayLength = (*env).GetArrayLength(staffsInfoArray);
    jint staffsInfo[arrayLength];
    (*env).GetIntArrayRegion(staffsInfoArray, 0, arrayLength, staffsInfo);

    // Create Variable Sheet Music Data
    int rows = sheetMusic.rows, cols = sheetMusic.cols;
    int keyTemp = 0;
    uchar* pixelData = sheetMusic.data;

    // Find Objects
    vector<vector<Point>> contours;
    findContours(sheetMusic, contours, RETR_LIST, CHAIN_APPROX_NONE);
    vector<RectInfo> objectInfo;
    RectInfo rectInfo;
    for (int i = 0; i < contours.size(); i++) {
        Rect rect = boundingRect(contours[i]);
        for (int j = 0; j < arrayLength / 5; j++) {
            double yCenter = (rect.y + rect.y + rect.height) / 2;
            double errorW = rect.width / cols;
            double errorH = rect.height / rows;
            double topPixel, botPixel;
            if ((rect.width >= weighted(20) && rect.height >= weighted(50)) || (rect.width >= weighted(20) && rect.height <= weighted(25) && rect.height >= weighted(15))) {
                topPixel = staffsInfo[j * 5] - weighted(40);
                botPixel = staffsInfo[(j + 1) * 5 - 1] + weighted(40);
            } else {
                topPixel = staffsInfo[j * 5] - weighted(10);
                botPixel = staffsInfo[(j + 1) * 5 - 1] + weighted(10);
            }
            if (rect.width >= weighted(3) && yCenter >=  topPixel && yCenter <= botPixel && errorW < 0.3 && errorH < 0.3) {
                rectInfo = {j, rect};
                objectInfo.push_back(rectInfo);
            }
        }
    }
    sort(objectInfo.begin(), objectInfo.end(), cmp);
    LOGI("%s :: objectInfo.size : %s", LOGTAG, to_string(objectInfo.size()).data());

    // Merge Adjacent Objects
    for (int i = 0; i < objectInfo.size(); i++) {
        for (int j = 0; j < objectInfo.size(); j++) {
            Rect r1 = objectInfo[i].rect;
            Rect r2 = objectInfo[j].rect;
            objectInfo[i].rect = mergeRect(r1, r2);
        }
    }
    for (int i = 0; i < objectInfo.size(); i++) {
        for (int j = 0; j < objectInfo.size(); j++) {
            Rect r1 = objectInfo[i].rect;
            Rect r2 = objectInfo[j].rect;
            objectInfo[i].rect = mergeRect(r1, r2);
        }
    }

    // Deduplication
    vector<RectInfo> objectInfoEx;
    int overlap = 0;
    for (int i = 0; i < objectInfo.size(); i++) {
        if (objectInfo[i].rect.width >= weighted(15) && objectInfo[i].rect.height >= weighted(10)) {
            if (objectInfoEx.size() == 0) {
                objectInfoEx.push_back(objectInfo[i]);
            } else {
                for (int j = 0; j < objectInfoEx.size(); j++) {
                    if (objectInfo[i].rect == objectInfoEx[j].rect) {
                        overlap++;
                    }
                }
                if (overlap == 0) {
                    objectInfoEx.push_back(objectInfo[i]);
                }
                overlap = 0;
            }
        }
    }
    int SIZE = objectInfoEx.size();
    LOGI("%s :: objectInfoEx.size : %s", LOGTAG, to_string(SIZE).data());

    // Get Information on Notes to Recognize Notes
    typedef struct _NoteInfo {
        int lineNumber;
        Rect rect;
        vector<int> tailLoc;
        vector<int> tailStart;
        vector<int> tailEnd;
        vector<int> tailWidth;
        int headWay;
    } NoteInfo;
    NoteInfo noteInfo;
    vector<NoteInfo> notesInfo;
    for (int i = 0; i < SIZE; i++) {
        int xLoc = objectInfoEx[i].rect.x;
        int yLoc = objectInfoEx[i].rect.y;
        int width = objectInfoEx[i].rect.width;
        int height = objectInfoEx[i].rect.height;
        vector<int> tailLoc, tailStart, tailEnd, tailWidth;
        int headWay = 0;
        for (int x = xLoc; x < xLoc + width; x++) {
            int histogram = 0;
            int tailEndTemp = 0;
            for (int y = yLoc; y < yLoc + height; y++) {
                if (pixelData[y * cols + x] != 255) {
                    if (histogram >= 5) {
                        if (pixelData[(y + 1) * cols + x] != 0) {
                            tailEndTemp = y;
                            if (histogram < 50) {
                                histogram = 0;
                            } else {
                                break;
                            }
                        }
                    }
                    histogram++;
                }
            }
            if (histogram > weighted(50)) {
                int tailStartTemp = tailEndTemp - histogram;
                if (x <= xLoc + weighted(10)) {
                    headWay = 1;
                }
                if (tailLoc.size() == 0) {
                    tailLoc.push_back(x);
                    tailStart.push_back(tailStartTemp);
                    tailEnd.push_back(tailEndTemp);
                    tailWidth.push_back(0);
                } else if (abs(tailLoc.back() - x) > 1) {
                    tailLoc.push_back(x);
                    tailStart.push_back(tailStartTemp);
                    tailEnd.push_back(tailEndTemp);
                    tailWidth.push_back(0);
                } else {
                    tailLoc[tailLoc.size() - 1] = x;
                    tailWidth[tailWidth.size() - 1]++;
                }
            }
        }
        noteInfo = {objectInfoEx[i].lineNumber, objectInfoEx[i].rect, tailLoc, tailStart, tailEnd, tailWidth, headWay};
        notesInfo.push_back(noteInfo);
    }
    SIZE = notesInfo.size();
    LOGI("%s :: notesInfo.size : %s", LOGTAG, to_string(notesInfo.size()).data());

    // Recognize Process
    vector<int> beats;
    vector<int> notes;
    pixelData = sheetMusic.data;
    for (int i = 0; i < SIZE; i++) {
        int lineNumber = notesInfo[i].lineNumber;
        int staffsLoc[5] = {
                staffsInfo[(lineNumber * 5) + 0],
                staffsInfo[(lineNumber * 5) + 1],
                staffsInfo[(lineNumber * 5) + 2],
                staffsInfo[(lineNumber * 5) + 3],
                staffsInfo[(lineNumber * 5) + 4]
        };
        int width = notesInfo[i].rect.width;
        int height = notesInfo[i].rect.height;
        int left = notesInfo[i].rect.x;
        int top = notesInfo[i].rect.y;
        int right = left + width;
        int bot = top + height;
        double xCenter = left + width * 0.5;
        double yCenter = top + height * 0.5;
        int headWay = notesInfo[i].headWay;
        int tailCnt = notesInfo[i].tailLoc.size();
        int value = 0;

        // Recognize Key
        int keyAreaTop[2] = {staffsLoc[0] - weighted(5), staffsLoc[0] + weighted(5)};
        int keyAreaBot[2] = {staffsLoc[4] - weighted(5), staffsLoc[4] + weighted(5)};
        int keyCenterTop = staffsLoc[2] - weighted(5);
        int keyCenterBot = staffsLoc[2] + weighted(5);
        if (i == 1) {
            if (top >= keyAreaTop[0] && top <= keyAreaTop[1] && bot >= keyAreaBot[0] && bot <= keyAreaBot[1] && yCenter >= keyCenterTop && yCenter <= keyCenterBot) {
                if (width >= weighted(20) && width <= weighted(35) && height >= weighted(75) && height <= weighted(90)) {
                    int pixelValue = 0;
                    for (int x = left; x < right; x++) {
                        for (int y = top; y < bot; y++) {
                            if (pixelData[y * cols + x] != 255) {
                                pixelValue++;
                            }
                        }
                    }
                    if (pixelValue >= weighted(800)) {
                        Point point = {left, top - weighted(20)};
                        putText(sheetMusic, "T", point, 2, weightedEx(1.5), Scalar(0, 0, 0));
                    }
                }
            } else {
                vector<int> tailCnt;
                int tailStart;
                int tailEnd;
                for (int x = left; x < right; x++) {
                    int histogram = 0;
                    int tailEndTemp = 0;
                    for (int y = top; y < bot; y++) {
                        if (pixelData[y * cols + x] != 255) {
                            if (histogram >= 5) {
                                if (pixelData[(y + 1) * cols + x] != 0) {
                                    tailEndTemp = y;
                                    if (histogram < 30) {
                                        histogram = 0;
                                    } else {
                                        break;
                                    }
                                }
                            }
                            histogram++;
                        }
                    }
                    if (histogram > weighted(30)) {
                        int tailStartTemp = tailEndTemp - histogram;
                        if (tailCnt.size() == 0) {
                            tailCnt.push_back(x);
                            tailStart = tailStartTemp;
                            tailEnd = tailEndTemp;
                        } else if (abs(tailCnt.back() - x) > 1) {
                            tailCnt.push_back(x);
                        } else {
                            tailCnt[tailCnt.size() - 1] = x;
                        }
                    }
                }
                LOGI("%s :: i : %s, tailCnt : %s, tailStart : %s, tailEnd : %s", LOGTAG, to_string(i).data(), to_string(tailCnt.size()).data(), to_string(tailStart).data(), to_string(tailEnd).data());
                int keyCnt;
                int keyAreaTop[2] = {staffsLoc[0] - weighted(10), staffsLoc[0] + weighted(10)};
                int keyAreaBot[2] = {staffsLoc[2], staffsLoc[3]};
                Point point = {left, top - weighted(20)};
                if (tailStart >= keyAreaTop[0] && tailStart <= keyAreaTop[1] && tailEnd >= keyAreaBot[0] && tailEnd <= keyAreaBot[1]) {
                    keyCnt = tailCnt.size();
                    putText(sheetMusic, "b" + to_string(keyCnt), point, 2, weightedEx(1.5), Scalar(0, 0, 0));
                    value = 1;
                    keyTemp = 10 + keyCnt;
                } else {
                    keyCnt = tailCnt.size() / 2;
                    putText(sheetMusic, "#" + to_string(keyCnt), point, 2, weightedEx(1.5), Scalar(0, 0, 0));
                    value = 1;
                    keyTemp = 20 + keyCnt;
                }
            }
        }

        // Recognize Notes
        LOGI("%s :: i : %s, tailCnt : %s, headWay : %s", LOGTAG, to_string(i).data(), to_string(tailCnt).data(), to_string(headWay).data());
        if (value == 0 && width >= weighted(20) && height >= weighted(50)){
            for (int j = 0; j < tailCnt; j++) {
                // Get the Position of a Note
                int noteAreaTop, noteAreaBot;
                if (headWay == 0) {
                    noteAreaTop = notesInfo[i].tailEnd[j] - weighted(20);
                    noteAreaBot = notesInfo[i].tailEnd[j] + weighted(20);
                } else {
                    noteAreaTop = notesInfo[i].tailStart[j] - weighted(20);
                    noteAreaBot = notesInfo[i].tailStart[j] + weighted(20);
                }
                vector<int> noteVec;
                vector<int> histoVec;
                vector<int> histoExVec;
                for (int y = noteAreaTop; y < noteAreaBot; y++) {
                    int histo = 0, histoEx = 0, noteLeft, noteRight;
                    if (headWay == 0) {
                        noteRight = notesInfo[i].tailLoc[j] - notesInfo[i].tailWidth[j];
                        noteLeft = noteRight - weighted(20);
                    } else {
                        noteLeft = notesInfo[i].tailLoc[j] + 1;
                        noteRight = noteLeft + weighted(20) + 1;
                    }
                    for (int x = noteLeft; x < noteRight; x++) {
                        if (pixelData[y * cols + x] != 255) {
                            histo++;
                        }
                    }
                    if (histo >= 10) {
                        noteVec.push_back(y);
                        histoVec.push_back(histo);
                    }
                    for (int x = noteLeft; x < noteRight; x++) {
                        if (pixelData[y * cols + x] != 255) {
                            if (histoEx >= 5) {
                                if (pixelData[y * cols + x + 1] != 0) {
                                    break;
                                }
                            }
                            histoEx++;
                        }
                    }
                    if (histoEx >= 10) {
                        histoExVec.push_back(histoEx);
                    }
                }
                int total = 0, avg = 0, noteLoc = 0;
                for (int i = 0; i < histoVec.size(); i++) {
                    total += noteVec[i];
                    avg++;
                }
                if (avg >= 15) {
                    // Get the Number of Wings
                    noteLoc = rInt(total / avg);
                    int wingAreaTop, wingAreaBot;
                    if (headWay == 0) {
                        wingAreaTop = notesInfo[i].tailStart[j];
                        wingAreaBot = noteLoc - weighted(20);
                    } else {
                        wingAreaTop = noteLoc + weighted(20);
                        wingAreaBot = notesInfo[i].tailEnd[j];
                    }
                    int wings = 0, wideX = 0;
                    if (j == 0) wideX = notesInfo[i].tailLoc[j] + weighted(7);
                    else wideX = notesInfo[i].tailLoc[j] - notesInfo[i].tailWidth[j] - weighted(7);
                    for (int y = wingAreaTop; y < wingAreaBot; y++) {
                        if (pixelData[y * cols + wideX] != 255) {
                            wings++;
                        }
                    }
                    // Find a Point
                    int pointAreaTop = noteLoc - weighted(10);
                    int pointAreaBot = noteLoc + weighted(10);
                    int points = 0;
                    int pointAreaLeft, pointAreaRight;
                    if (headWay == 0) {
                        pointAreaLeft = notesInfo[i].tailLoc[j] + weighted(5);
                        pointAreaRight = notesInfo[i].tailLoc[j] + weighted(15);
                    } else {
                        pointAreaLeft = notesInfo[i].tailLoc[j] + weighted(29);
                        pointAreaRight = notesInfo[i].tailLoc[j] + weighted(39);
                    }
                    // rectangle(sheetMusic, Point(pointAreaLeft, pointAreaTop), Point(pointAreaRight, pointAreaBot), Scalar(0), 3);
                    for (int y = pointAreaTop; y < pointAreaBot; y++) {
                        for (int x = pointAreaLeft; x < pointAreaRight; x++) {
                            if (pixelData[y * cols + x] != 255) {
                                points++;
                            }
                        }
                    }
                    // Separate Notes
                    int histogram = *max_element(histoVec.begin(), histoVec.end());
                    int histogramEx = *max_element(histoExVec.begin(), histoExVec.end());
                    int beat = 0;
                    double std = weighted(18);
                    int staffsLocTop = staffsLoc[0] - 15;
                    int staffsLocBot = staffsLoc[4] + 15;
                    int pointStd;
                    if (noteLoc >= staffsLocTop && noteLoc <= staffsLocBot) pointStd = weighted(15);
                    else pointStd = weighted(60);
                    if (histogramEx >= std && histoExVec.size() >= weighted(15) && wings == weighted(0) && points < pointStd) beat = 4;
                    else if (histogramEx >= std && histoExVec.size() >= weighted(15) && wings == weighted(0) && points >= pointStd) beat = -4;
                    else if (histogramEx >= std && histoExVec.size() >= weighted(15) && wings >= weighted(5) && wings < weighted(17) && points < pointStd) beat = 8;
                    else if (histogramEx >= std && histoExVec.size() >= weighted(15) && wings >= weighted(5) && wings < weighted(17) && points >= pointStd) beat = -8;
                    else if (histogramEx >= std && histoExVec.size() >= weighted(15) && wings >= weighted(17) && points < pointStd) beat = 16;
                    else if (histogramEx >= std && histoExVec.size() >= weighted(15) && wings >= weighted(17) && points >= pointStd) beat = -16;
                    else if (histoExVec.size() < weighted(15) && histogram >= weighted(10) && wings < weighted(5) && points < pointStd) beat = 2;
                    else if (histoExVec.size() < weighted(15) && histogram >= weighted(10) && wings < weighted(5) && points >= pointStd) beat = -2;
                    if (beat != 0) {
                        LOGI("%s :: i : %s, j : %s, histoExVecSize : %s, wings : %s, points : %s", LOGTAG, to_string(i).data(), to_string(j).data(), to_string(histoExVec.size()).data(), to_string(wings).data(), to_string(points).data());
                        Point point = {notesInfo[i].tailLoc[j] - weighted(20), top - weighted(20)};
                        putText(sheetMusic, to_string(beat), point, 2, weightedEx(1.5), Scalar(0, 0, 0));
                        notes.push_back(noteLoc);
                        beats.push_back(beat);
                        value = 1;
                    }
                }
            }
        }

        // Recognize Rests
        int restAreaTop[2] = {staffsLoc[1], staffsLoc[1] + 10};
        int restAreaBot[2] = {staffsLoc[1] + 10, staffsLoc[2]};
        int restCenterTop = staffsLoc[1];
        int restCenterBot = staffsLoc[3];
        if (value == 0 && yCenter >= restCenterTop && yCenter <= restCenterBot && top >= staffsLoc[0] && bot <= staffsLoc[4]) {
            if (width >= weighted(25) && width <= weighted(30) && height >= weighted(14) && height <= weighted(18)) {
                int pixelValue = 0;
                for (int x = left; x < right; x++) {
                    for (int y = top; y < bot; y++) {
                        if (pixelData[y * cols + x] != 255) {
                            pixelValue++;
                        }
                    }
                }
                LOGI("%s :: i : %s, pixelValue : %s", LOGTAG, to_string(i).data(), to_string(pixelValue).data());
                int restValue = 0;
                if (pixelValue >= weighted(370) && pixelValue <= weighted(470)) {
                    if (yCenter >= restAreaTop[0] && yCenter <= restAreaBot[0]) restValue = 1;
                    else if (yCenter >= restAreaTop[1] && yCenter <= restAreaBot[1]) restValue = 2;
                    Point point = {left, top - weighted(20)};
                    putText(sheetMusic, "R" + to_string(restValue), point, 2, weightedEx(1.5), Scalar(0, 0, 0));
                    notes.push_back(-1);
                    beats.push_back(restValue);
                    value = 1;
                }
            } else if (width >= weighted(20) && width <= weighted(25) && height >= weighted(50) && height <= weighted(65)) {
                int pixelValue = 0;
                for (int x = left; x < right; x++) {
                    for (int y = top; y < bot; y++) {
                        if (pixelData[y * cols + x] != 255) {
                            pixelValue++;
                        }
                    }
                }
                LOGI("%s :: i : %s, pixelValue : %s", LOGTAG, to_string(i).data(), to_string(pixelValue).data());
                if (pixelValue >= weighted(550) && pixelValue <= weighted(800)) {
                    Point point = {left, top - weighted(20)};
                    putText(sheetMusic, "R" + to_string(4), point, 2, weightedEx(1.5), Scalar(0, 0, 0));
                    notes.push_back(-1);
                    beats.push_back(4);
                    value = 1;
                }
            } else if (width >= weighted(20) && width <= weighted(25) && height >= weighted(36) && height <= weighted(46)) {
                int pixelValue = 0;
                for (int x = left; x < right; x++) {
                    for (int y = top; y < bot; y++) {
                        if (pixelData[y * cols + x] != 255) {
                            pixelValue++;
                        }
                    }
                }
                LOGI("%s :: i : %s, pixelValue : %s", LOGTAG, to_string(i).data(), to_string(pixelValue).data());
                if (pixelValue >= weighted(250) && pixelValue <= weighted(400)) {
                    Point point = {left, top - weighted(20)};
                    putText(sheetMusic, "R" + to_string(8), point, 2, weightedEx(1.5), Scalar(0, 0, 0));
                    notes.push_back(-1);
                    beats.push_back(8);
                    value = 1;
                }
            }
        }
    }
    cvtColor(sheetMusic, sheetMusic, COLOR_GRAY2RGB);
    for (int i = 0; i < notesInfo.size(); i++) {
        int x;
        if (i < 10) {
            x = notesInfo[i].rect.x;
        } else {
            x = notesInfo[i].rect.x - weighted(10);
        }
        int y = notesInfo[i].rect.y + notesInfo[i].rect.height + weighted(60);
        putText(sheetMusic, to_string(i), Point(x, y), 2, weightedEx(1.5), Scalar(0, 0, 0));
        rectangle(sheetMusic, notesInfo[i].rect, Scalar(255, 0, 0), 3);
        LOGI("%s :: i : %s, width : %s, height : %s", LOGTAG, to_string(i).data(), to_string(notesInfo[i].rect.width).data(), to_string(notesInfo[i].rect.height).data());
    }

    // Convert the Coordinates of Notes to Scale
    int totalStaffs = arrayLength / 5;
    for (int i = 0; i < notes.size(); i++) {
        for (int j = 0; j < totalStaffs; j++) {
            int Lines[21] = {
                    staffsInfo[(j * 5) + 4] + weighted(20) * 3,
                    staffsInfo[(j * 5) + 4] + weighted(20) * 2 + weighted(10), staffsInfo[(j * 5) + 4] + weighted(20) * 2,
                    staffsInfo[(j * 5) + 4] + weighted(20) * 1 + weighted(10), staffsInfo[(j * 5) + 4] + weighted(20) * 1,
                    staffsInfo[(j * 5) + 4] + weighted(10), staffsInfo[(j * 5) + 4],
                    staffsInfo[(j * 5) + 3] + weighted(10), staffsInfo[(j * 5) + 3],
                    staffsInfo[(j * 5) + 2] + weighted(10), staffsInfo[(j * 5) + 2],
                    staffsInfo[(j * 5) + 1] + weighted(10), staffsInfo[(j * 5) + 1],
                    staffsInfo[(j * 5) + 0] + weighted(10), staffsInfo[(j * 5) + 0],
                    staffsInfo[(j * 5) + 0] - weighted(20) * 1 + weighted(10), staffsInfo[(j * 5) + 0] - weighted(20) * 1,
                    staffsInfo[(j * 5) + 0] - weighted(20) * 2 + weighted(10), staffsInfo[(j * 5) + 0] - weighted(20) * 2,
                    staffsInfo[(j * 5) + 0] - weighted(20) * 3 + weighted(10), staffsInfo[(j * 5) + 0] - weighted(20) * 3
            };
            int include = weighted(7);
            if (notes[i] != -1) {
                for (int k = 0; k < 21; k++) {
                    int topLoc = Lines[k] - include;
                    int botLoc = Lines[k] + include;
                    if (notes[i] > topLoc && notes[i] < botLoc) {
                        notes[i] = k;
                    }
                }
            }
        }
    }

    for (int i = 0; i < notes.size(); i++) {
        LOGI("%s :: i : %s, notes : %s, beats : %s", LOGTAG, to_string(i).data(), to_string(notes[i]).data(), to_string(beats[i]).data());
    }

    // Convert Vector to intArray for Return
    jclass intArrayCls = (*env).FindClass("[I");
    jintArray intArray = (*env).NewIntArray(2);
    jobjectArray objArray = (*env).NewObjectArray(notes.size(), intArrayCls, intArray);
    for (int i = 0; i < notes.size(); i++) {
        jintArray elementArray = (*env).NewIntArray(2);
        jint element[] = {notes[i], beats[i]};
        (*env).SetIntArrayRegion(elementArray, 0, 2, element);
        (*env).SetObjectArrayElement(objArray, i, elementArray);
    }

    // Return Notes and Beats
    LOGI("Function End :: %s", LOGTAG);
    (*env).SetIntField(thiz, key, keyTemp);
    return objArray;
    /*
    for (int i = 0; i < notesInfo.size(); i++) {
        LOGI("%s :: i : %s, tailCnt : %s, headWay : %s", LOGTAG, to_string(i).data(), to_string(notesInfo[i].tailCnt).data(), to_string(notesInfo[i].headWay).data());
        for (int j = 0; j < notesInfo[i].tailLoc.size(); j++) {
            int x = floor(notesInfo[i].rect.x + notesInfo[i].rect.width * 0.3 + 0.5);
            int y = floor(notesInfo[i].rect.y + notesInfo[i].rect.height + 60 + 0.5);
            putText(sheetMusic, to_string(notesInfo[i].tailLoc.size()), Point(x, y), 2, weighted(1.5), Scalar(0, 0, 0));
            LOGI("%s :: tailLoc : %s, tailWidth : %s", LOGTAG, to_string(notesInfo[i].tailLoc[j]).data(), to_string(notesInfo[i].tailWidth[j]).data());
        }
    }
    */
    /*
    // Create Variable for Template Matching
    const int SIZE = 4;
    Mat notesImages[SIZE] = {
            *(Mat *) note02cAddr,
            *(Mat *) note04cAddr,
            *(Mat *) note04dAddr,
            *(Mat *) note08cAddr,
    };
    Mat matchResult[SIZE];
    double maxValue[SIZE];
    Point maxLoc[SIZE];
    Mat notesResult[SIZE];

    for (int i = 0; i < SIZE; i++) {
        double weight = 20.0 / notesImages[i].rows;
        double newWidth = notesImages[i].cols * weight;
        double newHeight = notesImages[i].rows * weight;
        logStr = to_string(weight);
        LOGI("%s :: weight : %s", LOGTAG, logStr.data());
        logStrX = to_string(notesImages[i].cols);
        logStrY = to_string(notesImages[i].rows);
        LOGI("%s :: Original width : %s, Original height : %s", LOGTAG, logStrX.data(), logStrY.data());
        logStrX = to_string(newWidth);
        logStrY = to_string(newHeight);
        LOGI("%s :: Weighted width : %s, Weighted height : %s", LOGTAG, logStrX.data(), logStrY.data());
        resize(notesImages[i], notesImages[i], Size(newWidth, newHeight));
    }

    // Template Image GrayScale & Binaryzation
    for (int i = 0; i < SIZE; i++) {
        cvtColor(notesImages[i], notesImages[i], COLOR_RGB2GRAY);
        threshold(notesImages[i], notesImages[i], 127, 255, THRESH_BINARY | THRESH_OTSU);
    }

    // Save Location of Image with highest Similarity
    for (int i = 0; i < SIZE; i++) {
        matchResult[i] = Mat(sheetMusic.cols - notesImages[i].cols + 1, sheetMusic.rows - notesImages[i].rows + 1, CV_8U);
        matchTemplate(sheetMusic, notesImages[i], matchResult[i], TM_CCOEFF_NORMED);
        minMaxLoc(matchResult[i], NULL, &maxValue[i], NULL, &maxLoc[i]);
        logStr = to_string(maxValue[i]);
        LOGI("highest Similarity : %s", logStr.data());
    }

    // Create Custom Note Images
    for (int i = 0; i < SIZE; i++) {
        notesResult[i] = Mat(sheetMusic, Rect(maxLoc[i].x, maxLoc[i].y, notesImages[i].cols, notesImages[i].rows));
    }

    // Template Matching
    for (int i = 0; i < SIZE; i++) {
        matchResult[i] = Mat(sheetMusic.cols - notesResult[i].cols + 1, sheetMusic.rows - notesResult[i].rows + 1, CV_8U);
        matchTemplate(sheetMusic, notesResult[i], matchResult[i], TM_CCOEFF_NORMED);
        normalize(matchResult[i], matchResult[i], 0, 1, NORM_MINMAX);
    }

    cvtColor(sheetMusic, sheetMusic, COLOR_GRAY2RGB);
    float value = 0;
    for (int i = 0; i < SIZE; i++) {
        for (int y = 0; y < matchResult[i].rows; y++) {
            for (int x = 0; x < matchResult[i].cols; x++) {
                value = matchResult[i].at<float>(y, x);
                if (value > 0.9 && maxValue[i] != 0) {
                    if (i == 0) {
                        rectangle(sheetMusic, Point(x, y), Point(x + notesResult[i].cols - 1, y + notesResult[i].rows - 1), Scalar(255, 0, 0), 3);
                    } else if (i == 1) {
                        rectangle(sheetMusic, Point(x, y), Point(x + notesResult[i].cols - 1, y + notesResult[i].rows - 1), Scalar(255, 255, 0), 3);
                    } else if (i == 2) {
                        rectangle(sheetMusic, Point(x, y), Point(x + notesResult[i].cols - 1, y + notesResult[i].rows - 1), Scalar(0, 255, 0), 3);
                    } else {
                        rectangle(sheetMusic, Point(x, y), Point(x + notesResult[i].cols - 1, y + notesResult[i].rows - 1), Scalar(0, 0, 255), 3);
                    }
                }
            }
        }
    }
    */
}

// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// 폐기된 코드
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================
// ============================================================================================

JNIEXPORT void JNICALL Java_com_example_practiceopencv_TestActivity_convertRGBtoGray(
        JNIEnv *env,
        jobject thiz,
        jlong matAddrInput,
        jlong matAddrOutput) {

    Mat matInput = *(Mat *) matAddrInput;
    Mat &matOutput = *(Mat *) matAddrOutput;

    cvtColor(matInput, matOutput, COLOR_RGBA2GRAY);
}

JNIEXPORT void JNICALL Java_com_example_practiceopencv_TestActivity_convertGraytoBinary(
        JNIEnv *env,
        jobject thiz,
        jlong matAddrInput,
        jlong matAddrOutput,
        jlong standard,
        jlong max) {

    Mat matInput = *(Mat *) matAddrInput;
    Mat &matOutput = *(Mat *) matAddrOutput;

    threshold(matInput, matOutput, standard, max, THRESH_BINARY | THRESH_OTSU);
}

JNIEXPORT jobjectArray JNICALL Java_com_example_practiceopencv_TestActivity_staffHistogram(
        JNIEnv *env,
        jobject thiz,
        jlong matAddrInput,
        jlong matAddrOutput,
        jlong matAddrHisto) {

    Mat matInput = *(Mat *) matAddrInput;
    Mat &matOutput = *(Mat *) matAddrOutput;
    Mat &matHisto = *(Mat *) matAddrHisto;

    // Create Variable for Log
    string logStr, logStrX, logStrY;

    // Create LinesInfo Struct(Line == Staff)
    typedef struct _LinesInfo {
        int lineNumber;
        int yPixel;
    } LinesInfo;
    LinesInfo linesInfo;

    // Create Vector has a LinesInfo as a Member
    vector<LinesInfo> staffInfo;
    int lineNum = 0;

    // Create Variable for Staff Detection
    double value = 0;
    int rows = matInput.rows, cols = matInput.cols;
    logStr = to_string(rows);
    LOGI("Input Rows : %s", logStr.data());
    logStr = to_string(cols);
    LOGI("Input Cols : %s", logStr.data());
    uchar *inputPixel = matInput.data;
    uchar *outputPixel = matOutput.data;
    uchar *histoPixel = matHisto.data;

    // Repeat by Input Height
    for (int y = 0; y < rows; y++) {
        // Repeat by Input Width
        for (int x = 0; x < cols; x++) {
            // If Pixel != White
            if (inputPixel[y * cols + x] != 255) {
                // Draw Histogram
                histoPixel[y * cols + int(value)] = 0;
                value++;
            }
        }
        // If a Pixel exceeds 50% in a Rows
        if (value / cols > 0.5) {
            // Vector is Empty
            if (lineNum == 0) {
                lineNum++;
                linesInfo = {lineNum, y};
                staffInfo.push_back(linesInfo);
            }
            // Next Staff
            else if (abs(staffInfo[staffInfo.size() - 1].yPixel - y) > 1) {
                if (lineNum % 5 == 0) {
                    lineNum = 0;
                }
                lineNum++;
                linesInfo = {lineNum, y};
                staffInfo.push_back(linesInfo);
            }
            // Same Staff
            else {
                staffInfo[staffInfo.size() - 1].yPixel = y;
            }
            // Clear the Staff
            for (int z = 0; z < matInput.cols; z++) {
                outputPixel[y * cols + z] = 255;
            }
        }
        value = 0;
    }

    // Get a Average Distance Between Staffs
    int lineHeight = 0;
    for (int i = 0; i < staffInfo.size(); i++) {
        if (i < 4) {
            lineHeight += abs(staffInfo[i].yPixel - staffInfo[i + 1].yPixel);
        }
        logStr = to_string(staffInfo[i].lineNumber);
        logStrY = to_string(staffInfo[i].yPixel);
        LOGI("%s Lines yPixel : %s", logStr.data(), logStrY.data());
    }
    lineHeight /= 4;
    logStr = to_string(lineHeight);
    LOGI("Staff Average Distance : %s", logStr.data());

    // Staffs Division
    Mat crop = matOutput.clone();
    int topPixel = staffInfo[0].yPixel - lineHeight * 10;
    int botPixel = (staffInfo[4].yPixel + lineHeight * 10) - (staffInfo[0].yPixel - lineHeight * 10);
    logStr = to_string(topPixel);
    LOGI("Rect Top yPixel : %s", logStr.data());
    logStr = to_string(botPixel);
    LOGI("Rect Bottom yPixel : %s", logStr.data());
    Mat Rect1(crop, Rect(0, topPixel, matOutput.cols, botPixel));
    matOutput = Rect1.clone();

    // Staffs Info Modify
    for (int i = 0; i < 5; i++) {
        staffInfo[i].yPixel -= topPixel;
    }

    // Create J Variable For Return
    jclass classInfo = (*env).FindClass("[I");
    jobjectArray retArray = (jobjectArray) (*env).NewObjectArray(staffInfo.size(), classInfo, NULL);
    jintArray element;
    for (int i = 0; i < staffInfo.size(); i++) {
        element = (*env).NewIntArray(2);
        jint buf[2] = {staffInfo[i].lineNumber, staffInfo[i].yPixel};
        (*env).SetIntArrayRegion(element, 0, 2, buf);
        (*env).SetObjectArrayElement(retArray, i, element);
    }

    return retArray;
}

JNIEXPORT void JNICALL Java_com_example_practiceopencv_TestActivity_staffMorphology(
        JNIEnv *env,
        jobject thiz,
        jlong matAddrInput,
        jlong matAddrOutput) {

    Mat matInput = *(Mat *) matAddrInput; // Not Used
    Mat &matOutput = *(Mat *) matAddrOutput;

    Mat Kernel = Mat(3, 3, CV_8U, Scalar(255));
    erode(matOutput, matOutput, Kernel, Point(-1, -1), 3);
    dilate(matOutput, matOutput, Kernel, Point(-1, -1), 3);
}

JNIEXPORT void JNICALL Java_com_example_practiceopencv_TestActivity_templateHandCraft(
        JNIEnv *env,
        jobject thiz,
        jlong matAddrInput,
        jlong matAddrOutput,
        jlong matAddrTemplate) {

    Mat matInput = *(Mat *) matAddrInput;
    Mat &matOutput = *(Mat *) matAddrOutput;
    Mat matTemplate = *(Mat *) matAddrTemplate;

    cvtColor(matTemplate, matTemplate, COLOR_RGBA2GRAY);

    // Create Variable for Log
    string logStr, logStrX, logStrY;

    // Create PointInfo Struct
    typedef struct _PointInfo {
        int x;
        int y;
        int sad;
    } PointInfo;
    PointInfo pointInfo;

    // Create Vector has a PointInfo as a Member
    vector<PointInfo> point;

    // Create Variable for Template Matching
    int input_h = matInput.rows;
    int input_w = matInput.cols;
    int template_h = matTemplate.rows;
    int template_w = matTemplate.cols;
    logStrX = to_string(input_w);
    logStrY = to_string(input_h);
    LOGI("input width = %s, height = %s", logStrX.data(), logStrY.data());
    logStrX = to_string(template_w);
    logStrY = to_string(template_h);
    LOGI("template width = %s, height = %s", logStrX.data(), logStrY.data());
    uchar *data_input = matInput.data;
    uchar *data_template = matTemplate.data;

    // Create Variable Standard Location
    int std_position_x = 0;
    int std_position_y = 0;
    int std_sad = 100000;

    // Repeat by Input Height Minus Template Height
    for (int input_y = 0; input_y < input_h - template_h; input_y++) {
        // Repeat by Input Width Minus Template Width
        for (int input_x = 0; input_x < input_w - template_w; input_x++) {
            int SAD = 0;
            // Repeat by Template Height
            for (int template_y = 0; template_y < template_h; template_y++) {
                // Repeat by Template Width
                for (int template_x = 0; template_x < template_w; template_x++) {
                    // Get SAD(Sum of Absolute Difference)
                    int input_pixel = data_input[(input_y + template_y) * input_w + (input_x + template_x)];
                    int template_pixel = data_template[template_y * template_w + template_x];
                    SAD += abs(input_pixel - template_pixel);
                }
            }
            if (SAD < 20000) {
                /*
                logStr = to_string(SAD);
                logStrX = to_string(input_x);
                logStrY = to_string(input_y);
                LOGI("SAD of x(%s) y(%s) : %s", logStrX.data(), logStrY.data(), logStr.data());
                */
            }
            // Change SAD
            if (std_sad > SAD) {
                std_sad = SAD;
                std_position_y = input_y;
                std_position_x = input_x;
            }
            pointInfo = {input_x, input_y, SAD};
            point.push_back(pointInfo);
        }
    }

    logStr = to_string(std_sad);
    logStrX = to_string(std_position_x);
    logStrY = to_string(std_position_y);
    LOGI("Standard x(%s) y(%s) SAD : %s", logStrX.data(), logStrY.data(), logStr.data());

    cvtColor(matOutput, matOutput, COLOR_GRAY2RGB);

    // Draw a Rectangle on a Match
    for (int i = 0; i < point.size(); i++) {
        if (abs(std_sad - point[i].sad) < 4800) {
            rectangle(matOutput, Point(point[i].x, point[i].y), Point(point[i].x + template_w, point[i].y + template_h), Scalar(255, 0, 0), 3);
        }
    }
}

JNIEXPORT void JNICALL Java_com_example_practiceopencv_TestActivity_noteExtaction(
        JNIEnv *env,
        jobject thiz,
        jlong matAddrInput,
        jlong matAddrOutput) {

    Mat matInput = *(Mat *) matAddrInput;
    Mat &matOutput = *(Mat *) matAddrOutput;

    // Create Variable for Log
    string logStr, logStrX, logStrY;

    // Create Variable for Note Extraction
    int rows = matInput.rows, cols = matInput.cols;
    logStr = to_string(rows);
    LOGI("Input Rows : %s", logStr.data());
    logStr = to_string(cols);
    LOGI("Input Cols : %s", logStr.data());
    uchar *inputPixel = matInput.data;
    uchar *outputPixel = matOutput.data;

    // Repeat by Input Height
    for (int y = 0; y < rows; y++) {
        /*
        logStrY = to_string(y);
        LOGI("y : %s", logStrY.data());
        */
        // Repeat by Input Width
        for (int x = 0; x < cols; x++) {
            uchar rPixel = inputPixel[y * cols * 3 + x * 3];
            uchar gPixel = inputPixel[y * cols * 3 + x * 3 + 1];
            uchar bPixel = inputPixel[y * cols * 3 + x * 3 + 2];
            // If Input Pixel != BGR(0, 0, 255)
            if (rPixel > 200 && gPixel < 50 && bPixel < 50) {
                for (int i = 0; i < 3; i++) {
                    outputPixel[y * cols * 3 + x * 3 + i] = 255;
                }
            } else {
                for (int i = 0; i < 3; i++) {
                    outputPixel[y * cols * 3 + x * 3 + i] = 0;
                }
            }
        }
    }
}

JNIEXPORT jobjectArray JNICALL Java_com_example_practiceopencv_TestActivity_noteLabeling(
        JNIEnv *env,
        jobject thiz,
        jlong matAddrInput,
        jlong matAddrOutput) {

    Mat matInput = *(Mat *) matAddrInput;
    Mat &matOutput = *(Mat *) matAddrOutput;

    cvtColor(matInput, matInput, COLOR_RGBA2GRAY);
    threshold(matInput, matInput, 127, 255, THRESH_BINARY | THRESH_OTSU);

    // Create Variable for Log
    string logStr, logStrX, logStrY;

    // Create LinesInfo Struct(Line == Staff)
    typedef struct _NoteInfo {
        int xPixel;
        int yPixel;
    } NoteInfo;
    NoteInfo noteInfo;

    // Create Vector has a NoteInfo as a Member & For Sort
    vector<NoteInfo> notesInfo;
    vector<int> notesInfoSortIndex;
    vector<NoteInfo> notesInfoSort;

    // Create Mat Class for Labeling
    Mat matLabels, matStats, matCentroids;
    int noteLables = connectedComponentsWithStats(matInput, matLabels, matStats, matCentroids, 8, CV_32S);

    // Note Labeling
    for (int i = 1; i < noteLables; i++) {
        int x = matCentroids.at<double>(i, 0); //중심좌표
        int y = matCentroids.at<double>(i, 1);
        noteInfo = {x, y};
        notesInfo.push_back(noteInfo);
        notesInfoSortIndex.push_back(x);
        /*
        logStrX = to_string(x);
        logStrY = to_string(y);
        LOGI("x : %s, y : %s", logStrX.data(), logStrY.data());
        */
        circle(matOutput, Point(x, y), 5, Scalar(255, 0, 0), 1);
    }

    // Notes Info Vector Sort
    sort(notesInfoSortIndex.begin(), notesInfoSortIndex.end());
    for (int i = 0; i < notesInfoSortIndex.size(); i++) {
        for (int j = 0; j < notesInfo.size(); j++) {
            if (notesInfoSortIndex[i] == notesInfo[j].xPixel) {
                int xPixel = notesInfo[j].xPixel;
                int yPixel = notesInfo[j].yPixel;
                logStrX = to_string(xPixel);
                logStrY = to_string(yPixel);
                LOGI("x : %s, y : %s", logStrX.data(), logStrY.data());
                noteInfo = {xPixel, yPixel};
                notesInfoSort.push_back(noteInfo);
            }
        }
    }

    // Create J Variable For Return
    jclass classInfo = (*env).FindClass("[I");
    jobjectArray retArray = (jobjectArray) (*env).NewObjectArray(notesInfoSort.size(), classInfo, NULL);
    jintArray element;
    for (int i = 0; i < notesInfoSort.size(); i++) {
        element = (*env).NewIntArray(2);
        jint buf[2] = {notesInfoSort[i].xPixel, notesInfoSort[i].yPixel};
        (*env).SetIntArrayRegion(element, 0, 2, buf);
        (*env).SetObjectArrayElement(retArray, i, element);
    }

    return retArray;
}

}