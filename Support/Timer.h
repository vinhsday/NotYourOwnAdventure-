#ifndef TIMER_H
#define TIMER_H

class Timer {
private:
    float timeSMax;
    float timeSCurrent;

public:
    Timer(float setTimeSMax, float setTimeSCurrent = 0.0f);
    void countUp(float dT);
    void countDown(float dT);
    void resetToZero();
    void resetToMax();
    bool timeSIsZero();
    bool timeSIsGreaterThanOrEqualTo(float timeSCheck);

    // Thêm các phương thức mới
    void reset(float time) { timeSMax = time; timeSCurrent = time; } // Đặt lại thời gian tối đa và hiện tại
    float getTimeSCurrent() { return timeSCurrent; } // Thêm getter để debug
};

#endif
