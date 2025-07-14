#ifndef ITWEEN
#define ITWEEN

class ITween {
public:
    virtual ~ITween() = default;
    virtual void ProcessTime() = 0;
};

#endif 


