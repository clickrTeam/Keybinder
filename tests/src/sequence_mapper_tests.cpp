#include "event.h"
#include "key_code.h"
#include "profile.h"
#include "test_util.h"
#include <gtest/gtest.h>

// ,
// Release,
// Default,
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
                    InputEvent{KeyCode::A, KeyEventType::Press},
                    InputEvent{KeyCode::B, KeyEventType::Press},

                    InputEvent{KeyCode::D, KeyEventType::Press},
                    InputEvent{KeyCode::D, KeyEventType::Release},

                    InputEvent{KeyCode::A, KeyEventType::Press},
                    InputEvent{KeyCode::B, KeyEventType::Press},

                    InputEvent{KeyCode::A, KeyEventType::Press},
                    InputEvent{KeyCode::D, KeyEventType::Press},
                },
                {
                    InputEvent{KeyCode::C, KeyEventType::Press},
                    InputEvent{KeyCode::C, KeyEventType::Release},

                    InputEvent{KeyCode::D, KeyEventType::Press},
                    InputEvent{KeyCode::D, KeyEventType::Release},

                    InputEvent{KeyCode::C, KeyEventType::Press},
                    InputEvent{KeyCode::C, KeyEventType::Release},

                    InputEvent{KeyCode::D, KeyEventType::Press},
                });
}

TEST(SequenceMapperTests, ABToCDefault) {
    mapper_test(Profile::from_bytes(QByteArray(a_b_to_c_default)),
                {
                    InputEvent{KeyCode::A, KeyEventType::Press},
                    InputEvent{KeyCode::B, KeyEventType::Press},

                    InputEvent{KeyCode::D, KeyEventType::Press},
                    InputEvent{KeyCode::D, KeyEventType::Release},

                    InputEvent{KeyCode::A, KeyEventType::Press},
                    InputEvent{KeyCode::B, KeyEventType::Press},

                    InputEvent{KeyCode::A, KeyEventType::Press},
                    InputEvent{KeyCode::D, KeyEventType::Press},
                },
                {
                    InputEvent{KeyCode::C, KeyEventType::Press},
                    InputEvent{KeyCode::C, KeyEventType::Release},

                    InputEvent{KeyCode::D, KeyEventType::Press},
                    InputEvent{KeyCode::D, KeyEventType::Release},

                    InputEvent{KeyCode::C, KeyEventType::Press},
                    InputEvent{KeyCode::C, KeyEventType::Release},

                    InputEvent{KeyCode::A, KeyEventType::Press},
                    InputEvent{KeyCode::D, KeyEventType::Press},
                });
}

TEST(SequenceMapperTests, ABToCRelease) {
    mapper_test(Profile::from_bytes(QByteArray(a_b_to_c_release)),
                {
                    InputEvent{KeyCode::A, KeyEventType::Press},
                    InputEvent{KeyCode::B, KeyEventType::Press},

                    InputEvent{KeyCode::D, KeyEventType::Press},
                    InputEvent{KeyCode::D, KeyEventType::Release},

                    InputEvent{KeyCode::A, KeyEventType::Press},
                    InputEvent{KeyCode::B, KeyEventType::Press},

                    InputEvent{KeyCode::A, KeyEventType::Press},
                    InputEvent{KeyCode::D, KeyEventType::Press},
                },
                {
                    InputEvent{KeyCode::A, KeyEventType::Press},
                    InputEvent{KeyCode::B, KeyEventType::Press},
                    InputEvent{KeyCode::C, KeyEventType::Press},
                    InputEvent{KeyCode::C, KeyEventType::Release},

                    InputEvent{KeyCode::D, KeyEventType::Press},
                    InputEvent{KeyCode::D, KeyEventType::Release},

                    InputEvent{KeyCode::A, KeyEventType::Press},
                    InputEvent{KeyCode::B, KeyEventType::Press},
                    InputEvent{KeyCode::C, KeyEventType::Press},
                    InputEvent{KeyCode::C, KeyEventType::Release},

                    InputEvent{KeyCode::A, KeyEventType::Press},
                    InputEvent{KeyCode::D, KeyEventType::Press},
                });
}
