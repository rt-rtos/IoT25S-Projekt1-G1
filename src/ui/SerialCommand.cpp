#include "ui/SerialCommand.h"
// High Difficulty String parsing, ask for help if you get stuck. 
// Same idea as the Arduino "SerialCommand" library (kroimon fork), reduced
// to one handler and a Stream&: no command table, the handler gets the
// first word and the rest of the line. Not a dependency; same name only.
void SerialCommand::poll() {
    // TODO(serial): while io_.available(), c = io_.read() into line_; on '\n' call
    // dispatch() unless overflow_ is set, then reset len_ and overflow_.
    // Ignore '\r'. Set overflow_ when a line exceeds sizeof line_ - 1.
}

void SerialCommand::dispatch() {
    // TODO(serial): terminate line_, skip leading spaces, split first word
    // from the rest, call handler_(cmd, arg).
}
