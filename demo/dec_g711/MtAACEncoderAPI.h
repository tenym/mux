
#ifndef MtAACEncoder_API_H
#define	MtAACEncoder_API_H

#ifdef _WIN32
#define Mt_API  __declspec(dllexport)
#define Mt_APICALL  __stdcall
#else
#define Mt_API
#define Mt_APICALL 
#endif

#define Mt_Handle void*
#define MtAACEncoder_Handle void*

///* Audio Codec */
enum Law
{
	Law_ULaw	=	0, 		/**< U law */
	Law_ALaw	=	1, 		/**< A law */
	Law_PCM16	=	2, 		/**< 16 bit uniform PCM values. ԭʼ pcm ���� */  
	Law_G726	=	3		/**< G726 */
};

///* Rate Bits */
enum Rate
{
	Rate16kBits=2,	/**< 16k bits per second (2 bits per ADPCM sample) */
	Rate24kBits=3,	/**< 24k bits per second (3 bits per ADPCM sample) */
	Rate32kBits=4,	/**< 32k bits per second (4 bits per ADPCM sample) */
	Rate40kBits=5	/**< 40k bits per second (5 bits per ADPCM sample) */
};

typedef struct _g711param
{
	;
}G711Param;

typedef struct _g726param
{
	unsigned char ucRateBits;//Rate16kBits Rate24kBits Rate32kBits Rate40kBits
}G726Param;

typedef struct _initParam
{
	unsigned char	ucAudioCodec;			// Law_uLaw  Law_ALaw Law_PCM16 Law_G726
	unsigned char	ucAudioChannel;			//1
	unsigned int	u32AudioSamplerate;		//8000
	unsigned int	u32PCMBitSize;			//16
	union
	{
		G711Param g711param;
		G726Param g726param;
	};

}InitParam;

#ifdef __cplusplus
extern "C"
{
#endif
	/* ����AAC Encoder ����Ϊ���ֵ */
	Mt_API MtAACEncoder_Handle Mt_APICALL Mt_AACEncoder_Init(InitParam initPar);

	/* ����������ݣ����ر�������� */
	Mt_API int Mt_APICALL Mt_AACEncoder_Encode(MtAACEncoder_Handle handle, unsigned char* inbuf, unsigned int inlen, unsigned char* outbuf, unsigned int* outlen);

	/* �ͷ�AAC Encoder */
	Mt_API void Mt_APICALL Mt_AACEncoder_Release(MtAACEncoder_Handle handle);

#ifdef __cplusplus
}
#endif

#endif	/* MtAACEncoder_API_H */
