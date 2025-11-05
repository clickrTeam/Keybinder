
#include "profile.h"
#include <gtest/gtest.h>

static const char *profileJson1 = R"(
{
  "profile_name": "ProfileOne",
  "default_layer": 0,
  "layers": [
    {
      "layer_name": "LayerA",
      "remappings": [
        {
          "trigger": {
            "type": "key_press",
            "value": "A"
          },
          "binds": [
            {
              "type": "press_key",
              "value": "B"
            }
          ]
        }
      ]
    }
  ]
}
)";

TEST(ProfileTest, LoadProfileOne) {
    Profile profile = Profile::from_bytes(QByteArray(profileJson1));

    EXPECT_EQ(profile.name, "ProfileOne");
    EXPECT_EQ(profile.default_layer, 0);
    ASSERT_EQ(profile.layers.size(), 1);

    const Layer &layer = profile.layers[0];
    EXPECT_EQ(layer.layer_name, "LayerA");
    ASSERT_EQ(layer.basic_remappings.size(), 1);

    const auto &[trigger, binds] = layer.basic_remappings[0];
    EXPECT_TRUE(std::holds_alternative<KeyPress>(trigger));
    EXPECT_EQ(std::get<KeyPress>(trigger).key_code,
              str_to_keycode.find_forward("A"));

    ASSERT_EQ(binds.size(), 1);
    EXPECT_TRUE(std::holds_alternative<PressKey>(binds[0]));
    EXPECT_EQ(std::get<PressKey>(binds[0]).key_code,
              str_to_keycode.find_forward("B"));
}

static const char *profileJson2 = R"(
{
  "profile_name": "ProfileTwo",
  "default_layer": 1,
  "layers": [
    {
      "layer_name": "LayerB",
      "remappings": [
        {
          "triggers": [
            {
              "type": "key_press",
              "value": "C"
            },
            {
              "type": "key_release",
              "value": "D"
            }
          ],
          "binds": [
            {
              "type": "switch_layer",
              "value": 2
            }
          ],
          "behavior": "capture"
        }
      ]
    }
  ]
}
)";
TEST(ProfileTest, LoadProfileTwo) {
    Profile profile = Profile::from_bytes(QByteArray(profileJson2));

    EXPECT_EQ(profile.name, "ProfileTwo");
    EXPECT_EQ(profile.default_layer, 1);
    ASSERT_EQ(profile.layers.size(), 1);

    const Layer &layer = profile.layers[0];
    EXPECT_EQ(layer.layer_name, "LayerB");
    EXPECT_TRUE(layer.basic_remappings.isEmpty());
    ASSERT_EQ(layer.sequence_remappings.size(), 1);

    const SequenceTrigger &seq = layer.sequence_remappings[0];
    EXPECT_EQ(seq.behavior, SequenceBehavior::Capture);

    ASSERT_EQ(seq.sequence.size(), 2);
    EXPECT_TRUE(std::holds_alternative<KeyPress>(seq.sequence[0]));
    EXPECT_EQ(std::get<KeyPress>(seq.sequence[0]).key_code,
              str_to_keycode.find_forward("C"));
    EXPECT_TRUE(std::holds_alternative<KeyRelease>(seq.sequence[1]));
    EXPECT_EQ(std::get<KeyRelease>(seq.sequence[1]).key_code,
              str_to_keycode.find_forward("D"));

    ASSERT_EQ(seq.binds.size(), 1);
    EXPECT_TRUE(std::holds_alternative<SwapLayer>(seq.binds[0]));
    EXPECT_EQ(std::get<SwapLayer>(seq.binds[0]).new_layer, 2);
}
