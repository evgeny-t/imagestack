#ifndef IMAGESTACK_MATH_H
#define IMAGESTACK_MATH_H
#include "header.h"

class Add : public Operation {
public:
    void help();
    void parse(vector<string> args);
};

class Multiply : public Operation {
public:
    void help();
    void parse(vector<string> args);
    enum Mode {Elementwise = 0, Inner, Outer};

    static NewImage apply(NewImage a, NewImage b, Mode m);
};

class Subtract : public Operation {
public:
    void help();
    void parse(vector<string> args);
};

class Divide : public Operation {
public:
    void help();
    void parse(vector<string> args);
};

class Maximum : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static void apply(NewImage a, NewImage b);
};

class Minimum : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static void apply(NewImage a, NewImage b);
};

class Log : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static void apply(NewImage a);
};

class Exp : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static void apply(NewImage a, float base = E);
};

class Abs : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static void apply(NewImage a);
};

class Offset : public Operation {
public:
    void help();
    void parse(vector<string> args);
};

class Scale : public Operation {
public:
    void help();
    void parse(vector<string> args);
};

class Gamma : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static void apply(NewImage a, vector<float>);
    static void apply(NewImage a, float);
};

class Mod : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static void apply(NewImage a, vector<float>);
    static void apply(NewImage a, float);
};

class Clamp : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static void apply(NewImage a, float lower = 0, float upper = 1);
};

class DeNaN : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static void apply(NewImage a, float replacement = 0);
};

class Threshold : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static void apply(NewImage a, float val);
};

class Normalize : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static void apply(NewImage a);
};

class Quantize : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static void apply(NewImage a, float increment);
};

#include "footer.h"
#endif
