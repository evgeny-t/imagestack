#ifndef IMAGESTACK_COMPLEX_H
#define IMAGESTACK_COMPLEX_H
#include "header.h"

class ComplexMultiply : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static void apply(NewImage a, NewImage b, bool conj);
};

class ComplexDivide : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static void apply(NewImage a, NewImage b, bool conj);
};

class ComplexReal : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static NewImage apply(NewImage im);
};

class RealComplex : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static NewImage apply(NewImage im);
};

class ComplexImag : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static NewImage apply(NewImage im);
};

class ComplexMagnitude : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static NewImage apply(NewImage im);
};

class ComplexPhase : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static NewImage apply(NewImage im);
};

class ComplexConjugate : public Operation {
public:
    void help();
    void parse(vector<string> args);
    static void apply(NewImage im);
};

#include "footer.h"
#endif
