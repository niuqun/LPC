#include "track.h"

static int getOpSize(int size, int *idx)
{
	int i;
	for (i=0; i<800; i++)
	{	
		if ( optSize[i] >= size )
		{
			size = optSize[i];
			*idx=i;
			break;
		}
	}
	return size;
}

CTrack::CTrack()
{
	bInit=false;
	mWidth=0;
	mHeight=0;
}

CTrack::~CTrack()
{
	if ( mSigma != NULL )
	{
		delete mSigma;
	}
	if ( fgProb != NULL )
	{
		delete fgProb;
	}
}

void CTrack::initialize()
{
	if ( bInit )
	{
		return;
	}

	alpha = 2.25;
	beta = 1.5;
	rho = 0.075;

	mHeight=288;
	mWidth=360;

	mSigma=new float[mWidth*mHeight];
	for ( int i=0;i<mWidth;i++ )
	{
		for ( int j=0;j<mHeight;j++ )
		{
			mSigma[j*mWidth+i]=0.5*exp(-(pow((i-mWidth/2),2.0)+pow((j-mHeight/2),2.0))/(2*119*119));
		}
	}

	Mat c=imread("color.jpg",1);
	colorTable=c.clone().col(0);
	confidenceRecord=Mat::zeros(mHeight,mWidth,CV_8UC1);
	confidence=Mat::zeros(mHeight,mWidth,CV_8UC3);
	fgProb=new float[mHeight*mWidth];

	bInit=true;
}

bool CTrack::isInitialized()
{
	return bInit;
}

void CTrack::resetTrackResult()
{
	mTracking.clear();
}

void CTrack::track( vector<ObjectProps> trackQueue, unsigned char* lastFrame, unsigned char* currentFrame )
{
	if ( !isInitialized() )
	{
		return;
	}

	resetTrackResult();
	mTracking=trackQueue;
	newTrack(mTracking,lastFrame,currentFrame);
}

void CTrack::newTrack( vector<ObjectProps> &trackQueue, unsigned char* lastFrame, unsigned char* currentFrame )
{
	confidenceRecord.setTo(0);
	confidence.setTo(0);

	stcInit(trackQueue,lastFrame);
	stcTrack(trackQueue,currentFrame);

#ifdef _DEBUG_IMAGE
	for ( int i=0;i<mHeight;i++ )
	{
		for ( int j=0;j<mWidth;j++ )
		{
			int idx=confidenceRecord.at<unsigned char>(i,j);
			confidence.at<Vec3b>(i,j)[0]=colorTable.at<Vec3b>(idx,0)[0];
			confidence.at<Vec3b>(i,j)[1]=colorTable.at<Vec3b>(idx,0)[1];
			confidence.at<Vec3b>(i,j)[2]=colorTable.at<Vec3b>(idx,0)[2];
		}
	}
	imshow("confidence",confidence);
	cv::moveWindow("confidence",550,300);
#endif // _DEBUG_IMAGE

}

