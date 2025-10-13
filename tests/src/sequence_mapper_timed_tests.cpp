#include "event.h"
#include "key_code.h"
#include "profile.h"
#include "test_util.h"
#include <gtest/gtest.h>

static const char *a_tap_to_b = R"(
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
              "type": "maximum_wait",
              "duration": 500
            },
            {
              "type": "key_release",
              "value": "A"
            }
          ],
          "binds": [
            {
              "type": "press_key",
              "value": "B"
            },
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

TEST(SequenceMapperTimedTests, ATapToBSuccessful) {
    mapper_test_timed(Profile::from_bytes(QByteArray(a_tap_to_b)),
                      {
                          InputEvent{KeyCode::A, KeyEventType::Press},
                          InputEvent{KeyCode::A, KeyEventType::Release},
                      },
                      {
                          0,
                          100,
                      },
                      {
                          InputEvent{KeyCode::B, KeyEventType::Press},
                          InputEvent{KeyCode::B, KeyEventType::Release},
                      });
}

TEST(SequenceMapperTimedTests, ATapToBUnsuccessful) {
    mapper_test_timed(Profile::from_bytes(QByteArray(a_tap_to_b)),
                      {
                          InputEvent{KeyCode::A, KeyEventType::Press},
                          InputEvent{KeyCode::A, KeyEventType::Release},
                      },
                      {
                          0,
                          8000,
                      },
                      {
                          InputEvent{KeyCode::A, KeyEventType::Release},
                      });
}
