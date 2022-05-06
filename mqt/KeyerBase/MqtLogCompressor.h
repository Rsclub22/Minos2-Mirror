#ifndef MQTLOGCOMPRESSOR_H
#define MQTLOGCOMPRESSOR_H

#include <math.h>

class MqtLogCompressor
{
    int gamma = 1;
    double limit = 1;
    double threshold = 0.25;
public:
    MqtLogCompressor()
    {}

    void setGamma(int g)
    {
        gamma = g;
        limit = log((1 - threshold) * gamma + 1 );

    }
    void setThreshold(double t)
    {
        threshold = t;
    }

    double logfunc(double s)
    {
        if (s > threshold)
        {
            return log((s - threshold) * gamma + 1 )/limit;
        }
        else
        {
            return s * log(threshold * gamma + 1)/limit;
        }
    }

    void procChannel(double &s)
    {
        if (s < 0)
        {
            s = -s;
            s = logfunc(s);
            s = -s;
        }
        else
        {
            s = logfunc(s);

        }
    }
    void process(double &s1, double &s2)
    {
        procChannel(s1);
        procChannel(s2);
    }
};


#endif // MQTLOGCOMPRESSOR_H
