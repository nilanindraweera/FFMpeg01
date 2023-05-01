#pragma once
#include <string>
#include <iostream>

extern "C"
{
#include <libavcodec/avcodec.h>
}

#define INBUF_SIZE 4096

class DecodeVideo
{
public:
	DecodeVideo(const std::string& inputFileName, const std::string& outputFileName)
		:m_inputFileName(inputFileName), m_outputFileName(outputFileName)
	{
	}
	~DecodeVideo()
	{
	}

	void decodeVideo()
	{
        std::cout << "====================== Decode Video ======================" << std::endl;

		if (m_ret = avformat_open_input(&m_avFormatContext, m_inputFileName.c_str(), nullptr, nullptr))
		{
            std::cout << "AVFormatContext cannot open for " << m_inputFileName << std::endl;
            destroy();
			return;
		}

		if ((m_ret = avformat_find_stream_info(m_avFormatContext, nullptr) < 0))
		{
            std::cout << "Stream info cannot find" << std::endl;
            destroy();
			return;
		}

		for (auto i = 0; i < m_avFormatContext->nb_streams; i++)
		{
			if (m_avFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
			{
                m_videoStreamIndex = i;
				break;
			}
		}

        if (m_videoStreamIndex < 0)
        {
            std::cout << "video stream not found" << std::endl;
            destroy();
            return;
        }

        // dump video stream info
        av_dump_format(m_avFormatContext, m_videoStreamIndex, m_inputFileName.c_str(), false);

        // allocate memory for codec context
        m_avCodecContext = avcodec_alloc_context3(nullptr);
        
        // retrieve code params from format context
        if (m_ret = avcodec_parameters_to_context(m_avCodecContext, m_avFormatContext->streams[m_videoStreamIndex]->codecpar) < 0)
        {
            std::cout << "codec info not found" << std::endl;
            destroy();
            return;
        }

        // find decoding codec
        m_decodingCodec = avcodec_find_decoder(m_avCodecContext->codec_id);
		if (m_ret = avcodec_parameters_to_context(m_avCodecContext, m_avFormatContext->streams[m_videoStreamIndex]->codecpar) < 0)
		{
			std::cout << "codec info not found" << std::endl;
			destroy();
			return;
		}

        // open decoding codec
        if (m_ret = avcodec_open2(m_avCodecContext, m_decodingCodec, nullptr) < 0)
        {
			std::cout << "Decoding codec cannot open" << std::endl;
			destroy();
			return;
        }

        // initialize avPacket
        m_avPacket = av_packet_alloc();
        //av_init_packet(m_avPacket);
		if (!m_avPacket)
		{
			std::cout << "AVPacket cannot initialize" << std::endl;
			destroy();
			return;
		}

        // initialize avFrame
        m_avFrame = av_frame_alloc();
        if (!m_avFrame)
        {
			std::cout << "AVFrame cannot initialize" << std::endl;
			destroy();
			return;
        }

        // open input file
        fopen_s(&m_inputFile, m_inputFileName.c_str(), "rb");
        if (!m_inputFile)
        {
			std::cout << "Input file cannot open" << std::endl;
			destroy();
			return;
        }

        // open output file
		fopen_s(&m_outputFile, m_outputFileName.c_str(), "wb");
		if (!m_outputFile)
		{
			std::cout << "Output file cannot create" << std::endl;
			destroy();
			return;
		}

        // read file 
        while (1)
        {
            if (m_ret = av_read_frame(m_avFormatContext, m_avPacket) < 0)
            {
				std::cout << "Frame cannot read" << std::endl;
				break; // may be EOF
            }
            if (m_avPacket->stream_index == m_videoStreamIndex)
            {
                decode();
            }

            av_packet_unref(m_avPacket);
        }

        decode();
	}

private:
	void decode()
    {        
        char buf[1024];
        int ret;

        ret = avcodec_send_packet(m_avCodecContext, m_avPacket);
        if (ret < 0) {
            fprintf(stderr, "Error sending a packet for decoding\n");
            exit(1);
        }

        while (ret >= 0) 
        {
            ret = avcodec_receive_frame(m_avCodecContext, m_avFrame);

            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                return;
            
            if (ret < 0) 
            {
                fprintf(stderr, "Error during decoding\n");
                exit(1);
            }

            printf("saving frame %d\n", m_avCodecContext->frame_num);
            fflush(stdout);

            /* the picture is allocated by the decoder. no need to free it */
            snprintf(buf, sizeof(buf), "%s-%d", m_inputFileName, m_avCodecContext->frame_num);
            pgm_save(m_avFrame->data[0], m_avFrame->linesize[0], m_avFrame->width, m_avFrame->height);
        }

        fclose(m_outputFile);
    }

	void pgm_save(unsigned char* buf, int wrap, int xsize, int ysize)
	{
		fprintf(m_outputFile, "P5\n%d %d\n%d\n", xsize, ysize, 255);

        //fopen_s(&m_outputFile, m_outputFileName.c_str(), "wb");

        for (int i = 0; i < ysize; i++)
        {
            fwrite(buf + i * wrap, 1, xsize, m_outputFile);
        }

		//fclose(m_outputFile);
	}

    void destroy()
    {
        if (m_inputFile)
        {
            fclose(m_inputFile);
        }
        if (m_outputFile)
        {
            fclose(m_outputFile);
        }
        if (m_avCodecContext)
        {
            avcodec_close(m_avCodecContext);
        }
        if (m_avFormatContext)
        {
            avformat_close_input(&m_avFormatContext);
        }
        if (m_avFrame)
        {
            av_frame_free(&m_avFrame);
        }
        if (m_avPacket)
        {
            av_packet_free(&m_avPacket);
        }
    }

private:
    std::string m_inputFileName;
    std::string m_outputFileName;

    int m_videoStreamIndex = -1;
    int m_ret;

    AVFormatContext* m_avFormatContext = nullptr;
    AVCodecContext* m_avCodecContext = nullptr;
    const AVCodec* m_decodingCodec = nullptr;

    FILE* m_inputFile = nullptr;
    FILE* m_outputFile = nullptr;

    AVFrame* m_avFrame = nullptr;
    AVPacket* m_avPacket = nullptr;

	//const AVCodec* m_codec;
	//AVCodecParserContext* m_parser;
	//AVCodecContext* m_avcodecContext = NULL;
	//FILE* m_file;
	//AVFrame* m_avframe;

	//uint8_t m_inbuf[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
	//uint8_t* m_data;
	//size_t   m_data_size;
	//int m_ret;
	//int m_endOfFile;
	//AVPacket* m_pkt;
};








//
//
//m_pkt = av_packet_alloc();
//if (!m_pkt) {
//    return;
//}
//
///* set end of buffer to 0 (this ensures that no over-reading happens for damaged MPEG streams) */
//memset(m_inbuf + INBUF_SIZE, 0, AV_INPUT_BUFFER_PADDING_SIZE);
//
///* find the MPEG-1 video decoder */
//m_codec = avcodec_find_decoder(AV_CODEC_ID_MPEG4);
//if (!m_codec) {
//    fprintf(stderr, "Codec not found\n");
//    exit(1);
//}
//
//m_parser = av_parser_init(m_codec->id);
//if (!m_parser) {
//    fprintf(stderr, "parser not found\n");
//    exit(1);
//}
//
//m_avcodecContext = avcodec_alloc_context3(m_codec);
//if (!m_avcodecContext) {
//    fprintf(stderr, "Could not allocate video codec context\n");
//    exit(1);
//}
//
///* For some codecs, such as msmpeg4 and mpeg4, width and height
//   MUST be initialized there because this information is not
//   available in the bitstream. */
//
//   /* open it */
//if (avcodec_open2(m_avcodecContext, m_codec, NULL) < 0) {
//    fprintf(stderr, "Could not open codec\n");
//    exit(1);
//}
//
//fopen_s(&m_file, m_inputFileName.c_str(), "rb");
//if (!m_file) {
//    fprintf(stderr, "Could not open %s\n", m_inputFileName);
//    exit(1);
//}
//
//m_avframe = av_frame_alloc();
//if (!m_avframe) {
//    fprintf(stderr, "Could not allocate video frame\n");
//    exit(1);
//}
//
//do {
//    /* read raw data from the input file */
//    m_data_size = fread(m_inbuf, 1, INBUF_SIZE, m_file);
//    if (ferror(m_file))
//    {
//        break;
//    }
//    m_endOfFile = !m_data_size;
//
//    /* use the parser to split the data into frames */
//    m_data = m_inbuf;
//    while (m_data_size > 0 || m_endOfFile) {
//        m_ret = av_parser_parse2(m_parser, m_avcodecContext, &m_pkt->data, &m_pkt->size,
//            m_data, m_data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
//        if (m_ret < 0) {
//            fprintf(stderr, "Error while parsing\n");
//            exit(1);
//        }
//        m_data += m_ret;
//        m_data_size -= m_ret;
//
//        if (m_pkt->size)
//            decode(m_avcodecContext, m_avframe, m_pkt, m_outputFileName);
//        else if (m_endOfFile)
//            break;
//    }
//} while (!m_endOfFile);
//
///* flush the decoder */
//decode(m_avcodecContext, m_avframe, NULL, m_outputFileName);
//
//fclose(m_file);
//
//av_parser_close(m_parser);
//avcodec_free_context(&m_avcodecContext);
//av_frame_free(&m_avframe);
//av_packet_free(&m_pkt);