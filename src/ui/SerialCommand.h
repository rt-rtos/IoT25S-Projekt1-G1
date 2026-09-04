#pragma once
// Line-based commands over the USB serial port, the demo's control path
// (outline 13, item 4: serial is the fallback for the cmd topic).
//
//   scenario <steady|heatup|cooldown|stuck|badcrc>   emulator only
//   sample <seconds>                                  sample interval
//   help
//
// poll() reads whatever is available and calls the handler once per
// complete line with the first word and the rest (may be empty). Lines
// longer than the buffer are dropped.
#include <Arduino.h>
#include <stddef.h>

class SerialCommand {
public:
    typedef void (*Handler)(const char* cmd, const char* arg);

    explicit SerialCommand(Stream& io) : io_(io) {}
    void onCommand(Handler h) { handler_ = h; }
    void poll();

private:
    void dispatch();

    Stream& io_;
    Handler handler_ = nullptr;
    char    line_[48];
    size_t  len_ = 0;
    bool    overflow_ = false;
};
