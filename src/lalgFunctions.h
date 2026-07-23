#ifndef LALGFUNC_H
#define LALGFUNC_H

static const char* lalgDefinitions = "\n\
import @math;\n\
const int xAxis = 1;\n\
const int yAxis = 2;\n\
const int zAxis = 3;\n\
struct vector2 {\n\
    double128 x;\n\
    double128 y;\n\
    function vector2(double128 x, double128 y) -> vector2 {\n\
        this.x = x;\n\
        this.y = y;\n\
        return this;\n\
    }\n\
    function magnitude() -> double128 {\n\
        return ((this.x*this.x)+(this.y*this.y))**0.5ld;\n\
    }\n\
    function dotProduct(vector2 n) -> double128 {\n\
        return this.x*n.x+this.y*n.y;\n\
    }\n\
    function crossProduct(vector2 n) -> double128 {\n\
        return this.x*n.y - this.y*n.x;\n\
    }\n\
    function angle(vector2 n) -> double128 {\n\
        double128 numerator = this.dotProduct(n);\n\
        double128 denom = this.magnitude()*n.magnitude();\n\
        if (denom == 0 && numerator >= 0) {return inf;}\n\
        if (denom == 0 && numerator < 0) {return -inf;}\n\
        return acos(numerator/denom);\n\
    }\n\
    function round(int places) -> vector2 {\n\
        this.x = round(this.x, places);\n\
        this.y = round(this.y, places);\n\
        return this;\n\
    }\n\
    function toString() -> string {\n\
        return formats(\"<%D, %D>\", this.x, this.y);\n\
    }\n\
    function normalize() -> vector2 {\n\
        double128 mag = this.magnitude();\n\
        return vector2(this.x/mag, this.y/mag);\n\
    }\n\
    function normalizeN(double128 n) -> vector2 {\n\
        vector2 norm = this.normalize();\n\
        return vector2(norm.x*n, norm.y*n);\n\
    }\n\
    \n\
    function rotate(double128 t) -> vector2 {\n\
        double128 nx = this.x*cos(t) - this.y*sin(t);\n\
        double128 ny = this.x*sin(t) - this.y*cos(t);\n\
        return vector2(nx, ny);\n\
    }\n\
    function op_add(vector2 n) -> vector2 {return vector2(this.x+n.x, this.y+n.y);}\n\
    function op_sub(vector2 n) -> vector2 {return vector2(this.x-n.x, this.y-n.y);}\n\
    function scalarMult(double128 n) -> vector2 {return vector2(this.x*n, this.y*n);}\n\
}\n\
struct vector3 {\n\
    double128 x;\n\
    double128 y;\n\
    double128 z;\n\
    function vector3(double128 x, double128 y, double128 z) -> vector3 {\n\
        this.x = x;\n\
        this.y = y;\n\
        this.z = z;\n\
        return this;\n\
    }\n\
    function magnitude() -> double128 {\n\
        return ((this.x*this.x)+(this.y*this.y)+(this.z*this.z))**0.5ld;\n\
    }\n\
    function dotProduct(vector3 n) -> double128 {\n\
        return this.x*n.x+this.y*n.y+this.z*n.z;\n\
    }\n\
    function crossProduct(vector3 n) -> vector3 {\n\
        double128 nx = this.y*n.z - this.z*n.y;\n\
        double128 ny = this.z*n.x - this.x*n.z;\n\
        double128 nz = this.x*n.y - this.y*n.x;\n\
        return vector3(nx, ny, nz);\n\
    }\n\
    function angle(vector3 n) -> double128 {\n\
        double128 numerator = this.dotProduct(n);\n\
        double128 denom = this.magnitude()*n.magnitude();\n\
        if (denom == 0 && numerator >= 0) {return inf;}\n\
        if (denom == 0 && numerator < 0) {return -inf;}\n\
        return acos(numerator/denom);\n\
    }\n\
    function round(int places) -> vector3 {\n\
        this.x = round(this.x, places);\n\
        this.y = round(this.y, places);\n\
        this.z = round(this.z, places);\n\
        return this;\n\
    }\n\
    function toString() -> string {\n\
        return formats(\"<%D, %D, %D>\", this.x, this.y, this.z);\n\
    }\n\
    function normalize() -> vector3 {\n\
        double128 mag = this.magnitude();\n\
        return vector3(this.x/mag, this.y/mag, this.z/mag);\n\
    }\n\
    function normalizeN(double128 n) -> vector3 {\n\
        vector3 norm = this.normalize();\n\
        norm.x *= n;\n\
        norm.y *= n;\n\
        norm.z *= n;\n\
        return norm;\n\
    }\n\
    function rotate(int axis, double128 theta) -> vector3 {\n\
        double128 nx = this.x;\n\
        double128 ny = this.y;\n\
        double128 nz = this.z;\n\
        if (axis == xAxis) {\n\
            ny = (this.y*cos(theta))-(this.z*sin(theta));\n\
            nz = (this.y*sin(theta))+(this.z*cos(theta));\n\
            return vector3(this.x, ny, nz);\n\
        }\n\
        elif (axis == yAxis) {\n\
            nx = (this.x*cos(theta))+(this.z*sin(theta));\n\
            nz = (this.x*-sin(theta))+(this.z*cos(theta));\n\
            return vector3(nx, this.y, nz);\n\
        }\n\
        elif (axis == zAxis) {\n\
            nx = (this.x*cos(theta))+(this.y*-sin(theta));\n\
            ny = (this.x*sin(theta))+(this.y*cos(theta));\n\
            return vector3(nx, ny, this.z);\n\
        }\n\
        else {\n\
        print(\"Invalid axis provided\");\n\
        exit();\n\
        }\n\
    }\n\
    function rotateByVector3(vector3 n, int[] arr) -> vector3 {\n\
        if (#arr != 3) {exception ex = exception(0x30, \"Rotation array needs to be an array of axes; see documentation\", -1); throw(ex);}\n\
        vector3 tr = vector3(this.x, this.y, this.z);\n\
        for (int i = 0; i < 3; i++) {\n\
            if (arr[i] == xAxis) {tr = tr.rotate(xAxis, n.x, true);}\n\
            elif (arr[i] == yAxis) {tr = tr.rotate(yAxis, n.y, true);}\n\
            elif (arr[i] == zAxis) {tr = tr.rotate(zAxis, n.z, true);}\n\
            else {print(\"%i\", arr[i]); exception ex2 = exception(0x30, \"Rotation array needs to be an array of axes; see documentation\", -1); throw(ex2);}\n\
        }\n\
        return tr;\n\
    }\n\
    function op_add(vector3 n) -> vector3 {return vector3(this.x+n.x, this.y+n.y, this.z+n.z);}\n\
    function op_sub(vector3 n) -> vector3 {return vector3(this.x-n.x, this.y-n.y, this.z-n.z);}\n\
    function scalarMult(double128 n) -> vector3 {return vector3(this.x*n, this.y*n, this.z*n);}\n\
}\n\
struct vectorN {\n\
    double128[] values;\n\
    function vectorN(double128[] values) -> vectorN {\n\
        this.values = values;\n\
        return this;\n\
    }\n\
    function toString() -> string {\n\
        string toReturn = formats(\"%a\", this.values);\n\
        toReturn[0] = '<';\n\
        toReturn = toReturn.replace(\"]\", \">\");\n\
        return toReturn;\n\
    }\n\
    function dims() -> int {\n\
        return #this.values;\n\
    }\n\
    function magnitude() -> double128 {\n\
        double128 sum = 0;\n\
        for (int i = 0; i < #this.values; i++) {\n\
            sum += (this.values[i]*this.values[i]);\n\
        }\n\
        return sum**0.5ld;\n\
    }\n\
    function dotProduct(vectorN n) -> double128 {\n\
        if (#this.values != #n.values) {\n\
            exception ex = exception(0x30, \"Cannot take the dot product of two vectors with unequal dimension\", -1);\n\
            throw(ex);\n\
        }\n\
        double128 sum = 0;\n\
        for (int i = 0; i < #n; i++) {\n\
            sum += this.values[i]*n.values[i];\n\
        }\n\
        return sum;\n\
    }\n\
    function angle(vectorN n) -> double128 {\n\
        double128 numerator = this.dotProduct(n);\n\
        double128 denom = this.magnitude()*n.magnitude();\n\
        if (denom == 0 && numerator >= 0) {return inf;}\n\
        if (denom == 0 && numerator < 0) {return -inf;}\n\
        return acos(numerator/denom);\n\
    }\n\
    function round(int places) -> vectorN {\n\
        for (int i = 0; i < #this.values; i++) {\n\
            this.values[i] = round(this.values[i], places);\n\
        }\n\
        return this;\n\
    }\n\
    function normalize() -> vectorN {\n\
        double128 mag = this.magnitude();\n\
        double128[] toGenerate = [];\n\
        for (int i = 0; i < #this.values; i++) {\n\
            toGenerate.append(this.values[i]/mag);\n\
        }\n\
        return vectorN(toGenerate);\n\
    }\n\
    function normalizeN(double128 n) -> vectorN {\n\
        vectorN norm = this.normalize();\n\
        for (int i = 0; i < #norm.values; i++) {\n\
            norm.values[i] *= n;\n\
        }\n\
        return norm;\n\
    }\n\
    function op_add(vectorN n) -> vectorN {\n\
        if (#n.values != #this.values) {exception ex = exception(0x30, \"Vectors have to be equal dimensions to add\", -1); throw(ex);}\n\
        double128[] toReturn = [];\n\
        for (int i = 0; i < #n.values; i++) {\n\
            toReturn.append(n.values[i]+this.values[i])\n\
        }\n\
        return vectorN(toReturn);\n\
    }\n\
    function op_sub(vectorN n) -> vectorN {\n\
        if (#n.values != #this.values) {exception ex = exception(0x30, \"Vectors have to be equal dimensions to add\", -1); throw(ex);}\n\
        double128[] toReturn = [];\n\
        for (int i = 0; i < #n.values; i++) {\n\
            toReturn.append(n.values[i]-this.values[i])\n\
        }\n\
        return vectorN(toReturn);\n\
    }\n\
    function scalarMult(vectorN n) -> vectorN {\n\
        double128[] toReturn = [];\n\
        for (int i = 0; i < #this.values; i++) {\n\
            toReturn.append(this.values[i]*n)\n\
        }\n\
        return vectorN(toReturn);\n\
    }\n\
}\n\
function detMatrix(double128[] matrix, int size) -> double128 {\n\
    if (size == 1) {\n\
        return matrix[0][0];\n\
    }\n\
    if (size == 2) {\n\
        return matrix[0][0] * matrix[1][1] - matrix[0][1] * matrix[1][0];\n\
    }\n\
    double128 sum = 0;\n\
    for (int col = 0; col < size; col++) {\n\
        double128[][] sub = [];\n\
        for (int i = 1; i < size; i++) {\n\
            double128[] row = [];\n\
            for (int j = 0; j < size; j++) {\n\
                if (j != col) {\n\
                    row.append(matrix[i][j]);\n\
                }\n\
            }\n\
            sub.append(row);\n\
        }\n\
        double128 sign = 1ld;\n\
        if (col % 2 == 1) { sign = -1ld; }\n\
        sum += sign * matrix[0][col] * detMatrix(sub, size - 1);\n\
    }\n\
    return sum;\n\
}\n\
\n\
function vectorNinit(int amt) -> vectorN {\n\
    vectorN toReturn = vectorN([0]);\n\
    if (amt <= 0) {return toReturn;}\n\
    for (int i = 0; i < amt-1; i++) {\n\
        toReturn.values.append(0);\n\
    }\n\
    return toReturn;\n\
}\n\
function doubleMtrxFromDims(int rows, int cols) -> double128[] {\n\
    double128[] toReturn = [];\n\
    for (int i = 0; i < rows; i++) {\n\
        double128[] toApp = [];\n\
        for (int j = 0; j < cols; j++) {toApp.append(0ld);}\n\
        toReturn.append(toApp);\n\
    }\n\
    return toReturn;\n\
}\n\
struct mtrx {\n\
    double128[] data;\n\
    int rows;\n\
    int cols;\n\
    function mtrx(double128[] data, int rows, int cols) -> mtrx {\n\
        this.data = data;\n\
        this.rows = rows;\n\
        this.cols = cols;\n\
        return this;\n\
    }\n\
    function det() -> double128 {\n\
        if (this.rows != this.cols) {\n\
            exception ex = exception(0x30, \"Determinant undefined for non-square matrices\", -1);\n\
            throw(ex);\n\
        }\n\
        return detMatrix(this.data, this.rows);\n\
    }\n\
    function toVector() -> vectorN {\n\
        if (this.rows == 1 || this.cols == 1) {\n\
            double128[] d = [];\n\
            int max = this.cols;\n\
            if (max == 1) {max = this.rows;}\n\
            for (int i = 0; i < max; i++) {\n\
                if (this.cols == 1) {d.append(this.data[i][0]);}\n\
                if (this.rows == 1) {d.append(this.data[0][i]);}\n\
            }\n\
            return vectorN(d);\n\
        }\n\
        exception ex = exception(0x30, \"Cannot make a NxN matrix into a vector when one dimension is greater than one\", -1);\n\
        throw(ex);\n\
    }\n\
    function toString() -> string {\n\
        string toReturn = \"\";\n\
        for (int i = 0; i < #this.data; i++) {\n\
            toReturn += formats(\"%a\\n\", this.data[i]);\n\
        }\n\
        print(\"%s\", toReturn);\n\
        return toReturn;\n\
    }\n\
    function mult(mtrx v) -> mtrx {\n\
        if (this.cols != v.rows) {return 0;};\n\
        double128[] newMtrx = doubleMtrxFromDims(this.rows, v.cols);\n\
        int n = this.rows;\n\
        int p = v.cols;\n\
        int m = this.cols;\n\
        for (int i = 0; i < n; i++) {\n\
            for (int j = 0; j < p; j++) {\n\
                int sum = 0;\n\
                for (int k = 0; k < m; k++) {\n\
                    sum += this.data[i][k]*v.data[k][j];\n\
                }\n\
                newMtrx[i][j] = sum;\n\
            }\n\
        }\n\
        return mtrx(newMtrx, n, p);\n\
    }\n\
    function round(int p) -> mtrx {\n\
        for (int i = 0; i < this.cols; i++) {\n\
            for (int j = 0; j < this.rows; j++) {\n\
                this.data[i][j] = round(this.data[i][j], p);\n\
            }\n\
        }\n\
        return mtrx(this.data, this.rows, this.cols);\n\
    }\n\
    function scalarMult(double128 n) -> mtrx {\n\
        double128[] toReturn = doubleMtrxFromDims(this.rows, this.cols);\n\
        for (int i = 0; i < this.rows; i++) {\n\
            for (int j = 0; j < this.cols; j++) {\n\
                toReturn[i][j] = this.data[i][j]*n;\n\
            }\n\
        }\n\
        return mtrx(toReturn, this.rows, this.cols);\n\
    }\n\
    function op_add(mtrx n) -> mtrx {\n\
        if (this.cols != n.cols || this.rows != n.rows) {exception ex = exception(0x30, \"Matrices have to have equal dimensions\", -1); throw(ex);}\n\
        double128[] toReturn = doubleMtrxFromDims(this.rows, this.cols);\n\
        for (int i = 0; i < this.rows; i++) {\n\
            for (int j = 0; j < this.cols; j++) {\n\
                toReturn[i][j] = this.data[i][j] + n.data[i][j];\n\
            }\n\
        }\n\
        return mtrx(toReturn, this.rows, this.cols);\n\
    }\n\
    function op_sub(mtrx n) -> mtrx {\n\
        if (this.cols != n.cols || this.rows != n.rows) {exception ex = exception(0x30, \"Matrices have to have equal dimensions\", -1); throw(ex);}\n\
        double128[] toReturn = doubleMtrxFromDims(this.rows, this.cols);\n\
        for (int i = 0; i < this.rows; i++) {\n\
            for (int j = 0; j < this.cols; j++) {\n\
                toReturn[i][j] = this.data[i][j] - n.data[i][j];\n\
            }\n\
        }\n\
        return mtrx(toReturn, this.rows, this.cols);\n\
    }\n\
}\n\
function identityMtrx(int n) -> mtrx {\n\
    double128 toReturn = doubleMtrxFromDims(n, n);\n\
    for (int i = 0; i < n; i++) {\n\
        toReturn[i][i] = 1;\n\
    }\n\
    return mtrx(toReturn, n, n);\n\
}\n\
";

#endif
