#include "event.h"
#include "key_code.h"
#include "profile.h"
#include "test_util.h"
#include <gtest/gtest.h>

static const char *a_to_b_profile_json = R"(
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
        },
        {
          "trigger": {
            "type": "key_release",
            "value": "A"
          },
          "binds": [
            {
              "type": "release_key",
              "value": "B"
            }
          ]
        }
      ]
    }
  ]
}

)";

TEST(BasicMapperTests, KeyTranslation) {

    mapper_test(Profile::from_bytes(QByteArray(a_to_b_profile_json)),
                {
                    InputEvent::fromKey(KeyCode::A, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::A, KeyEventType::Release),
                    InputEvent::fromKey(KeyCode::B, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::B, KeyEventType::Release),
                    InputEvent::fromKey(KeyCode::C, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::C, KeyEventType::Release),
                },
                {
                    InputEvent::fromKey(KeyCode::B, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::B, KeyEventType::Release),
                    InputEvent::fromKey(KeyCode::B, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::B, KeyEventType::Release),
                    InputEvent::fromKey(KeyCode::C, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::C, KeyEventType::Release),
                });
}

static const char *a_b_swap_profile_json = R"(
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
                },
                {

                    "trigger": {
                        "type": "key_release",
                        "value": "A"
                    },
                    "binds": [
                        {
                            "type": "release_key",
                            "value": "B"
                        }
                    ]
                },
                {
                    "trigger": {
                        "type": "key_press",
                        "value": "B"
                    },
                    "binds": [
                        {
                            "type": "press_key",
                            "value": "A"
                        }
                    ]
                },
                {

                    "trigger": {
                        "type": "key_release",
                        "value": "B"
                    },
                    "binds": [
                        {
                            "type": "release_key",
                            "value": "A"
                        }
                    ]
                }
            ]
        }
    ]
}
)";
TEST(BasicMapperTests, KeySwap) {

    mapper_test(Profile::from_bytes(QByteArray(a_b_swap_profile_json)),
                {
                    InputEvent::fromKey(KeyCode::A, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::B, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::C, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::A, KeyEventType::Release),
                    InputEvent::fromKey(KeyCode::B, KeyEventType::Release),
                    InputEvent::fromKey(KeyCode::C, KeyEventType::Release),
                },
                {

                    InputEvent::fromKey(KeyCode::B, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::A, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::C, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::B, KeyEventType::Release),
                    InputEvent::fromKey(KeyCode::A, KeyEventType::Release),
                    InputEvent::fromKey(KeyCode::C, KeyEventType::Release),
                });
}

// Profile with 2 layers. Press c to swtich between. The first layer maps A to B
// the second maps B to A
static const char *layer_a_b_profile = R"(
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
        },
        {
          "trigger": {
            "type": "key_release",
            "value": "A"
          },
          "binds": [
            {
              "type": "release_key",
              "value": "B"
            }
          ]
        },
        {
          "trigger": {
            "type": "key_press",
            "value": "C"
          },
          "binds": [
            {
              "type": "switch_layer",
              "value": 1
            }
          ]
        }
      ]
    },
    {
      "layer_name": "LayerB",
      "remappings": [
        {
          "trigger": {
            "type": "key_press",
            "value": "B"
          },
          "binds": [
            {
              "type": "press_key",
              "value": "A"
            }
          ]
        },
        {
          "trigger": {
            "type": "key_release",
            "value": "B"
          },
          "binds": [
            {
              "type": "release_key",
              "value": "A"
            }
          ]
        },
        {
          "trigger": {
            "type": "key_press",
            "value": "C"
          },
          "binds": [
            {
              "type": "switch_layer",
              "value": 0
            }
          ]
        }
      ]
    }
  ]
}
)";

TEST(ProfileTest, BasicLayers) {
    mapper_test(Profile::from_bytes(QByteArray(layer_a_b_profile)),
                {
                    // First layer
                    InputEvent::fromKey(KeyCode::A, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::A, KeyEventType::Release),
                    InputEvent::fromKey(KeyCode::B, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::B, KeyEventType::Release),

                    InputEvent::fromKey(KeyCode::C, KeyEventType::Press),

                    // Second layer
                    InputEvent::fromKey(KeyCode::A, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::A, KeyEventType::Release),
                    InputEvent::fromKey(KeyCode::B, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::B, KeyEventType::Release),

                    InputEvent::fromKey(KeyCode::C, KeyEventType::Press),

                    // Back to first layer
                    InputEvent::fromKey(KeyCode::A, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::A, KeyEventType::Release),
                    InputEvent::fromKey(KeyCode::B, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::B, KeyEventType::Release),
                },
                {

                    InputEvent::fromKey(KeyCode::B, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::B, KeyEventType::Release),
                    InputEvent::fromKey(KeyCode::B, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::B, KeyEventType::Release),

                    InputEvent::fromKey(KeyCode::A, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::A, KeyEventType::Release),
                    InputEvent::fromKey(KeyCode::A, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::A, KeyEventType::Release),

                    InputEvent::fromKey(KeyCode::B, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::B, KeyEventType::Release),
                    InputEvent::fromKey(KeyCode::B, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::B, KeyEventType::Release),

                });
}
