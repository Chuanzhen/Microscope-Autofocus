#include "stdafx.h"
#include "ImageProcess.h"

Mat GenerateFitSequence(int _length)
{
	Mat temp(_length, 1, CV_64F);
	for (int i = 0; i < _length; i++)
		temp.at<double>(i, 0) = i + 1;
	temp.convertTo(temp, CV_32F);

	return temp;
}

Mat CalculeteLog(Mat _src, int _ch)
{
	// I - constant
	// 0 to 1 rows. 1 to 1 cols;
	Mat I, temp;
	_src.convertTo(I, CV_32F);
	reduce(I, temp, _ch, CV_REDUCE_AVG, -1);
	if (_ch == 0)
		transpose(temp, temp);

	log(temp, temp);
	return temp;
}


CalResult PolyResults(Mat _src)
{
	// calculate according to rows, result is 1 col
	Mat I = _src.clone();
	// rows, y
	Mat yr = CalculeteLog(I, 1);
	Mat xr = GenerateFitSequence(yr.rows);

	// cols, x
	Mat yc = CalculeteLog(I, 0);
	Mat xc = GenerateFitSequence(yc.rows);

	int order = 2;

	// rows, y
	Mat rfitCoeff = Mat(order + 1, 1, CV_32F);
	polyfit(xr, yr, rfitCoeff, order);
	rfitCoeff.convertTo(rfitCoeff, CV_64F);

	// cols, x
	Mat cfitCoeff = Mat(order + 1, 1, CV_32F);
	polyfit(xc, yc, cfitCoeff, order);
	cfitCoeff.convertTo(cfitCoeff, CV_64F);

	int posX, posY; // means
	double MeanX, MeanY;
	double DisX, DisY; // width

	DisY = rfitCoeff.at<double>(2, 0);
	DisX = cfitCoeff.at<double>(2, 0);

	MeanY = rfitCoeff.at<double>(1, 0);
	MeanX = cfitCoeff.at<double>(1, 0);

	posY = int(-MeanY / DisY / 2);
	posX = int(-MeanX / DisX / 2);

	I.convertTo(I, CV_64F);

	CalResult Output;
	Output.ComplexDis = sqrt(-(1 / DisY + 1 / DisX) / 2);
	Output.PixelInten = I.at<double>(posY, posX);

	return Output;
}

CalResult SolveResults(Mat _src)
{
	// calculate according to rows, result is 1 col
	Mat I = _src.clone();
	// rows, y
	Mat yr = CalculeteLog(I, 1);
	Mat xr = GenerateFitSequence(yr.rows);
	Mat temp2, temp0, temp; pow(xr, 2, temp2); pow(xr, 0, temp0);
	hconcat(temp0, xr, temp); hconcat(temp, temp2, xr);

	// cols, x
	Mat yc = CalculeteLog(I, 0);
	Mat xc = GenerateFitSequence(yc.rows);
	pow(xc, 2, temp2); pow(xc, 0, temp0);
	hconcat(temp0, xc, temp); hconcat(temp, temp2, xc);

	int order = 2;
	// rows, y
	Mat rfitCoeff = Mat::ones(order + 1, 1, CV_32F);
	solve(xr, yr, rfitCoeff, DECOMP_LU | DECOMP_NORMAL);
	rfitCoeff.convertTo(rfitCoeff, CV_64F);

	// cols, x
	Mat cfitCoeff = Mat::ones(order + 1, 1, CV_32F);
	solve(xc, yc, cfitCoeff, DECOMP_LU | DECOMP_NORMAL);
	cfitCoeff.convertTo(cfitCoeff, CV_64F);

	int posX, posY; // means
	double MeanX, MeanY;
	double DisX, DisY; // width

	DisY = rfitCoeff.at<double>(2, 0);
	DisX = cfitCoeff.at<double>(2, 0);

	MeanY = rfitCoeff.at<double>(1, 0);
	MeanX = cfitCoeff.at<double>(1, 0);

	posY = int(-MeanY / DisY / 2);
	posX = int(-MeanX / DisX / 2);

	I.convertTo(I, CV_64F);

	CalResult Output;
	Output.ComplexDis = sqrt(-(1 / DisY + 1 / DisX) / 2);
	Output.PixelInten = I.at<double>(posY, posX);

	return Output;
}

double findMaxRadius(Mat _src)
{
	Mat I;
	_src.copyTo(I);
	//cvtColor(I, I, CV_RGB2GRAY);

	Mat bw = Mat::zeros(I.size(), CV_8UC1);
	threshold(I, bw, 127, 255, 0);
	morphologyEx(bw, bw, MORPH_CLOSE, getStructuringElement(MORPH_ELLIPSE, Size(20, 20)));
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours(bw, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

	/// Find maximun
	double MaxRadius = 0;
	for (int i = 0; i < contours.size(); i++)
	{
		double tempR = sqrt(contourArea(contours.at(i)) / 3.14159);
		if (tempR > MaxRadius)
		{
			MaxRadius = tempR;
		}
	}
	return MaxRadius;

}