void CTrack::stcInit(vector<ObjectProps>& trackingQueue, unsigned char* lastFrame)
{
	Mat imgLog=Mat::zeros(mHeight,mWidth,CV_32FC1);
	Mat img=Mat::zeros(mHeight,mWidth,CV_8UC1);
	Mat imgLBPC=Mat::zeros(mHeight,mWidth,CV_32FC1);
	memcpy(img.data,lastFrame,sizeof(unsigned char)*mHeight*mWidth);
	img.convertTo(imgLog,CV_32FC1);
	add(imgLog,1,imgLog);
	log(imgLog,imgLog);
	normalize(imgLog,imgLog,1,0,CV_MINMAX);
	medianBlur(imgLog,imgLog,3);
	imgLog.convertTo(img,CV_8UC1,255);
	img.convertTo(imgLBPC,CV_32FC1,(1+LBP_C));

	for ( vector<ObjectProps>::iterator itor=trackingQueue.begin();itor!=trackingQueue.end();itor++ )
	{
		if ( itor->bInit==false && itor->corrected ==false )
		{
			// the context region
			itor->center.x=(int)itor->rects.back().x+itor->rects.back().width/2;
			itor->center.y=(int)itor->rects.back().y+itor->rects.back().height/2;

			int ctxW=itor->rects.back().width*2;
			int ctxH=itor->rects.back().height*2;
			int ctxX=itor->center.x-ctxW/2;
			int ctxY=itor->center.y-ctxH/2;

			if ( ctxX < 0 )
			{
				ctxX=0;
			}
			if ( ctxY < 0 )
			{
				ctxY=0;
			}

			int opWidx=0,opHidx=0;
			ctxW=getOpSize(ctxW,&opWidx);
			ctxH=getOpSize(ctxH,&opHidx);

			while ( ctxX+ctxW >= mWidth && --opWidx >= 0 )
			{
				ctxW=optSize[opWidx];
			}
			while ( ctxY+ctxH >= mHeight && --opHidx >= 0 )
			{
				ctxH=optSize[opHidx];
			}
			itor->cxtRegion.x=ctxX;
			itor->cxtRegion.y=ctxY;
			itor->cxtRegion.width=ctxW;
			itor->cxtRegion.height=ctxH;

			itor->context=Mat::zeros(itor->cxtRegion.height, itor->cxtRegion.width, CV_64FC1);
			itor->cxtPriorPro=Mat::zeros(itor->cxtRegion.height, itor->cxtRegion.width, CV_64FC1);
			itor->cxtConf=Mat::zeros(itor->cxtRegion.height, itor->cxtRegion.width, CV_64FC1);
			itor->STModel=Mat::zeros(itor->cxtRegion.height, itor->cxtRegion.width, CV_64FC1);
			itor->STCModel=Mat::zeros(itor->cxtRegion.height, itor->cxtRegion.width, CV_64FC1);

			itor->sigma=mSigma[itor->center.y*mWidth+itor->center.x]*(itor->rects.back().width+itor->rects.back().height);

			//Hamming window
			itor->hammingWin=Mat::zeros(itor->cxtRegion.height, itor->cxtRegion.width, CV_64FC1);
			for ( int i=0;i<itor->hammingWin.rows;i++ )
			{
				for ( int j=0;j<itor->hammingWin.cols;j++ )
				{
					itor->hammingWin.at<double>(i,j) = (0.54 - 0.46 * cos( 2 * CV_PI * i / itor->hammingWin.rows )) 
						* (0.54 - 0.46 * cos( 2 * CV_PI * j / itor->hammingWin.cols ));
				}
			}

			//////////////////////////////////////////////////////////////////////////
			//带参数的lbp提取纹理
			//////////////////////////////////////////////////////////////////////////
			for ( int i=1;i<itor->cxtRegion.width-1;i++ )
			{
				for ( int j=1;j<itor->cxtRegion.height-1;j++ )
				{
					int ofsX=i+itor->cxtRegion.x;
					int ofsY=j+itor->cxtRegion.y;
					int c1=img.at<unsigned char>(ofsY,ofsX)>imgLBPC.at<float>(ofsY-1,ofsX-1) ? 128 : 0;
					int c2=img.at<unsigned char>(ofsY,ofsX)>imgLBPC.at<float>(ofsY-1,ofsX) ? 64 : 0;
					int c3=img.at<unsigned char>(ofsY,ofsX)>imgLBPC.at<float>(ofsY-1,ofsX+1) ? 32 : 0;
					int c4=img.at<unsigned char>(ofsY,ofsX)>imgLBPC.at<float>(ofsY,ofsX-1) ? 16 : 0;
					int c5=img.at<unsigned char>(ofsY,ofsX)>imgLBPC.at<float>(ofsY,ofsX+1) ? 8 : 0;
					int c6=img.at<unsigned char>(ofsY,ofsX)>imgLBPC.at<float>(ofsY+1,ofsX-1) ? 4 : 0;
					int c7=img.at<unsigned char>(ofsY,ofsX)>imgLBPC.at<float>(ofsY+1,ofsX) ? 2 : 0;
					int c8=img.at<unsigned char>(ofsY,ofsX)>imgLBPC.at<float>(ofsY+1,ofsX+1) ? 1 : 0;
					int c=c1|c2|c3|c4|c5|c6|c7|c8;

					if ( fgProb[ofsY*mWidth+ofsX]>0 )
					{
						itor->context.at<double>(j,i)=fgProb[ofsY*mWidth+ofsX]*c;
					}
					else
					{
						itor->context.at<double>(j,i)=c;
					}
				}
			}
			//////////////////////////////////////////////////////////////////////////

			itor->context=itor->context.mul(itor->hammingWin);
			learnSTCModel(*itor);
			itor->bInit=true;
		}
	}
}

