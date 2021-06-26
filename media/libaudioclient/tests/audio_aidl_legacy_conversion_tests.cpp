/*
 * Copyright (C) 2021 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include <media/AudioCommonTypes.h>
#include <media/AidlConversion.h>

using namespace android;
using namespace android::aidl_utils;

namespace {

size_t hash(const media::AudioDeviceDescription& add) {
    return std::hash<media::AudioDeviceDescription>{}(add);
}

size_t hash(const media::AudioFormatDescription& afd) {
    return std::hash<media::AudioFormatDescription>{}(afd);
}

media::AudioDeviceDescription make_AudioDeviceDescription(media::AudioDeviceType type,
        const std::string& connection = "") {
    media::AudioDeviceDescription result;
    result.type = type;
    result.connection = connection;
    return result;
}

media::AudioDeviceDescription make_ADD_None() {
    return media::AudioDeviceDescription{};
}

media::AudioDeviceDescription make_ADD_DefaultIn() {
    return make_AudioDeviceDescription(media::AudioDeviceType::IN_DEFAULT);
}

media::AudioDeviceDescription make_ADD_DefaultOut() {
    return make_AudioDeviceDescription(media::AudioDeviceType::OUT_DEFAULT);
}

media::AudioDeviceDescription make_ADD_WiredHeadset() {
    return make_AudioDeviceDescription(media::AudioDeviceType::OUT_HEADSET,
            media::AudioDeviceDescription::CONNECTION_ANALOG());
}

media::AudioDeviceDescription make_ADD_BtScoHeadset() {
    return make_AudioDeviceDescription(media::AudioDeviceType::OUT_HEADSET,
            media::AudioDeviceDescription::CONNECTION_BT_SCO());
}

media::AudioFormatDescription make_AudioFormatDescription(media::AudioFormatType type) {
    media::AudioFormatDescription result;
    result.type = type;
    return result;
}

media::AudioFormatDescription make_AudioFormatDescription(media::PcmType pcm) {
    auto result = make_AudioFormatDescription(media::AudioFormatType::PCM);
    result.pcm = pcm;
    return result;
}

media::AudioFormatDescription make_AudioFormatDescription(const std::string& encoding) {
    media::AudioFormatDescription result;
    result.encoding = encoding;
    return result;
}

media::AudioFormatDescription make_AudioFormatDescription(media::PcmType transport,
        const std::string& encoding) {
    auto result = make_AudioFormatDescription(encoding);
    result.pcm = transport;
    return result;
}

media::AudioFormatDescription make_AFD_Default() {
    return media::AudioFormatDescription{};
}

media::AudioFormatDescription make_AFD_Invalid() {
    return make_AudioFormatDescription(media::AudioFormatType::SYS_RESERVED_INVALID);
}

media::AudioFormatDescription make_AFD_Pcm16Bit() {
    return make_AudioFormatDescription(media::PcmType::INT_16_BIT);
}

media::AudioFormatDescription make_AFD_Bitstream() {
    return make_AudioFormatDescription("example");
}

media::AudioFormatDescription make_AFD_Encap() {
    return make_AudioFormatDescription(media::PcmType::INT_16_BIT, "example.encap");
}

media::AudioFormatDescription make_AFD_Encap_with_Enc() {
    auto afd = make_AFD_Encap();
    afd.encoding += "+example";
    return afd;
}

}  // namespace

// Verify that two independently constructed ADDs/AFDs have the same hash.
// This ensures that regardless of whether the ADD/AFD instance originates
// from, it can be correctly compared to other ADD/AFD instance. Thus,
// for example, a 16-bit integer format description provided by HAL
// is identical to the same format description constructed by the framework.
class HashIdentityTest : public ::testing::Test {
  public:
    template<typename T> void verifyHashIdentity(const std::vector<std::function<T()>>& valueGens) {
        for (size_t i = 0; i < valueGens.size(); ++i) {
            for (size_t j = 0; j < valueGens.size(); ++j) {
                if (i == j) {
                    EXPECT_EQ(hash(valueGens[i]()), hash(valueGens[i]())) << i;
                } else {
                    EXPECT_NE(hash(valueGens[i]()), hash(valueGens[j]())) << i << ", " << j;
                }
            }
        }
    }
};

TEST_F(HashIdentityTest, AudioDeviceDescriptionHashIdentity) {
    verifyHashIdentity<media::AudioDeviceDescription>({
            make_ADD_None, make_ADD_DefaultIn, make_ADD_DefaultOut, make_ADD_WiredHeadset,
            make_ADD_BtScoHeadset});
}

TEST_F(HashIdentityTest, AudioFormatDescriptionHashIdentity) {
    verifyHashIdentity<media::AudioFormatDescription>({
            make_AFD_Default, make_AFD_Invalid, make_AFD_Pcm16Bit, make_AFD_Bitstream,
            make_AFD_Encap, make_AFD_Encap_with_Enc});
}

class AudioDeviceDescriptionRoundTripTest :
        public testing::TestWithParam<media::AudioDeviceDescription> {};
TEST_P(AudioDeviceDescriptionRoundTripTest, Aidl2Legacy2Aidl) {
    const auto initial = GetParam();
    auto conv = aidl2legacy_AudioDeviceDescription_audio_devices_t(initial);
    ASSERT_TRUE(conv.ok());
    auto convBack = legacy2aidl_audio_devices_t_AudioDeviceDescription(conv.value());
    ASSERT_TRUE(convBack.ok());
    EXPECT_EQ(initial, convBack.value());
}
INSTANTIATE_TEST_SUITE_P(AudioDeviceDescriptionRoundTrip,
        AudioDeviceDescriptionRoundTripTest,
        testing::Values(media::AudioDeviceDescription{}, make_ADD_DefaultIn(),
                make_ADD_DefaultOut(), make_ADD_WiredHeadset(), make_ADD_BtScoHeadset()));

class AudioFormatDescriptionRoundTripTest :
        public testing::TestWithParam<media::AudioFormatDescription> {};
TEST_P(AudioFormatDescriptionRoundTripTest, Aidl2Legacy2Aidl) {
    const auto initial = GetParam();
    auto conv = aidl2legacy_AudioFormatDescription_audio_format_t(initial);
    ASSERT_TRUE(conv.ok());
    auto convBack = legacy2aidl_audio_format_t_AudioFormatDescription(conv.value());
    ASSERT_TRUE(convBack.ok());
    EXPECT_EQ(initial, convBack.value());
}
INSTANTIATE_TEST_SUITE_P(AudioFormatDescriptionRoundTrip,
        AudioFormatDescriptionRoundTripTest,
        testing::Values(make_AFD_Invalid(), media::AudioFormatDescription{}, make_AFD_Pcm16Bit()));
