#pragma once

class Stopwatch
{
public:
    Stopwatch()
    {
        QueryPerformanceFrequency(&this->Frequency);
        this->reset();
    }

    LARGE_INTEGER get() const
    {
        LARGE_INTEGER ret;
        QueryPerformanceCounter(&ret);
        ret.QuadPart -= this->StartingTime.QuadPart;
        return ret;
    }

    LARGE_INTEGER get_nano() const
    {
        LARGE_INTEGER ret = get();
        ret.QuadPart *= 1000000000;
        ret.QuadPart /= this->Frequency.QuadPart;
        return ret;
    }

    LARGE_INTEGER get_micro() const
    {
        LARGE_INTEGER ret = get();
        ret.QuadPart *= 1000000;
        ret.QuadPart /= this->Frequency.QuadPart;
        return ret;
    }

    LARGE_INTEGER get_milli() const
    {
        LARGE_INTEGER ret = get();
        ret.QuadPart *= 1000;
        ret.QuadPart /= this->Frequency.QuadPart;
        return ret;
    }

    void reset()
    {
        QueryPerformanceCounter(&this->StartingTime);
    }

private:
    LARGE_INTEGER Frequency;
    LARGE_INTEGER StartingTime;
};
