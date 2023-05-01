#pragma once
#include <string>
#include <iostream>

extern "C"
{
#include <libavcodec/avcodec.h>
#include "libavformat/avformat.h"
#include "libavutil/dict.h"
}

class FileMetaData
{
public:
	FileMetaData(const std::string& fileName):m_fileName(fileName)
	{
		if (m_ret = avformat_open_input(&format_context, m_fileName.c_str(), nullptr, nullptr))
		{
			return;
		}
	}

	~FileMetaData()
	{
		avformat_close_input(&format_context);
	}

	void printMetaData()
	{
		std::cout << "====================== Meta data info ======================" << std::endl;
		while ((tag = av_dict_get(format_context->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
		{
			std::cout << tag->key << "=" << tag->value << std::endl;
		}
	}

	void printStreamInfo()
	{
		std::cout << "====================== Stream info ======================" << std::endl;
		if ((m_ret = avformat_find_stream_info(format_context, nullptr) < 0))
		{
			return;
		}

		for (auto i = 0; i < format_context->nb_streams; i++)
		{
			av_dump_format(format_context, i, m_fileName.c_str(), false);
		}
	}

	void printVideoStreamInfo()
	{
		std::cout << "====================== Video Stream info ======================" << std::endl;
		
		if (format_context->nb_streams == 0 && (m_ret = avformat_find_stream_info(format_context, nullptr) < 0))
		{
			return;
		}
		int videoStreamIndex = -1;

		for (auto i = 0; i < format_context->nb_streams; i++)
		{
			if (format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
			{
				videoStreamIndex = i;
				break;
			}
		}
		av_dump_format(format_context, videoStreamIndex, m_fileName.c_str(), false);
	}
private:
	std::string m_fileName;
	AVFormatContext* format_context = nullptr;
	AVDictionaryEntry* tag = nullptr;
	int m_ret;
};