void CTrack::learnSTCModel(ObjectProps &trackingObj)
{
	// step 1: Get context prior and posterior probability
	getCxtPriorPosteriorModel(trackingObj);

	// step 2-1: Execute 2D DFT for prior probability
	Mat priorFourier;
	Mat planes1[] = {trackingObj.cxtPriorPro, Mat::zeros(trackingObj.cxtPriorPro.size(), CV_64F)};
	merge(planes1, 2, priorFourier);
	dft(priorFourier, priorFourier);

	// step 2-2: Execute 2D DFT for posterior probability
	Mat postFourier; 
	Mat planes2[] = {trackingObj.cxtConf, Mat::zeros(trackingObj.cxtConf.size(), CV_64F)};
	merge(planes2, 2, postFourier);
	dft(postFourier, postFourier);

	// step 3: Calculate the division
	Mat conditionalFourier;
	complexOperation(postFourier, priorFourier, conditionalFourier, 1);

	// step 4: Execute 2D inverse DFT for conditional probability and we obtain STModel
	dft(conditionalFourier, trackingObj.STModel, DFT_INVERSE | DFT_REAL_OUTPUT | DFT_SCALE);

	// step 5: Use the learned spatial context model to update spatio-temporal context model
	cv::addWeighted(trackingObj.STCModel, 1.0 - rho, trackingObj.STModel, rho, 0.0, trackingObj.STCModel);
}

void CTrack::complexOperation(const Mat src1, const Mat src2, Mat &dst, int flag )
{
	Mat A_Real, A_Imag, B_Real, B_Imag, R_Real, R_Imag;
	vector<Mat> planes;
	split(src1, planes);
	planes[0].copyTo(A_Real);
	planes[1].copyTo(A_Imag);

	split(src2, planes);
	planes[0].copyTo(B_Real);
	planes[1].copyTo(B_Imag);

	dst.create(src1.rows, src1.cols, CV_64FC2);
	split(dst, planes);
	R_Real = planes[0];
	R_Imag = planes[1];

	for (int i = 0; i < A_Real.rows; i++)
	{
		for (int j = 0; j < A_Real.cols; j++)
		{
			double a = A_Real.at<double>(i, j);
			double b = A_Imag.at<double>(i, j);
			double c = B_Real.at<double>(i, j);
			double d = B_Imag.at<double>(i, j);

			if (flag)
			{
				// division: (a+bj) / (c+dj)
				R_Real.at<double>(i, j) = (a * c + b * d) / (c * c + d * d + 0.000001);
				R_Imag.at<double>(i, j) = (b * c - a * d) / (c * c + d * d + 0.000001);
			}
			else
			{
				// multiplication: (a+bj) * (c+dj)
				R_Real.at<double>(i, j) = a * c - b * d;
				R_Imag.at<double>(i, j) = b * c + a * d;
			}
		}
	}
	merge(planes, dst);
}

void CTrack::getCxtPriorPosteriorModel(ObjectProps &trackingObj)
{
	double sum_prior(0), sum_post(0);
	for (int i = 0; i < trackingObj.cxtRegion.height; i++)
	{
		for (int j = 0; j < trackingObj.cxtRegion.width; j++)
		{
			double x = j + trackingObj.cxtRegion.x;
			double y = i + trackingObj.cxtRegion.y;
			
			double dist = sqrt((trackingObj.center.x - x) * (trackingObj.center.x - x)
				+ (trackingObj.center.y - y) * (trackingObj.center.y - y));

			trackingObj.cxtPriorPro.at<double>(i, j) = exp(- dist * dist / (2 * trackingObj.sigma * trackingObj.sigma));
			sum_prior += trackingObj.cxtPriorPro.at<double>(i, j);

			trackingObj.cxtConf.at<double>(i, j) = exp(- pow(dist / sqrt(alpha), beta));
			sum_post += trackingObj.cxtConf.at<double>(i, j);
		}
	}
	trackingObj.cxtPriorPro.convertTo(trackingObj.cxtPriorPro, -1, 1.0/sum_prior);
	trackingObj.cxtPriorPro = trackingObj.cxtPriorPro.mul(trackingObj.context);
	trackingObj.cxtConf.convertTo(trackingObj.cxtConf, -1, 1.0/sum_post);
}

