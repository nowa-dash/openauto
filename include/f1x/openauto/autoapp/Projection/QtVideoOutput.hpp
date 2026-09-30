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

#pragma once

#include <QObject>
#include <boost/noncopyable.hpp>
#include <gst/gst.h>
#include <mutex>

#include <f1x/openauto/autoapp/Projection/VideoOutput.hpp>

namespace f1x::openauto::autoapp::projection
{

class QtVideoOutput: public QObject, public VideoOutput, boost::noncopyable
{
    Q_OBJECT

public:
    QtVideoOutput(configuration::IConfiguration::Pointer configuration);
    bool open() override;
    bool init() override;
    void write(uint64_t timestamp, const aasdk::common::DataConstBuffer& buffer) override;
    void stop() override;

signals:
    void startPlayback();
    void stopPlayback();

protected slots:
    void onStartPlayback();
    void onStopPlayback();

private:
    // Guards appsrc_/pipeline_ against concurrent access from the asio thread (write) and the Qt thread
    std::mutex gstMutex_;
    GstElement* pipeline_ = nullptr;
    GstElement* appsrc_ = nullptr;
};

}
