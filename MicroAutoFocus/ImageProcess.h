#pragma once
#include "stdafx.h"

struct CalResult{ double ComplexDis; double PixelInten; };

Mat GenerateFitSequence(int _length);
Mat CalculeteLog(Mat _src, int _ch);
CalResult PolyResults(Mat _src);
CalResult SolveResults(Mat _src);
double findMaxRadius(Mat _src);
