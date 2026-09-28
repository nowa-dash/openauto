/*
*  This file is part of openauto project.
*  Copyright (C) 2018 f1x.studio (Michal Szwaj)
*
*  openauto is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 3 of the License, or
*  (at your option) any later version.

*  openauto is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with openauto. If not, see <http://www.gnu.org/licenses/>.
*/

#include <QApplication>
#include <gst/app/gstappsrc.h>
#include <cstring>
#include <string>
#include <f1x/openauto/autoapp/Projection/QtVideoOutput.hpp>
#include <f1x/openauto/Common/Log.hpp>

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace projection
{

namespace
{

// Which H.264 decoder to use is a property of the machine we run on, not something
// that can be hardcoded: v4l2h264dec only exists on Raspberry Pi style V4L2 hosts and
// is the only hardware accelerated option, while x86 hosts fall back to a software
// decoder. Software decoders hand out system memory, which GstGL sinks cannot consume
// directly and which therefore need a videoconvert bridge; dmabuf output goes straight
// into the sink instead.
struct PipelineCandidate
{
    const char* element;
    const char* properties;
    const char* tail;
};

const PipelineCandidate PIPELINE_CANDIDATES[] = {
    { "v4l2h264dec", "capture-io-mode=dmabuf", "" },
    { "v4l2slh264dec", "capture-io-mode=dmabuf", "" },
    { "avdec_h264", "", "videoconvert" },
    { "openh264dec", "", "videoconvert" },
};

std::string buildPipelineDescription(const PipelineCandidate& candidate)
{
    std::string description = "appsrc name=src is-live=true block=false format=time ! h264parse ! ";
    description += candidate.element;

    if (candidate.properties[0] != '\0')
    {
        description += ' ';
        description += candidate.properties;
    }

    if (candidate.tail[0] != '\0')
    {
        description += " ! ";
        description += candidate.tail;
    }

    description += " ! waylandsink sync=false";
    return description;
}

GstBusSyncReply onBusMessage(GstBus*, GstMessage* message, gpointer)
{
    if (GST_MESSAGE_TYPE(message) != GST_MESSAGE_ERROR)
    {
        return GST_BUS_PASS;
    }

    GError* gstError = nullptr;
    gchar* debugInfo = nullptr;
    gst_message_parse_error(message, &gstError, &debugInfo);

    OPENAUTO_LOG(error) << "[QtVideoOutput] pipeline error: " << (gstError != nullptr ? gstError->message : "unknown");

    if (debugInfo != nullptr)
    {
        OPENAUTO_LOG(debug) << "[QtVideoOutput] " << debugInfo;
        g_free(debugInfo);
    }

    g_clear_error(&gstError);
    return GST_BUS_DROP;
}

}

QtVideoOutput::QtVideoOutput(configuration::IConfiguration::Pointer configuration)
    : VideoOutput(std::move(configuration))
{
    this->moveToThread(QApplication::instance()->thread());
    connect(this, &QtVideoOutput::startPlayback, this, &QtVideoOutput::onStartPlayback, Qt::QueuedConnection);
    connect(this, &QtVideoOutput::stopPlayback, this, &QtVideoOutput::onStopPlayback, Qt::QueuedConnection);

    static bool gstInited = false;
    if (!gstInited)
    {
        gst_init(nullptr, nullptr);
        gstInited = true;
    }
}

bool QtVideoOutput::open()
{
    return true;
}

bool QtVideoOutput::init()
{
    emit startPlayback();
    return true;
}

void QtVideoOutput::stop()
{
    emit stopPlayback();
}

void QtVideoOutput::write(uint64_t, const aasdk::common::DataConstBuffer& buffer)
{
    std::lock_guard<std::mutex> lock(gstMutex_);

    if (appsrc_ == nullptr)
    {
        return;
    }

    GstBuffer* gstBuf = gst_buffer_new_allocate(nullptr, buffer.size, nullptr);
    GstMapInfo map;
    gst_buffer_map(gstBuf, &map, GST_MAP_WRITE);
    std::memcpy(map.data, buffer.cdata, buffer.size);
    gst_buffer_unmap(gstBuf, &map);

    gst_app_src_push_buffer(GST_APP_SRC(appsrc_), gstBuf);
}

void QtVideoOutput::onStartPlayback()
{
    std::lock_guard<std::mutex> lock(gstMutex_);

    OPENAUTO_LOG(debug) << "[QtVideoOutput] starting gstreamer pipeline.";

    for (const auto& candidate : PIPELINE_CANDIDATES)
    {
        if (gst_element_factory_find(candidate.element) == nullptr)
        {
            continue;
        }

        const std::string description = buildPipelineDescription(candidate);

        GError* error = nullptr;
        pipeline_ = gst_parse_launch(description.c_str(), &error);

        if (pipeline_ != nullptr && error == nullptr)
        {
            OPENAUTO_LOG(info) << "[QtVideoOutput] pipeline: " << description;
            break;
        }

        OPENAUTO_LOG(debug) << "[QtVideoOutput] unusable pipeline (" << candidate.element
                            << "): " << (error != nullptr ? error->message : "no pipeline produced");

        g_clear_error(&error);
        pipeline_ = nullptr;
    }

    if (pipeline_ == nullptr)
    {
        OPENAUTO_LOG(error) << "[QtVideoOutput] no usable H.264 decoder found, video disabled.";
        return;
    }

    appsrc_ = gst_bin_get_by_name(GST_BIN(pipeline_), "src");

    if (appsrc_ == nullptr)
    {
        OPENAUTO_LOG(error) << "[QtVideoOutput] appsrc 'src' not found in pipeline.";
        gst_element_set_state(pipeline_, GST_STATE_NULL);
        gst_object_unref(pipeline_);
        pipeline_ = nullptr;
        return;
    }

    GstCaps* caps = gst_caps_new_simple("video/x-h264",
        "stream-format", G_TYPE_STRING, "byte-stream",
        "alignment", G_TYPE_STRING, "au",
        nullptr);
    g_object_set(appsrc_, "caps", caps, nullptr);
    gst_caps_unref(caps);

    GstBus* bus = gst_element_get_bus(pipeline_);
    gst_bus_set_sync_handler(bus, onBusMessage, nullptr, nullptr);
    gst_object_unref(bus);

    gst_element_set_state(pipeline_, GST_STATE_PLAYING);
}

void QtVideoOutput::onStopPlayback()
{
    std::lock_guard<std::mutex> lock(gstMutex_);

    if (pipeline_ != nullptr)
    {
        gst_element_set_state(pipeline_, GST_STATE_NULL);

        if (appsrc_ != nullptr)
        {
            gst_object_unref(appsrc_);
            appsrc_ = nullptr;
        }

        gst_object_unref(pipeline_);
        pipeline_ = nullptr;
    }
}

}
}
}
}
