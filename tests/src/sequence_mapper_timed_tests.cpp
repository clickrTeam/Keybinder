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
                          InputEvent::fromKey(KeyCode::A, KeyEventType::Press),
                          InputEvent::fromKey(KeyCode::A, KeyEventType::Release),
                      },
                      {
                          0,
                          100,
                      },
                      {
                          InputEvent::fromKey(KeyCode::B, KeyEventType::Press),
                          InputEvent::fromKey(KeyCode::B, KeyEventType::Release),
                      });
}

TEST(SequenceMapperTimedTests, ATapToBUnsuccessful) {
    mapper_test_timed(Profile::from_bytes(QByteArray(a_tap_to_b)),
                      {
                          InputEvent::fromKey(KeyCode::A, KeyEventType::Press),
                          InputEvent::fromKey(KeyCode::A, KeyEventType::Release),
                      },
                      {
                          0,
                          3000,
                      },
                      {
                          InputEvent::fromKey(KeyCode::A, KeyEventType::Release),
                      });
}

static const char *a_hold_to_b = R"(
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
              "type": "minimum_wait",
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

// FAILED
TEST(SequenceMapperTimedTests, AHoldToBSuccessful) {
    mapper_test_timed(Profile::from_bytes(QByteArray(a_hold_to_b)),
                      {
                          InputEvent::fromKey(KeyCode::A, KeyEventType::Press),
                          InputEvent::fromKey(KeyCode::A, KeyEventType::Release),
                      },
                      {
                          0,
                          5000,
                      },
                      {
                          InputEvent::fromKey(KeyCode::B, KeyEventType::Press),
                          InputEvent::fromKey(KeyCode::B, KeyEventType::Release),
                      });
}
TEST(SequenceMapperTimedTests, AHoldToBUnsuccessful) {
    mapper_test_timed(Profile::from_bytes(QByteArray(a_hold_to_b)),
                      {
                          InputEvent::fromKey(KeyCode::A, KeyEventType::Press),
                          InputEvent::fromKey(KeyCode::A, KeyEventType::Release),
                      },
                      {
                          0,
                          0,
                      },
                      {
                          InputEvent::fromKey(KeyCode::A, KeyEventType::Release),
                      });
}
