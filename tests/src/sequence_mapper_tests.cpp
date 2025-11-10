#include "event.h"
#include "key_code.h"
#include "profile.h"
#include "test_util.h"
#include <gtest/gtest.h>

static const char *a_b_to_c_capture = R"(
{
  "profile_name": "ProfileOne",
  "default_layer": 0,
  "layers": [
    {
      "layer_name": "LayerA",
      "remappings": [
        {

          "behavior": "capture",
          "triggers": [
            {
              "type": "key_press",
              "value": "A"
            },
            {
              "type": "key_press",
              "value": "B"
            }
          ],
          "binds": [
            {
              "type": "press_key",
              "value": "C"
            },
            {
              "type": "release_key",
              "value": "C"
            }
          ]
        }
      ]
    }
  ]
}
)";

static const char *a_b_to_c_release = R"(
{
  "profile_name": "ProfileOne",
  "default_layer": 0,
  "layers": [
    {
      "layer_name": "LayerA",
      "remappings": [
        {
          "behavior": "release",
          "triggers": [
            {
              "type": "key_press",
              "value": "A"
            },
            {
              "type": "key_press",
              "value": "B"
            }
          ],
          "binds": [
            {
              "type": "press_key",
              "value": "C"
            },
            {
              "type": "release_key",
              "value": "C"
            }
          ]
        }
      ]
    }
  ]
}
)";

static const char *a_b_to_c_default = R"(
{
  "profile_name": "ProfileOne",
  "default_layer": 0,
  "layers": [
    {
      "layer_name": "LayerA",
      "remappings": [
        {
          "behavior": "default",
          "triggers": [
            {
              "type": "key_press",
              "value": "A"
            },
            {
              "type": "key_press",
              "value": "B"
            }
          ],
          "binds": [
            {
              "type": "press_key",
              "value": "C"
            },
            {
              "type": "release_key",
              "value": "C"
            }
          ]
        }
      ]
    }
  ]
}
)";

TEST(SequenceMapperTests, ABToCCapture) {
    mapper_test(Profile::from_bytes(QByteArray(a_b_to_c_capture)),
                {
                    InputEvent::fromKey(KeyCode::A, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::B, KeyEventType::Press),

                    InputEvent::fromKey(KeyCode::D, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::D, KeyEventType::Release),

                    InputEvent::fromKey(KeyCode::A, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::B, KeyEventType::Press),

                    InputEvent::fromKey(KeyCode::A, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::D, KeyEventType::Press),
                },
                {
                    InputEvent::fromKey(KeyCode::C, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::C, KeyEventType::Release),

                    InputEvent::fromKey(KeyCode::D, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::D, KeyEventType::Release),

                    InputEvent::fromKey(KeyCode::C, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::C, KeyEventType::Release),

                    InputEvent::fromKey(KeyCode::D, KeyEventType::Press),
                });
}

TEST(SequenceMapperTests, ABToCDefault) {
    mapper_test(Profile::from_bytes(QByteArray(a_b_to_c_default)),
                {
                    InputEvent::fromKey(KeyCode::A, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::B, KeyEventType::Press),

                    InputEvent::fromKey(KeyCode::D, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::D, KeyEventType::Release),

                    InputEvent::fromKey(KeyCode::A, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::B, KeyEventType::Press),

                    InputEvent::fromKey(KeyCode::A, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::D, KeyEventType::Press),
                },
                {
                    InputEvent::fromKey(KeyCode::C, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::C, KeyEventType::Release),

                    InputEvent::fromKey(KeyCode::D, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::D, KeyEventType::Release),

                    InputEvent::fromKey(KeyCode::C, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::C, KeyEventType::Release),

                    InputEvent::fromKey(KeyCode::A, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::D, KeyEventType::Press),
                });
}

TEST(SequenceMapperTests, ABToCRelease) {
    mapper_test(Profile::from_bytes(QByteArray(a_b_to_c_release)),
                {
                    InputEvent::fromKey(KeyCode::A, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::B, KeyEventType::Press),

                    InputEvent::fromKey(KeyCode::D, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::D, KeyEventType::Release),

                    InputEvent::fromKey(KeyCode::A, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::B, KeyEventType::Press),

                    InputEvent::fromKey(KeyCode::A, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::D, KeyEventType::Press),
                },
                {
                    InputEvent::fromKey(KeyCode::A, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::B, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::C, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::C, KeyEventType::Release),

                    InputEvent::fromKey(KeyCode::D, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::D, KeyEventType::Release),

                    InputEvent::fromKey(KeyCode::A, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::B, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::C, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::C, KeyEventType::Release),

                    InputEvent::fromKey(KeyCode::A, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::D, KeyEventType::Press),
                });
}

// A B -> C
// A D -> E
static const char *overloaded_sequence = R"(
{
  "profile_name": "ProfileOne",
  "default_layer": 0,
  "layers": [
    {
      "layer_name": "LayerA",
      "remappings": [
        {
          "behavior": "capture",
          "triggers": [
            {
              "type": "key_press",
              "value": "A"
            },
            {
              "type": "key_press",
              "value": "B"
            }
          ],
          "binds": [
            {
              "type": "press_key",
              "value": "C"
            },
            {
              "type": "release_key",
              "value": "C"
            }
          ]
        },
        {
          "behavior": "capture",
          "triggers": [
            {
              "type": "key_press",
              "value": "A"
            },
            {
              "type": "key_press",
              "value": "D"
            }
          ],
          "binds": [
            {
              "type": "press_key",
              "value": "E"
            },
            {
              "type": "release_key",
              "value": "E"
            }
          ]
        }
      ]
    }
  ]
}
)";

// A B -> C
// A D -> E
TEST(SequenceMapperTests, OverloadedSequence) {
    mapper_test(Profile::from_bytes(QByteArray(overloaded_sequence)),
                {
                    InputEvent::fromKey(KeyCode::A, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::B, KeyEventType::Press),

                    InputEvent::fromKey(KeyCode::A, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::D, KeyEventType::Press),

                    InputEvent::fromKey(KeyCode::A, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::S, KeyEventType::Press),
                },
                {
                    InputEvent::fromKey(KeyCode::C, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::C, KeyEventType::Release),

                    InputEvent::fromKey(KeyCode::E, KeyEventType::Press),
                    InputEvent::fromKey(KeyCode::E, KeyEventType::Release),

                    InputEvent::fromKey(KeyCode::S, KeyEventType::Press),
                });
}
