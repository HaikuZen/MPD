// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright The Music Player Daemon Project

#include "DetectFilterFormat.hxx"
#include "Frame.hxx"
#include "SampleFormat.hxx"
#include "pcm/Silence.hxx"
#include "pcm/CheckAudioFormat.hxx"

extern "C" {
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
}

#include <cassert>

namespace Ffmpeg {

AudioFormat
DetectFilterOutputFormat(const AudioFormat &in_audio_format,
			 AVFilterContext &buffer_src,
			 AVFilterContext &buffer_sink)
{
	uint_least64_t silence[MAX_CHANNELS];
	const size_t silence_size = in_audio_format.GetFrameSize();
	assert(sizeof(silence) >= silence_size);

	PcmSilence(std::as_writable_bytes(std::span{silence}),
		   in_audio_format.format);

	Frame frame;
	frame->format = ToFfmpegSampleFormat(in_audio_format.format);
	frame->sample_rate = in_audio_format.sample_rate;
#if LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(57, 25, 100)
	av_channel_layout_default(&frame->ch_layout, in_audio_format.channels);
#else
	frame->channels = in_audio_format.channels;
#endif
	frame->nb_samples = 1;

	frame.GetBuffer();

	memcpy(frame.GetData(0), silence, silence_size);

	int err = av_buffersrc_add_frame(&buffer_src, frame.get());
	if (err < 0)
		throw MakeFfmpegError(err, "av_buffersrc_add_frame() failed");

	frame.Unref();

	err = av_buffersink_get_frame(&buffer_sink, frame.get());
	if (err < 0) {
		if (err == AVERROR(EAGAIN))
			/* one sample was not enough input data for
			   the given filter graph */
			return AudioFormat::Undefined();

		throw MakeFfmpegError(err, "av_buffersink_get_frame() failed");
	}

	const SampleFormat sample_format = FromFfmpegSampleFormat(AVSampleFormat(frame->format));
	if (sample_format == SampleFormat::UNDEFINED)
		throw std::runtime_error("Unsupported FFmpeg sample format");

#if LIBAVUTIL_VERSION_INT >= AV_VERSION_INT(57, 25, 100)
	const unsigned out_channels = frame->ch_layout.nb_channels;
#else
	const unsigned out_channels = frame->channels;
#endif

	return CheckAudioFormat(frame->sample_rate, sample_format,
				out_channels);
}

} // namespace Ffmpeg