void CTrack::stcTrack(vector<ObjectProps>& trackingQueue, unsigned char* currentFrame)
{
	Mat imgLog=Mat::zeros(mHeight,mWidth,CV_32FC1);
	Mat img=Mat::zeros(mHeight,mWidth,CV_8UC1);
	Mat imgLBPC=Mat::zeros(mHeight,mWidth,CV_32FC1);
	memcpy(img.data,currentFrame,sizeof(unsigned char)*mHeight*mWidth);
	img.convertTo(imgLog,CV_32FC1);
	add(imgLog,1,imgLog);
	log(imgLog,imgLog);
	normalize(imgLog,imgLog,1,0,CV_MINMAX);
	medianBlur(imgLog,imgLog,3);
	imgLog.convertTo(img,CV_8UC1,255);
	img.convertTo(imgLBPC,CV_32FC1,(1+LBP_C));

#ifdef _DEBUG_IMAGE
	Mat testShowLBPC=Mat::zeros(mHeight,mWidth,CV_8UC3);
	Mat showFgP=Mat::zeros(mHeight,mWidth,CV_8UC3);
	for ( int i=1;i<mHeight-1;i++ )
	{
		for ( int j=1;j<mWidth-1;j++ )
		{
			int c1=img.at<unsigned char>(i,j)>imgLBPC.at<float>(i-1,j-1) ? 128 : 0;
			int c2=img.at<unsigned char>(i,j)>imgLBPC.at<float>(i-1,j) ? 64 : 0;
			int c3=img.at<unsigned char>(i,j)>imgLBPC.at<float>(i-1,j+1) ? 32 : 0;
			int c4=img.at<unsigned char>(i,j)>imgLBPC.at<float>(i,j-1) ? 16 : 0;
			int c5=img.at<unsigned char>(i,j)>imgLBPC.at<float>(i,j+1) ? 8 : 0;
			int c6=img.at<unsigned char>(i,j)>imgLBPC.at<float>(i+1,j-1) ? 4 : 0;
			int c7=img.at<unsigned char>(i,j)>imgLBPC.at<float>(i+1,j) ? 2 : 0;
			int c8=img.at<unsigned char>(i,j)>imgLBPC.at<float>(i+1,j+1) ? 1 : 0;
			int c=c1|c2|c3|c4|c5|c6|c7|c8;

			//testShowLBPC.at<unsigned char>(i,j)=c;
			testShowLBPC.at<Vec3b>(i,j)[0]=colorTable.at<Vec3b>(c,0)[0];
			testShowLBPC.at<Vec3b>(i,j)[1]=colorTable.at<Vec3b>(c,0)[1];
			testShowLBPC.at<Vec3b>(i,j)[2]=colorTable.at<Vec3b>(c,0)[2];

// 			int idx=img.at<unsigned char>(i,j);
// 			showLOG.at<Vec3b>(i,j)[0]=colorTable.at<Vec3b>(idx,0)[0];
// 			showLOG.at<Vec3b>(i,j)[1]=colorTable.at<Vec3b>(idx,0)[1];
// 			showLOG.at<Vec3b>(i,j)[2]=colorTable.at<Vec3b>(idx,0)[2];

			int idx=(fgProb[i*mWidth+j]-1.2)/0.3*255;
			//int idx=fgProb[i*mWidth+j];
			showFgP.at<Vec3b>(i,j)[0]=colorTable.at<Vec3b>(idx,0)[0];
			showFgP.at<Vec3b>(i,j)[1]=colorTable.at<Vec3b>(idx,0)[1];
			showFgP.at<Vec3b>(i,j)[2]=colorTable.at<Vec3b>(idx,0)[2];

		}
	}
	imshow("fgp",showFgP);
	imshow("lbpc",testShowLBPC);
	cv::moveWindow("lbpc",550+352+50,300+0+0);
	cv::moveWindow("fgp",550+0+0,300+288+50);

// 	imshow("log",showLOG);
// 	cv::moveWindow("log",550+0+0,300+288+50);
#endif



	for ( vector<ObjectProps>::iterator itor=trackingQueue.begin();itor!=trackingQueue.end();itor++)
	{
		if ( itor->bInit==true && itor->corrected ==false )
		{
			
			//////////////////////////////////////////////////////////////////////////
			//带参数的lbp提取纹理
			//////////////////////////////////////////////////////////////////////////
			for ( int i=1;i<itor->cxtRegion.width-1;i++ )
			{
				for ( int j=1;j<itor->cxtRegion.height-1;j++ )
				{
					int ofsX=i+itor->cxtRegion.x;
					int ofsY=j+itor->cxtRegion.y;
					int c1=img.at<unsigned char>(ofsY,ofsX)>imgLBPC.at<float>(ofsY-1,ofsX-1) ? 128 : 0;
					int c2=img.at<unsigned char>(ofsY,ofsX)>imgLBPC.at<float>(ofsY-1,ofsX) ? 64 : 0;
					int c3=img.at<unsigned char>(ofsY,ofsX)>imgLBPC.at<float>(ofsY-1,ofsX+1) ? 32 : 0;
					int c4=img.at<unsigned char>(ofsY,ofsX)>imgLBPC.at<float>(ofsY,ofsX-1) ? 16 : 0;
					int c5=img.at<unsigned char>(ofsY,ofsX)>imgLBPC.at<float>(ofsY,ofsX+1) ? 8 : 0;
					int c6=img.at<unsigned char>(ofsY,ofsX)>imgLBPC.at<float>(ofsY+1,ofsX-1) ? 4 : 0;
					int c7=img.at<unsigned char>(ofsY,ofsX)>imgLBPC.at<float>(ofsY+1,ofsX) ? 2 : 0;
					int c8=img.at<unsigned char>(ofsY,ofsX)>imgLBPC.at<float>(ofsY+1,ofsX+1) ? 1 : 0;
					int c=c1|c2|c3|c4|c5|c6|c7|c8;
					if ( fgProb[ofsY*mWidth+ofsX]>0 )
					{
						itor->context.at<double>(j,i)=fgProb[ofsY*mWidth+ofsX]*c;
					}
					else
					{
						itor->context.at<double>(j,i)=c;
					}
					//itor->context.at<double>(j,i)=c;
				}
			}
			//////////////////////////////////////////////////////////////////////////

			itor->context=itor->context.mul(itor->hammingWin);
			getCxtPriorPosteriorModel(*itor);

			Mat priorFourier;
			Mat planes1[] = {itor->cxtPriorPro, Mat::zeros(itor->cxtPriorPro.size(), CV_64F)};
			merge(planes1, 2, priorFourier);
			dft(priorFourier, priorFourier);
			
			Mat STCModelFourier;
			Mat planes2[] = {itor->STCModel, Mat::zeros(itor->STCModel.size(), CV_64F)};
			merge(planes2, 2, STCModelFourier);
			dft(STCModelFourier, STCModelFourier);

			Mat postFourier;
			complexOperation(STCModelFourier, priorFourier, postFourier, 0);

			Mat confidenceMap;
			dft(postFourier, confidenceMap, DFT_INVERSE | DFT_REAL_OUTPUT| DFT_SCALE);	

			normalize(confidenceMap,confidenceMap,1,0,CV_MINMAX);

#ifdef _DEBUG_IMAGE
			for ( int i=0;i<itor->cxtRegion.width;i++ )
			{
				for ( int j=0;j<itor->cxtRegion.height;j++ )
				{
					int tmp=confidenceMap.at<double>(j,i)*255;
					if ( tmp>255 )
					{
						tmp=255;
					}
					confidenceRecord.at<unsigned char>(j+itor->cxtRegion.y,i+itor->cxtRegion.x)=
						tmp>confidenceRecord.at<unsigned char>(j+itor->cxtRegion.y,i+itor->cxtRegion.x)?
						tmp:confidenceRecord.at<unsigned char>(j+itor->cxtRegion.y,i+itor->cxtRegion.x);		
				}
			}
#endif // _DEBUG_IMAGE

			Mat	kern=(Mat_<int>(3,3)<<1,1,1,1,1,1,1,1,1);
			filter2D(confidenceMap,confidenceMap,confidenceMap.depth(),kern);

			Point point;
			minMaxLoc(confidenceMap, 0, 0, 0, &point);

			itor->center.x=itor->cxtRegion.x+point.x;
			itor->center.y=itor->cxtRegion.y+point.y;

			Rect oc;
			oc.height=itor->rects.back().height;
			oc.width=itor->rects.back().width;

			oc.x=(float)itor->cxtRegion.x+point.x-oc.width/2;
			oc.y=(float)itor->cxtRegion.y+point.y-oc.height/2;
			itor->rects.push_back(oc);

			int ctxX=itor->center.x-itor->cxtRegion.width/2;
			int ctxY=itor->center.y-itor->cxtRegion.height/2;
			int ctxW=itor->cxtRegion.width;
			int ctxH=itor->cxtRegion.height;
			if ( ctxX < 0 )
			{
				ctxX=0;
			}
			if ( ctxY < 0 )
			{
				ctxY=0;
			}

			int opWidx=0,opHidx=0;
			ctxW=getOpSize(ctxW,&opWidx);
			ctxH=getOpSize(ctxH,&opHidx);

			while ( ctxX+ctxW >= mWidth && --opWidx >= 0 )
			{
				ctxW=optSize[opWidx];
			}
			while ( ctxY+ctxH >= mHeight && --opHidx >= 0 )
			{
				ctxH=optSize[opHidx];
			}
			itor->cxtRegion.x=ctxX;
			itor->cxtRegion.y=ctxY;
			itor->cxtRegion.width=ctxW;
			itor->cxtRegion.height=ctxH;
 
			for ( int i=1;i<itor->cxtRegion.width-1;i++ )
			{
				for ( int j=1;j<itor->cxtRegion.height-1;j++ )
				{
					int ofsX=i+itor->cxtRegion.x;
					int ofsY=j+itor->cxtRegion.y;
					int c1=img.at<unsigned char>(ofsY,ofsX)>imgLBPC.at<float>(ofsY-1,ofsX-1) ? 128 : 0;
					int c2=img.at<unsigned char>(ofsY,ofsX)>imgLBPC.at<float>(ofsY-1,ofsX) ? 64 : 0;
					int c3=img.at<unsigned char>(ofsY,ofsX)>imgLBPC.at<float>(ofsY-1,ofsX+1) ? 32 : 0;
					int c4=img.at<unsigned char>(ofsY,ofsX)>imgLBPC.at<float>(ofsY,ofsX-1) ? 16 : 0;
					int c5=img.at<unsigned char>(ofsY,ofsX)>imgLBPC.at<float>(ofsY,ofsX+1) ? 8 : 0;
					int c6=img.at<unsigned char>(ofsY,ofsX)>imgLBPC.at<float>(ofsY+1,ofsX-1) ? 4 : 0;
					int c7=img.at<unsigned char>(ofsY,ofsX)>imgLBPC.at<float>(ofsY+1,ofsX) ? 2 : 0;
					int c8=img.at<unsigned char>(ofsY,ofsX)>imgLBPC.at<float>(ofsY+1,ofsX+1) ? 1 : 0;
					int c=c1|c2|c3|c4|c5|c6|c7|c8;

					if ( fgProb[ofsY*mWidth+ofsX]>0 )
					{
						itor->context.at<double>(j,i)=fgProb[ofsY*mWidth+ofsX]*c;
					}
					else
					{
						itor->context.at<double>(j,i)=c;
					}
				}
			}
			//////////////////////////////////////////////////////////////////////////

			itor->context=itor->context.mul(itor->hammingWin);
			learnSTCModel(*itor);
		}
	}
}

void CTrack::setFgProbability(float *fgP)
{
	if ( !bInit )
	{
		return;
	}
	if ( fgProb==NULL )
	{
		fgProb=new float[mWidth*mHeight];
	}
	memcpy(fgProb,fgP,sizeof(float)*mWidth*mHeight);
}
