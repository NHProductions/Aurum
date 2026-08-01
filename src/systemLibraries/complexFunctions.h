#ifndef COMPLEXFUNC_H
#define COMPLEXFUNC_H
#include "../processing/bytecoder.h"
#include "sysFunctions.h"
void cExponentialFuncs(auFunc);
void cTrig(auFunc);
static const char* cplxDefinitions = "\n\
import @math;\n\
struct cplxNum {\n\
    double128 a;\n\
    double128 b;\n\
    function cplxNum(double128 a, double128 b) -> cplxNum {\n\
        this.a = a;\n\
        this.b = b;\n\
        return this;\n\
    }\n\
    function cabs() -> double128 {return hypot(this.a, this.b);}\n\
    function op_len() -> double128 {return this.cabs();}\n\
    function carg() -> double128 {\n\
        if (this.a == 0) {return 0;}\n\
        return atan(this.b/this.a);\n\
    }\n\
    function imag() -> double128 {return this.b;}\n\
    function real() -> double128 {return this.a;}\n\
    function toPolarString() -> string {\n\
        double128 r = hypot(this.a, this.b);\n\
        double128 t = 0;\n\
        if (this.a != 0) {t = atan(this.b/this.a);}\n\
        return formats(\"%De^i%D\", r, t);\n\
    }\n\
    function toCartesianString() -> string {return formats(\"%D+%Di\", this.a, this.b);}\n\
    function conj() -> cplxNum {return cplxNum(this.a, -this.b);}\n\
    function op_add(cplxNum n) -> cplxNum {return cplxNum(this.a+n.a, this.b+n.b);}\n\
    function op_sub(cplxNum n) -> cplxNum {return cplxNum(this.a-n.a, this.b-n.b);}\n\
    function op_mul(cplxNum n) -> cplxNum {return cplxNum((this.a*n.a)-(this.b*n.b), (this.a*n.b)+(n.a*this.b));}\n\
    function op_exp(cplxNum n) -> cplxNum {return cpow(this, n);}\n\
}\n\
";
static const bcFunction cplxFunctions[] = {
    {.name = "cexp", .argc = 1, .returnStruct = "cplxNum"},
    {.name = "ctxp", .argc = 1, .returnStruct = "cplxNum"},
    {.name = "cpow", .argc = 2, .returnStruct = "cplxNum"},
    {.name = "cln", .argc = 1, .returnStruct = "cplxNum"},
    {.name = "clog2", .argc = 1, .returnStruct = "cplxNum"},
    {.name = "clog10", .argc = 1, .returnStruct = "cplxNum"},
    {.name = "clog", .argc = 2, .returnStruct = "cplxNum"},

    {.name = "csin", .argc = 1, .returnStruct = "cplxNum"},
    {.name = "ccos", .argc = 1, .returnStruct = "cplxNum"},
    {.name = "ctan", .argc = 1, .returnStruct = "cplxNum"},
    {.name = "ccsc", .argc = 1, .returnStruct = "cplxNum"},
    {.name = "csec", .argc = 1, .returnStruct = "cplxNum"},
    {.name = "ccot", .argc = 1, .returnStruct = "cplxNum"},
    {.name = "casin", .argc = 1, .returnStruct = "cplxNum"},
    {.name = "cacos", .argc = 1, .returnStruct = "cplxNum"},
    {.name = "catan", .argc = 1, .returnStruct = "cplxNum"},
    {.name = "cacsc", .argc = 1, .returnStruct = "cplxNum"},
    {.name = "casec", .argc = 1, .returnStruct = "cplxNum"},
    {.name = "cacot", .argc = 1, .returnStruct = "cplxNum"},
    {.name = "csinh", .argc = 1, .returnStruct = "cplxNum"},
    {.name = "ccosh", .argc = 1, .returnStruct = "cplxNum"},
    {.name = "ctanh", .argc = 1, .returnStruct = "cplxNum"},
    {.name = "ccsch", .argc = 1, .returnStruct = "cplxNum"},
    {.name = "csech", .argc = 1, .returnStruct = "cplxNum"},
    {.name = "ccoth", .argc = 1, .returnStruct = "cplxNum"},
    {.name = "casinh", .argc = 1, .returnStruct = "cplxNum"},
    {.name = "cacosh", .argc = 1, .returnStruct = "cplxNum"},
    {.name = "catanh", .argc = 1, .returnStruct = "cplxNum"},
    {.name = "cacsch", .argc = 1, .returnStruct = "cplxNum"},
    {.name = "casech", .argc = 1, .returnStruct = "cplxNum"},
    {.name = "cacoth", .argc = 1, .returnStruct = "cplxNum"},
    {.name = "", .argc = -1}
};
#endif