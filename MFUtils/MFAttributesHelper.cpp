﻿#include "pch.h"
using namespace Platform;
using namespace Windows::Storage::Streams;
using namespace Microsoft::WRL;

// Only throw in exception based code (C++/CX), never throw in HRESULT error code based code.
#define THROW_IF_FAILED(hr)     { if (FAILED(hr)) throw Platform::Exception::CreateException(hr); }

namespace MFUtils
{
    // This WinRT object provides JavaScript or C# code access to the information in the stream
    // that it needs to construct the AudioEncodingProperties needed to construct the AudioStreamDescriptor
    // needed to create a MediaStreamSource. Here is how to create it
    // var helper = new MFUtils.MFAttributesHelper(self.memoryStream, data.mimeType);

    public ref class MFAttributesHelper sealed
    {
    public:
        property UINT64 Duration;
        property UINT32 BitRate;
        property UINT32 SampleRate;
        property UINT32 ChannelCount;

        // The synchronous design only works with in memory streams.
        MFAttributesHelper(InMemoryRandomAccessStream^ stream, String^ mimeType)
        {
            THROW_IF_FAILED(MFStartup(MF_VERSION));
            // create an IMFByteStream from "stream"
            ComPtr<IMFByteStream> byteStream;
            THROW_IF_FAILED(MFCreateMFByteStreamOnStreamEx(reinterpret_cast<IUnknown*>(stream), &byteStream));

            // assign mime type to the attributes on this byte stream
            ComPtr<IMFAttributes> attributes;
            THROW_IF_FAILED(byteStream.As(&attributes));
            THROW_IF_FAILED(attributes->SetString(MF_BYTESTREAM_CONTENT_TYPE, mimeType->Data()));

            // create a source reader from the byte stream
            ComPtr<IMFSourceReader> sourceReader;
            THROW_IF_FAILED(MFCreateSourceReaderFromByteStream(byteStream.Get(), nullptr, &sourceReader));

            // get current media type
            ComPtr<IMFMediaType> mediaType;
            THROW_IF_FAILED(sourceReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, &mediaType));

            // get all the data we're looking for
            PROPVARIANT prop;
            THROW_IF_FAILED(sourceReader->GetPresentationAttribute(MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &prop));
            Duration = prop.uhVal.QuadPart;

            UINT32 data;
            THROW_IF_FAILED(sourceReader->GetPresentationAttribute(MF_SOURCE_READER_MEDIASOURCE, MF_PD_AUDIO_ENCODING_BITRATE, &prop));
            BitRate = prop.ulVal;

            THROW_IF_FAILED(mediaType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &data));
            SampleRate = data;

            THROW_IF_FAILED(mediaType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &data));
            ChannelCount = data;
        }

    private:
        ~MFAttributesHelper()
        {
            MFShutdown();
        }
    };
}
