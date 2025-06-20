#pragma once

#if defined(ARDUINO)

#include <Arduino.h>

#define TBD_UNITTEST_MAIN \
void setup() \
{ \
Serial.begin(115200); \
::testing::InitGoogleTest(); \
} \
\
void loop() \
{ \
if (RUN_ALL_TESTS()) \
delay(1000); \
}

#else

#define TBD_UNITTEST_MAIN \
int main(int argc, char **argv) \
{ \
::testing::InitGoogleTest(&argc, argv); \
if (RUN_ALL_TESTS()) \
; \
return 0; \
}

#endif

