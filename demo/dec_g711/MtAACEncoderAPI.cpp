#include "MtAACEncoderAPI.h"
#include "MtAACEncoder.h"
#include "condef.h"

Mt_API MtAACEncoder_Handle Mt_APICALL Mt_AACEncoder_Init(InitParam initPar)
{
    G7ToAac *encoder = new G7ToAac();
	InAudioInfo info(initPar );
	bool ret = encoder->init(info);
	if (!ret)
	{
		SAFE_DELETE_OBJ(encoder);
	}
    return encoder;
}

Mt_API int Mt_APICALL Mt_AACEncoder_Encode(MtAACEncoder_Handle handle, unsigned char* inbuf, unsigned int inlen, unsigned char* outbuf, unsigned int* outlen)
{
    if(handle == NULL)
    {
        return -1;
    }
    return ((G7ToAac*)handle)->aac_encode(inbuf, inlen, outbuf, outlen);
}

Mt_API void Mt_APICALL Mt_AACEncoder_Release(MtAACEncoder_Handle handle)
{
    if(handle != NULL)
    {
        delete ((G7ToAac*)handle);
    }
}


