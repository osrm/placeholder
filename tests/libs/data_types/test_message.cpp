#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "zkevm_framework/data_types/base.hpp"
#include "zkevm_framework/data_types/message.hpp"

using namespace data_types;

TEST(DataTypesMessageTests, SerializeDeserializeCommonMsgInfo) {
    Address src = {0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A,
                   0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A};
    Address dst = {0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
                   0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B};
    size_t value = 42;
    CommonMsgInfo info(src, dst, value);

    bytes blob = info.serialize();

    CommonMsgInfo result = CommonMsgInfo::deserialize(blob);

    EXPECT_EQ(result.m_src, src);
    EXPECT_EQ(result.m_dst, dst);
    EXPECT_EQ(result.m_value, value);
}

TEST(DataTypesMessageTests, SerializeDeserializeInMsg) {
    Address src = {0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A,
                   0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A};
    Address dst = {0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
                   0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B};
    size_t value = 42;
    CommonMsgInfo info(src, dst, value);
    InMsg msg(info);

    bytes blob = msg.serialize();

    InMsg result = InMsg::deserialize(blob);

    EXPECT_EQ(result.m_info.m_src, src);
    EXPECT_EQ(result.m_info.m_dst, dst);
    EXPECT_EQ(result.m_info.m_value, value);
}

TEST(DataTypesMessageTests, SerializeDeserializeOutMsg) {
    Address src = {0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A,
                   0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A};
    Address dst = {0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
                   0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B};
    size_t value = 42;
    CommonMsgInfo info(src, dst, value);
    OutMsg msg(info);

    bytes blob = msg.serialize();

    OutMsg result = OutMsg::deserialize(blob);

    EXPECT_EQ(result.m_info.m_src, src);
    EXPECT_EQ(result.m_info.m_dst, dst);
    EXPECT_EQ(result.m_info.m_value, value);
}
