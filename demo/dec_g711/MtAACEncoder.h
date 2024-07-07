#ifndef MtAACEncoder_H
#define	MtAACEncoder_H

#include "audio_buffer.h"
#include "IDecodeToPcm.h"
#include "PcmToAac.h"

class G7ToAac
{
public:
    G7ToAac();
    virtual ~G7ToAac();
    
	bool init();
	bool init(InAudioInfo info);
    
    int aac_encode(unsigned char* inbuf, unsigned int inlen, unsigned char* outbuf, unsigned int* outlen);

private:
	int aac_encode_obj(unsigned char* inbuf, unsigned int inlen, unsigned char* outbuf, unsigned int* outlen );
    
	bool CreateDecodePcm();
	bool CreateEncodeAac();
	bool CreateBuffer();
private:        
    int nRet;
    int nTmp;
    int nCount;
    int nStatus; 
    int nPCMRead;
 


    int m_nPCMBufferSize;
    unsigned char *m_pbPCMBuffer;

    unsigned long m_nMaxOutputBytes;
    unsigned char *m_pbAACBuffer;

    int m_nPCMSize;   
    unsigned char *m_pbPCMTmpBuffer; 

	unsigned char *m_pbG7FrameBuffer;
	unsigned long m_nG7FrameBufferSize;

    audio_buffer *m_audio_buffer_;
	//------
	InAudioInfo m_inAudioInfo;

	IDecodeToPcm* m_pDecodeToPcm;
	PcmToAac* m_pPCMToAAC;
};

#endif	/* MtAACEncoder_H */

