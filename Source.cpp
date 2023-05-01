#include <iostream>
#include "FileMetadata.h"
#include "DecodeVideo.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include "libavformat/avformat.h"
#include "libavutil/dict.h"


//#include <libavcodec/avcodec.h>
//#include <libavformat/avformat.h>
//#include <libswscale/swscale.h>
//#include <libavutil/pixfmt.h>
//#include <libavutil/imgutils.h>
}

const char* inputFileName = "C:\\Self-Study\\Video-Streaming\\Samples\\FFmpeg01\\Output\\MPEG4_Sample02.m4v";// "MPEG4_Sample01.m4v";
const char* outputFileName = "C:\\Self-Study\\Video-Streaming\\Samples\\FFmpeg01\\Output\\MPEG4_SampleOutput02.mp4";
int main()
{
	
	{
		FileMetaData* fileMetaData = nullptr;
		fileMetaData = new FileMetaData{ inputFileName };
		if (fileMetaData)
		{
			fileMetaData->printMetaData();
			fileMetaData->printStreamInfo();
			fileMetaData->printVideoStreamInfo();
		}
	}

	{
		DecodeVideo* decodeVideo = nullptr;
		decodeVideo = new DecodeVideo{ inputFileName , outputFileName };
		if (decodeVideo)
		{
			decodeVideo->decodeVideo();
		}
	}
	system("pause");
	return 0;
}

