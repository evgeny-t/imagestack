#ifndef IMAGESTACK_PAINT_H
#define IMAGESTACK_PAINT_H
#include "header.h"

class Eval : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static NewImage apply(NewImage im, string expression);
};

class EvalChannels : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static NewImage apply(NewImage im, vector<string> expressions);
};

class Plot : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static NewImage apply(NewImage im, int width, int height, float lineThickness);
};

class Composite : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static void apply(NewImage dst, NewImage src);
    static void apply(NewImage dst, NewImage src, NewImage mask);
};

#include "footer.h"
#endif
