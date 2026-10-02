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

#include <boost/property_tree/ini_parser.hpp>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>

namespace f1x::openauto::autoapp::configuration
{

class Configuration: public IConfiguration
{
public:
    Configuration();

    void load() override;
    void reset() override;
    void save() override;

    void setHandednessOfTrafficType(HandednessOfTrafficType value) override;
    HandednessOfTrafficType getHandednessOfTrafficType() const override;

    aasdk::proto::enums::VideoFPS::Enum getVideoFPS() const override;
    void setVideoFPS(aasdk::proto::enums::VideoFPS::Enum value) override;
    aasdk::proto::enums::VideoResolution::Enum getVideoResolution() const override;
    void setVideoResolution(aasdk::proto::enums::VideoResolution::Enum value) override;
    size_t getScreenDPI() const override;
    void setScreenDPI(size_t value) override;
    bool getTouchscreenEnabled() const override;
    void setTouchscreenEnabled(bool value) override;
    ButtonCodes getButtonCodes() const override;
    void setButtonCodes(const ButtonCodes& value) override;

    bool musicAudioChannelEnabled() const override;
    void setMusicAudioChannelEnabled(bool value) override;
    bool speechAudioChannelEnabled() const override;
    void setSpeechAudioChannelEnabled(bool value) override;

private:
    void readButtonCodes(boost::property_tree::ptree& iniConfig);
    void insertButtonCode(boost::property_tree::ptree& iniConfig, const std::string& buttonCodeKey, aasdk::proto::enums::ButtonCode::Enum buttonCode);
    void writeButtonCodes(boost::property_tree::ptree& iniConfig);

    HandednessOfTrafficType handednessOfTrafficType_;
    aasdk::proto::enums::VideoFPS::Enum videoFPS_;
    aasdk::proto::enums::VideoResolution::Enum videoResolution_;
    size_t screenDPI_;
    bool enableTouchscreen_;
    ButtonCodes buttonCodes_;
    bool musicAudioChannelEnabled_;
    bool speechAudiochannelEnabled_;

    static const std::string cConfigFilePath;

    static const std::string cGeneralHandednessOfTrafficTypeKey;

    static const std::string cVideoFPSKey;
    static const std::string cVideoResolutionKey;
    static const std::string cVideoScreenDPIKey;

    static const std::string cAudioMusicAudioChannelEnabled;
    static const std::string cAudioSpeechAudioChannelEnabled;


    static const std::string cInputEnableTouchscreenKey;
    static const std::string cInputPlayButtonKey;
    static const std::string cInputPauseButtonKey;
    static const std::string cInputTogglePlayButtonKey;
    static const std::string cInputNextTrackButtonKey;
    static const std::string cInputPreviousTrackButtonKey;
    static const std::string cInputHomeButtonKey;
    static const std::string cInputPhoneButtonKey;
    static const std::string cInputCallEndButtonKey;
    static const std::string cInputVoiceCommandButtonKey;
    static const std::string cInputLeftButtonKey;
    static const std::string cInputRightButtonKey;
    static const std::string cInputUpButtonKey;
    static const std::string cInputDownButtonKey;
    static const std::string cInputScrollWheelButtonKey;
    static const std::string cInputBackButtonKey;
    static const std::string cInputEnterButtonKey;
};

}
